// Fill out your copyright notice in the Description page of Project Settings.


#include "GalacticPlanetGlobe.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "../../UI/PlanetGlobeWidget.h"
#include "PlanetOperationSite.h"
#include "PlanetSelectRing.h"

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
}

void AGalacticPlanetGlobe::OnIconInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (auto icon = Cast<APlanetObjectiveIcon>(OtherActor))
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
            _globeWidget->ShowObjection(true, icon);
			_curIcon = icon;
        }
	}
}

void AGalacticPlanetGlobe::OnIconOutOfRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
    if (auto icon = Cast<APlanetObjectiveIcon>(OtherActor))
    {
        _globeWidget->ShowObjection(false);
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
        if (_curSite)
        {
            EnterFocus(_curSite);
        }
    }
    else if (_mode == EPlanetGlobeMode::Focus)
    {
        // 해당 임무 선택
	}
}

void AGalacticPlanetGlobe::EnterFocus(APlanetOperationSite* site)
{
    _curSiteLoc = site->GetActorLocation(); // 또는 아이콘 기준 앵커

    // 선택 지점을 화면 중앙으로 오게 글로브 목표 회전 계산
    FVector globeCenter = _mesh->GetComponentLocation();
    FVector normalVec = (_curSiteLoc - globeCenter).GetSafeNormal();
    FQuat quat = FRotationMatrix::MakeFromX(normalVec).ToQuat();
    _targetGlobeRotation = quat;

    // 링 스크린 초기값 = 선택 지점의 투영
    _playerController->ProjectWorldLocationToScreen(_curSiteLoc, _ringScreenPos, true);
    _ringVel = FVector2D::ZeroVector;

    _mode = EPlanetGlobeMode::Focus;
}

void AGalacticPlanetGlobe::TickBrowseMode(float DeltaTime)
{
    FVector2D delta;
    _playerController->GetInputMouseDelta(delta.X, delta.Y);
    delta *= 15.f;

    RotateGlobe(delta.X, delta.Y);

    // 구 중심을 스크린 좌표로
    FVector2D Screen;
    _playerController->ProjectWorldLocationToScreen(_mesh->GetComponentLocation(), Screen, true);

    // 스크린→월드 레이 만들기
    FVector rayOrigin, rayDir;
    _playerController->DeprojectScreenPositionToWorld(Screen.X, Screen.Y, rayOrigin, rayDir);

    // 글로브만 맞는 채널로 라인트레이스
    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);

    // 맞았으면 링 배치
    if (hit.bBlockingHit)
    {
        const FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        const FVector CamRight = _playerController->PlayerCameraManager->GetActorRightVector();
        const FVector UpTangent = FVector::CrossProduct(CamRight, surfaceNormal).GetSafeNormal();

        _ring->PlaceOnSurface(hit.ImpactPoint, surfaceNormal, UpTangent);
    }
}

void AGalacticPlanetGlobe::TickFocusMode(float DeltaTime)
{
    FVector2D delta;
    _playerController->GetInputMouseDelta(delta.X, delta.Y);
    delta.X *= 50.f;
    delta.Y *= -50.f;

    // 링 모션
    FVector2D anchor;
    _playerController->ProjectWorldLocationToScreen(_curSiteLoc, anchor, true);

    float followFactor = 0.6f;
    float springConstant = 12.f;
    float damping = 8.f;

    FVector2D target = anchor + delta * followFactor;
    const FVector2D ringDelta = (target - _ringScreenPos);

    _ringVel += ringDelta * springConstant * DeltaTime;
    _ringVel -= _ringVel * damping * DeltaTime;
    _ringScreenPos += _ringVel * DeltaTime;

    // 글로브 모션
    float _orbitSensitivity = 0.08f;
    float yaw = ringDelta.X * _orbitSensitivity;
    float pitch = ringDelta.Y * _orbitSensitivity;

    const FQuat offset =
        FQuat(FVector::UpVector, yaw) *
        FQuat(FVector::RightVector, pitch);

    FQuat targetQuat = offset * _targetGlobeRotation;

    // 보간
    float slerp = 10.f * DeltaTime;
    FQuat newRot = FQuat::Slerp(_mesh->GetComponentQuat(), targetQuat, slerp);
    _mesh->SetWorldRotation(newRot);

    // 링 월드 배치
    FVector rayOrigin, rayDir;
    _playerController->DeprojectScreenPositionToWorld(_ringScreenPos.X, _ringScreenPos.Y, rayOrigin, rayDir);

    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);
    if (hit.bBlockingHit)
    {
        const FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        const FVector camRight = _playerController->PlayerCameraManager->GetActorRightVector();
        const FVector upTan = FVector::CrossProduct(camRight, surfaceNormal).GetSafeNormal();
        _ring->PlaceOnSurface(hit.ImpactPoint, surfaceNormal, upTan);
    }
}

void AGalacticPlanetGlobe::RotateGlobe(float deltaX, float deltaY)
{
    FQuat quat = _mesh->GetComponentQuat();

    const APlayerCameraManager* Cam = _playerController->PlayerCameraManager;
    FVector viewRight = Cam->GetActorRightVector().GetSafeNormal();

    //FVector worldRight = FVector::RightVector; // 월드 기준 오른쪽 벡터

    float newPitch = FMath::Clamp(_curPitchDeg + -deltaY, -70.f, 70.f);
    float deltaPitch = newPitch - _curPitchDeg;
    _curPitchDeg = newPitch;

    FQuat pitchQuat = FQuat(viewRight, FMath::DegreesToRadians(deltaPitch));
    quat = pitchQuat * quat; // Pitch 먼저 적용

    FVector localUp = quat.GetUpVector(); // Pitch가 적용된 후의 로컬 Up 벡터(자전축)를 구함
    FQuat yawQuat = FQuat(localUp, FMath::DegreesToRadians(deltaX));
    quat = yawQuat * quat; // Yaw 적용

    quat.Normalize();
    _mesh->SetWorldRotation(quat);
}

FVector AGalacticPlanetGlobe::CalculateGlobePosition(float latitude, float longitude, float globeRadius)
{
    float lat = FMath::DegreesToRadians(latitude);
    float lon = FMath::DegreesToRadians(longitude);
    return { globeRadius * FMath::Cos(lat) * FMath::Cos(lon),
             globeRadius * FMath::Cos(lat) * FMath::Sin(lon),
             globeRadius * FMath::Sin(lat) };
}

