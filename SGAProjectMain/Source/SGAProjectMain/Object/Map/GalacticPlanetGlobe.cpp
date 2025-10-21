// Fill out your copyright notice in the Description page of Project Settings.


#include "GalacticPlanetGlobe.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "../../UI/PlanetGlobeWidget.h"
#include "../../CGameInstance.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "PlanetOperationSite.h"
#include "PlanetSelectRing.h"
#include "PlanetMissionIcon.h"
#include "../../Data/MissionDataAsset.h"
#include "../../Data/OperationDataAsset.h"

// Sets default values
AGalacticPlanetGlobe::AGalacticPlanetGlobe()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGalacticPlanetGlobe::BeginPlay()
{
	Super::BeginPlay();

    InitializeOperations();
	
	_playerController = GetWorld()->GetFirstPlayerController();
    SetPlayerInputComponent();

    if (_ringClass)
    {
        FActorSpawnParameters params;
        params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        _ring = GetWorld()->SpawnActor<APlanetSelectRing>(_ringClass, FVector::ZeroVector, FRotator::ZeroRotator, params);
        _ring->GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &AGalacticPlanetGlobe::OnIconInRange);
        _ring->GetMesh()->OnComponentEndOverlap.AddDynamic(this, &AGalacticPlanetGlobe::OnIconOutOfRange);
		_ring->SetActorHiddenInGame(true);
    }
	
    if (_globeWidgetClass)
		_globeWidget = CreateWidget<UPlanetGlobeWidget>(GetWorld(), _globeWidgetClass);
}

void AGalacticPlanetGlobe::InitializeOperations()
{
    for (FOperationData& opData : _operations)
    {
        if (!opData.OperationSiteClass)
            continue;

        UChildActorComponent* opSite = NewObject<UChildActorComponent>(this);
        if (opSite)
        {
            opSite->SetupAttachment(RootComponent);
            opSite->SetChildActorClass(opData.OperationSiteClass);
            opSite->RegisterComponent();
            _operationSites.Add(opSite);

            FVector localPos = CalculateGlobePosition(
                opData._latitude,
                opData._longitude,
                _globeRadius
            );
            opSite->SetRelativeLocation(localPos);

            // 표면을 향하도록 회전 (중심에서 바깥쪽)
            FRotator lookRotation = (-localPos).Rotation();
            opSite->SetRelativeRotation(lookRotation);
			APlanetOperationSite* opSiteActor = Cast<APlanetOperationSite>(opSite->GetChildActor());
        }
    }
}

void AGalacticPlanetGlobe::SetPlayerInputComponent()
{
	if (!_playerController) return;
	UInputComponent* playerInputComponent = _playerController->InputComponent;
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(playerInputComponent))
    {
        EIC->BindAction(_selectAction, ETriggerEvent::Triggered, this, &AGalacticPlanetGlobe::OnSelect);
    }
}

// Called every frame
void AGalacticPlanetGlobe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_isInteracting) return;

    if (_mode == EPlanetGlobeMode::Browse)
        TickBrowseMode(DeltaTime);

    else if (_mode == EPlanetGlobeMode::Focus)
        TickFocusMode(DeltaTime);
}

void AGalacticPlanetGlobe::Interact(AHellDiver* player)
{
	_isInteracting = !_isInteracting;
    if (_isInteracting)
        StartInteracting();
    else
        StopInteracting();
}

void AGalacticPlanetGlobe::StartInteracting()
{
	_isInteracting = true;
    _globeWidget->AddToViewport();
    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(_playerController->GetLocalPlayer()))
    { // IMC 교체
        subsystem->RemoveMappingContext(_gameIMC);
        subsystem->AddMappingContext(_globeWidgetIMC, 10);
    }
    _ring->SetActorHiddenInGame(false);
    HideMark();
}

void AGalacticPlanetGlobe::StopInteracting()
{
	_isInteracting = false;
    _globeWidget->RemoveFromParent();
    _mode = EPlanetGlobeMode::Browse;
    _curSite = nullptr;
	_curIcon = nullptr;
    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(_playerController->GetLocalPlayer()))
    { // IMC 복구
        subsystem->RemoveMappingContext(_globeWidgetIMC);
        subsystem->AddMappingContext(_gameIMC, 10);
	}
    _ring->SetActorHiddenInGame(true);
}

void AGalacticPlanetGlobe::OnIconInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (auto icon = Cast<APlanetMissionIcon>(OtherActor))
    {
        if (_mode == EPlanetGlobeMode::Browse)
        { // 아이콘이 속한 지점의 임무 표시
            if (auto site = Cast<APlanetOperationSite>(icon->GetParentActor()))
            {
                _globeWidget->ShowOperation(true, site);
                _curSite = site;
            }
        }

        else
        { // 아이콘에 해당하는 목표 표시
            _globeWidget->ShowMission(true, icon);
			_curIcon = icon;
        }
	}
}

void AGalacticPlanetGlobe::OnIconOutOfRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (auto icon = Cast<APlanetMissionIcon>(OtherActor))
    {
        _globeWidget->ShowMission(false);
		_curIcon = nullptr;

        if (_mode == EPlanetGlobeMode::Browse)
        {
            _globeWidget->ShowOperation(false);
            _curSite = nullptr;
        }
	}
}

void AGalacticPlanetGlobe::OnSelect(const FInputActionValue& value)
{
    if (_mode == EPlanetGlobeMode::Browse)
    {
        EnterFocus();
    }
    else if (_mode == EPlanetGlobeMode::Focus)
    {
        SelectMission();
	}
}

void AGalacticPlanetGlobe::OnBack(const FInputActionValue& value)
{
    if (_mode == EPlanetGlobeMode::Focus)
    {
        _mode = EPlanetGlobeMode::Browse;
        _globeWidget->EnterMissionMode();
	}
    if (_mode == EPlanetGlobeMode::Browse)
    {
        StopInteracting();
    }
}

void AGalacticPlanetGlobe::EnterFocus()
{
    if (!_curSite) return;

    _curSite->ChangeToFocusMode();
    _curSiteLoc = _curSite->GetActorLocation(); // 선택한 지점 위치

    // 선택 지점을 화면 중앙으로 오게 글로브 목표 회전 계산
    FVector globeCenter = _mesh->GetComponentLocation();
    FVector normalVec = (_curSiteLoc - globeCenter).GetSafeNormal();
    FQuat quat = FRotationMatrix::MakeFromX(normalVec).ToQuat();
    _baseGlobeRotation = quat;

    // 링 스크린 위치 초기화 -> 선택 지점 위치
    _playerController->ProjectWorldLocationToScreen(_curSiteLoc, _ringScreenPos, true);

    _mode = EPlanetGlobeMode::Focus;
	_globeWidget->EnterMissionMode();
}

void AGalacticPlanetGlobe::SelectMission()
{
	if (!_curIcon || !_curSite) return;

    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

	GI->GetPreDeployState()->SetCurOperation(_curSite->GetOperationData());
	GI->GetPreDeployState()->SetCurMission(_curIcon->GetMissionData());
        
    StopInteracting();
}

void AGalacticPlanetGlobe::TickBrowseMode(float DeltaTime)
{
    FVector2D delta;
    _playerController->GetInputMouseDelta(delta.X, delta.Y);
    delta *= 15.f;

    float newPitch = FMath::Clamp(_curPitchDeg + -delta.Y, -70.f, 70.f);
    float deltaPitch = newPitch - _curPitchDeg;
    _curPitchDeg = newPitch;

    FQuat newQuat = CalculateNewGlobeQuat(delta.X, deltaPitch, _mesh->GetComponentQuat());
    _mesh->SetWorldRotation(newQuat);

    // 구 중심을 스크린 좌표로
    FVector2D screen;
    _playerController->ProjectWorldLocationToScreen(_mesh->GetComponentLocation(), screen, true);

    // 스크린 -> 월드 레이 만들기
    FVector rayOrigin, rayDir;
    _playerController->DeprojectScreenPositionToWorld(screen.X, screen.Y, rayOrigin, rayDir);

    // 글로브만 맞는 채널로 라인트레이스
    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);

    // 맞았으면 링 배치
    if (hit.bBlockingHit)
    {
        const FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        const FVector camRight = _playerController->PlayerCameraManager->GetActorRightVector();
        const FVector upTangent = FVector::CrossProduct(camRight, surfaceNormal).GetSafeNormal();

        _ring->PlaceOnSurface(hit.ImpactPoint, surfaceNormal, upTangent);
    }
}

void AGalacticPlanetGlobe::TickFocusMode(float DeltaTime)
{
    FVector2D delta;
    _playerController->GetInputMouseDelta(delta.X, delta.Y);
	delta.Y *= -1.f; // Y축 반전

    // 링 모션
    FVector2D anchor;
    _playerController->ProjectWorldLocationToScreen(_curSiteLoc, anchor, true);

    FVector2D target = anchor + delta * _ringSensitivity;
    FVector distanceFromSite = _ring->GetActorLocation() - _curSiteLoc; // 현재 링과 현재 지역 간의 거리
    const float distanceSquared = distanceFromSite.SizeSquared();

    float slerp = 10.f * DeltaTime;
    if (distanceSquared > FMath::Square(_siteZoneRadius)) // 링이 지역에서부터 일정 거리를 벗어날 경우 복귀
    {
        _ringScreenPos = FMath::Lerp(_ringScreenPos, target, slerp);
    }
    else
    {
		_ringScreenPos = _ringScreenPos + delta * _ringSensitivity; // 일정 거리 내에서는 마우스 이동량만큼 이동
    }
	UE_LOG(LogTemp, Log, TEXT("distanceSquared: %f"), distanceSquared);

    // 링 월드 배치
    FVector rayOrigin, rayDir;
    _playerController->DeprojectScreenPositionToWorld(_ringScreenPos.X, _ringScreenPos.Y, rayOrigin, rayDir);

    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);
    if (hit.bBlockingHit)
    {
        const FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        const FVector camRight = _playerController->PlayerCameraManager->GetActorRightVector();
        const FVector upTangent = FVector::CrossProduct(camRight, surfaceNormal).GetSafeNormal();
        _ring->PlaceOnSurface(hit.ImpactPoint, surfaceNormal, upTangent);
    }

    // 글로브 모션
    float deltaYaw = delta.X * _globeSensitivity;
    float deltaPitch = delta.Y * _globeSensitivity;

    FQuat targetQuat = CalculateNewGlobeQuat(deltaYaw, deltaPitch, _baseGlobeRotation);

	FQuat newRot = FQuat::Slerp(_mesh->GetComponentQuat(), targetQuat, slerp); // 구면 선형 보간
    _mesh->SetWorldRotation(newRot);
}

FQuat AGalacticPlanetGlobe::CalculateNewGlobeQuat(float deltaYaw, float deltaPitch, FQuat baseQuat)
{
    auto cam = _playerController->PlayerCameraManager;
    FVector viewRight = cam->GetActorRightVector().GetSafeNormal();

    FQuat curQuat = _mesh->GetComponentQuat();
    FQuat pitchOffset = FQuat(viewRight, FMath::DegreesToRadians(deltaPitch));
    FVector localUp = (pitchOffset * curQuat).GetUpVector(); // Pitch가 적용된 후의 로컬 Up 벡터(자전축)를 구함
    FQuat yawOffset = FQuat(localUp, FMath::DegreesToRadians(deltaYaw));

    FQuat newQuat = yawOffset * pitchOffset * baseQuat;
    newQuat.Normalize();
    return newQuat;
}

FVector AGalacticPlanetGlobe::CalculateGlobePosition(float latitude, float longitude, float globeRadius)
{
    float lat = FMath::DegreesToRadians(latitude);
    float lon = FMath::DegreesToRadians(longitude);
    return { globeRadius * FMath::Cos(lat) * FMath::Cos(lon),
             globeRadius * FMath::Cos(lat) * FMath::Sin(lon),
             globeRadius * FMath::Sin(lat) };
}

