// Fill out your copyright notice in the Description page of Project Settings.


#include "GalacticPlanetGlobe.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraActor.h"
#include "Components/TimelineComponent.h"
#include "../../UI/PlanetGlobeWidget.h"
#include "../../CGameInstance.h"
#include "../../CSaveGame.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "PlanetOperationSite.h"
#include "PlanetSelectRing.h"
#include "PlanetMissionIcon.h"
#include "../../Data/MissionDataAsset.h"
#include "../../Data/OperationDataAsset.h"
#include "../../Helper/H_CharacterLoc.h"

// Sets default values
AGalacticPlanetGlobe::AGalacticPlanetGlobe()
{
	PrimaryActorTick.bCanEverTick = true;
	
    _root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = _root;
	_interactionMark->SetupAttachment(RootComponent);

	_globeRoot = CreateDefaultSubobject<USceneComponent>(TEXT("GlobeRoot"));
	_globeRoot->SetupAttachment(RootComponent);
    _mesh->SetupAttachment(_globeRoot);

	_browseCamera = CreateDefaultSubobject<UChildActorComponent>(TEXT("BrowseCamera"));
	_browseCamera->SetupAttachment(RootComponent);
    _browseCamera->SetChildActorClass(ACameraActor::StaticClass());
	_focusCamera = CreateDefaultSubobject<UChildActorComponent>(TEXT("FocusCamera"));
	_focusCamera->SetupAttachment(RootComponent);
	_focusCamera->SetChildActorClass(ACameraActor::StaticClass());

	_focusTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	_exitTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ExitTimeline"));

	_playerAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerAnchor"));
	_playerAnchor->SetupAttachment(RootComponent);

	_rotatingBand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RotatingBand"));
	_rotatingBand->SetupAttachment(RootComponent);
 }

// Called when the game starts or when spawned
void AGalacticPlanetGlobe::BeginPlay()
{
	Super::BeginPlay();

    InitializeOperations();
	
	_playerController = GetWorld()->GetFirstPlayerController();
    _playerViewTarget = _playerController->GetViewTarget();
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

    if (_timelineCurve)
    {
        FOnTimelineFloat focusTimelineUpdate;
        focusTimelineUpdate.BindUFunction(this, FName("OnFocusTimelineUpdate"));
		_focusTimeline->AddInterpFloat(_timelineCurve, focusTimelineUpdate);

        FOnTimelineFloat exitTimelineUpdate;
        exitTimelineUpdate.BindUFunction(this, FName("OnExitTimelineUpdate"));
        _exitTimeline->AddInterpFloat(_timelineCurve, exitTimelineUpdate);

        FOnTimelineEvent timelineFinished;
		timelineFinished.BindUFunction(this, FName("OnFocusTimelineFinished"));
		_focusTimeline->SetTimelineFinishedFunc(timelineFinished);
    }
	
    if (_globeWidgetClass)
		_globeWidget = CreateWidget<UPlanetGlobeWidget>(GetWorld(), _globeWidgetClass);

    if (_rotatingBand)
    {
		_startBandLoc = _rotatingBand->GetRelativeLocation();
		_targetBandLoc = _startBandLoc + FVector(0.f, 0.f, 200.f);
    }

	auto GI = Cast<UCGameInstance>(GetGameInstance());
	if (!GI) return;

    if (auto save = GI->GetCurrentSave())
    {
        // 현재 저장된 임무가 있으면 글로브 포커스 모드 진입
        if (save->GetCurOperationID().IsValid())
        {
            auto opData = GI->GetOperationDataAsset(save->GetCurOperationID());
            EnterFocusByOperation(opData);
        }
    }
}

void AGalacticPlanetGlobe::InitializeOperations()
{
    int32 opIndex = 0;

    for (FOperationData& opData : _operations)
    {
        if (!opData.OperationSiteClass)
            continue;

        UChildActorComponent* opSite = NewObject<UChildActorComponent>(this);
        if (opSite)
        {
            opSite->SetupAttachment(_mesh);
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
			if (opSiteActor)
				opSiteActor->SetOperationIndex(opIndex++);
        }
    }
}

void AGalacticPlanetGlobe::SetPlayerInputComponent()
{
	if (!_playerController) return;
	UInputComponent* playerInputComponent = _playerController->InputComponent;
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(playerInputComponent))
    {
        EIC->BindAction(_selectAction, ETriggerEvent::Started, this, &AGalacticPlanetGlobe::OnSelect);
		EIC->BindAction(_backAction, ETriggerEvent::Started, this, &AGalacticPlanetGlobe::OnBack);
    }
}

// Called every frame
void AGalacticPlanetGlobe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 글로브 상단 밴드 회전
    if (_rotatingBand)
    {
        FRotator rot = _rotatingBand->GetRelativeRotation();
        rot.Yaw += 20.f * DeltaTime;
        _rotatingBand->SetRelativeRotation(rot);
    }

	if (!_isInteracting) return;

    if (_mode == EPlanetGlobeMode::Browse)
        TickBrowseMode(DeltaTime);

    else if (_mode == EPlanetGlobeMode::Focus)
        TickFocusMode(DeltaTime);
}

void AGalacticPlanetGlobe::Interact(AHellDiver* player)
{
    if (_isInteracting) return;

	_isInteracting = true;
    StartInteracting();
}

void AGalacticPlanetGlobe::EnterFocusByOperation(UOperationDataAsset* operation)
{
	if (!operation) return;

    APlanetOperationSite* site = nullptr;

    // 특정 임무를 들고있는 사이트 탐색
    for (UChildActorComponent* opSiteComp : _operationSites)
    {
        APlanetOperationSite* opSite = Cast<APlanetOperationSite>(opSiteComp->GetChildActor());
        if (opSite && opSite->GetOperationData() == operation)
			site = opSite;
    }

    if (!site) return;
    _curSite = site;
    _curIcon = nullptr;

    FVector siteLoc = _curSite->GetActorLocation(); // 선택한 지점 위치
    FVector globeCenter = _mesh->GetComponentLocation();

    // 현재 사이트 방향
    FVector siteDir = (siteLoc - globeCenter).GetSafeNormal();

    // 브라우즈 카메라에서부터 글로브만 맞는 채널로 라인트레이스
    FVector rayOrigin = _browseCamera->GetComponentLocation() - FVector(0.f, 0.f, 50.f); // 글로브의 움직임을 고려해 약간 아래에서 시작
    FVector rayDir = _browseCamera->GetChildActor()->GetActorForwardVector();
    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);

	FQuat tempQuat;
    FVector tempLoc;
    // 맞았으면 글로브 browse 모드 회전 목표 저장
    if (hit.bBlockingHit)
    {
        FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        FQuat deltaQuat = FQuat::FindBetweenNormals(siteDir, surfaceNormal); // 사이트가 보이도록 회전하는 쿼터니언
        tempQuat = deltaQuat * _globeRoot->GetComponentQuat(); // 메시 대신 루트 회전
    }
    else
        tempQuat = _globeRoot->GetComponentQuat();

    tempLoc = _globeRoot->GetComponentLocation() + FVector(0.f, 0.f, 50.f);  // 약간 위로

	EnterFocus();

	// EnterFocus에서 변경된 값을 원래대로 복구
	_startGlobeLoc = tempLoc;
	_startGlobeQuat = tempQuat;

	SetCameraView(EPlanetGlobeMode::None);
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

    H_CharacterLoc::SetCharacterToGround(_playerController->GetCharacter(), _playerAnchor, GetWorld());

    _ring->SetActorHiddenInGame(false);

	// 아이콘 선택까지 완전히 끝난 상태였다면 포커스 모드 형태로 진입
    if (_mode == EPlanetGlobeMode::None)
    {
        _mode = EPlanetGlobeMode::Focus;
        if (_exitTimeline && _timelineCurve)
            _exitTimeline->Reverse();
    }

    _playerViewTarget = _playerController->GetViewTarget();
    SetCameraView(_mode);
}

void AGalacticPlanetGlobe::StopInteracting()
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (!GI) return;

    if (_curSite && _curIcon)
    {
		GI->SetOperation(_curSite->GetOperationIndex(), _curSite->GetOperationData());
		GI->SetMission(_curIcon->GetMissionIndex(), _curIcon->GetMissionData());
    }
	else
		GI->SetMission(-1, nullptr);
  //  if (!_curSite || !_curIcon)
		//GI->SetOperationAndMission(nullptr, nullptr);
  //  else
		//GI->SetOperationAndMission(_curSite->GetOperationData(), _curIcon->GetMissionData());

	_isInteracting = false;
    _globeWidget->RemoveFromParent();
	_curIcon = nullptr;

	SetCameraView(EPlanetGlobeMode::None);
    if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(_playerController->GetLocalPlayer()))
    { // IMC 복구
        subsystem->RemoveMappingContext(_globeWidgetIMC);
        subsystem->AddMappingContext(_gameIMC, 10);
	}

    _ring->SetActorHiddenInGame(true);
}

void AGalacticPlanetGlobe::EnterFocus()
{
    if (!_curSite) return;

    SetActorTickEnabled(false);

    _mode = EPlanetGlobeMode::Focus;
    SetCameraView(_mode);
    _ring->GetMesh()->SetGenerateOverlapEvents(false);
    _ring->AttachToComponent(_mesh, FAttachmentTransformRules::KeepWorldTransform); // 링이 글로브에 잠시 붙은 채로 회전하도록

	if (_globeWidget)
        _globeWidget->EnterMissionMode();

    _curSite->ChangeToFocusMode();
    FVector siteLoc = _curSite->GetActorLocation(); // 선택한 지점 위치
    FVector globeCenter = _mesh->GetComponentLocation();

    // 현재 사이트 방향
    FVector siteDir = (siteLoc - globeCenter).GetSafeNormal();

    // 포커스 카메라에서부터 글로브만 맞는 채널로 라인트레이스
    FVector rayOrigin = _focusCamera->GetComponentLocation() + FVector(0.f, 0.f, 50.f); // 글로브의 이후 움직임을 고려해 약간 위에서 시작
    FVector rayDir = _focusCamera->GetChildActor()->GetActorForwardVector();
    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);

    // 맞았으면 사이트가 보이도록 글로브 회전 목표 지정
    if (hit.bBlockingHit)
    {
        FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        FQuat deltaQuat = FQuat::FindBetweenNormals(siteDir, surfaceNormal); // 사이트가 보이도록 회전하는 쿼터니언
        _focusedGlobeRotation = deltaQuat * _mesh->GetComponentQuat(); // 포커스 모드에서의 글로브 기본 회전 상태
        _targetGlobeQuat = deltaQuat * _globeRoot->GetComponentQuat(); // 메시 대신 루트 회전
    }
    else
        _targetGlobeQuat = _globeRoot->GetComponentQuat();

    _startGlobeQuat = _globeRoot->GetComponentQuat();
    _startGlobeLoc = _globeRoot->GetComponentLocation();
    _targetGlobeLoc = _startGlobeLoc + FVector(0.f, 0.f, -50.f);  // 약간 아래로

    if (_focusTimeline && _timelineCurve)
    {
        _focusTimeline->PlayFromStart();
    }
}

void AGalacticPlanetGlobe::ExitFocus()
{
    SetActorTickEnabled(false);

    _curSite->ChangeToBrowseMode();
	_curIcon = nullptr;

    _ring->GetMesh()->SetGenerateOverlapEvents(false);
    _ring->AttachToComponent(_mesh, FAttachmentTransformRules::KeepWorldTransform); // 링이 글로브에 잠시 붙은 채로 회전하도록
    _globeWidget->EnterOperationMode();

    _mode = EPlanetGlobeMode::Browse;
    SetCameraView(_mode);

    if (_focusTimeline && _timelineCurve)
    {
        _focusTimeline->Reverse();
    }
}

void AGalacticPlanetGlobe::SelectMission()
{
    if (!_curIcon || !_curSite) return;

	auto GI = Cast<UCGameInstance>(GetGameInstance());
	if (!GI) return;

	auto mission = _curIcon->GetMissionData();
	auto state = GI->GetPreDeployState();
    if (!mission || !state) return;

    if (state->IsMissionCleared(mission))
		return; // 이미 클리어한 미션은 선택 불가

    if (_exitTimeline && _timelineCurve)
    {
        _exitTimeline->PlayFromStart();
    }

	_mode = EPlanetGlobeMode::None;
    StopInteracting();
}

void AGalacticPlanetGlobe::SetCameraView(EPlanetGlobeMode mode)
{
    AActor* cameraActor = nullptr;

    if (mode == EPlanetGlobeMode::Browse)
        cameraActor = Cast<ACameraActor>(_browseCamera->GetChildActor());
    else if (mode == EPlanetGlobeMode::Focus)
        cameraActor = Cast<ACameraActor>(_focusCamera->GetChildActor());
    else
        cameraActor = _playerViewTarget;

    if (cameraActor)
        _playerController->SetViewTargetWithBlend(cameraActor, 0.5f, VTBlend_Cubic);
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
    FVector2D target = _focusedSiteAnchor + delta * _ringSensitivity;
    FVector distanceFromSite = _ring->GetActorLocation() - _curSite->GetActorLocation(); // 현재 링과 현재 지역 간의 거리
    const float distanceSquared = distanceFromSite.SizeSquared();

    FVector2D newRingPos;
    float slerp = 10.f * DeltaTime;
    if (distanceSquared > FMath::Square(_siteZoneRadius)) // 링이 지역에서부터 일정 거리를 벗어날 경우 복귀
    {
        newRingPos = FMath::Lerp(_ringScreenPos, target, slerp);
    }
    else
    {
        newRingPos = _ringScreenPos + delta * _ringSensitivity; // 일정 거리 내에서는 마우스 이동량만큼 이동
    }

    // 링 월드 배치
    FVector rayOrigin, rayDir;
    _playerController->DeprojectScreenPositionToWorld(newRingPos.X, newRingPos.Y, rayOrigin, rayDir);

    FHitResult hit;
    GetWorld()->LineTraceSingleByChannel(hit, rayOrigin, rayOrigin + rayDir * 100000.f, ECC_GameTraceChannel4);
    if (hit.bBlockingHit)
    {
        const FVector surfaceNormal = hit.ImpactNormal.GetSafeNormal();
        const FVector camRight = _playerController->PlayerCameraManager->GetActorRightVector();
        const FVector upTangent = FVector::CrossProduct(camRight, surfaceNormal).GetSafeNormal();
        _ring->PlaceOnSurface(hit.ImpactPoint, surfaceNormal, upTangent);
		_ringScreenPos = newRingPos;    // 링이 글로브 위에 머무르는 범위 내에서 스크린 좌표 갱신
    }

    // 글로브 모션
    float deltaYaw = delta.X * _globeSensitivity;
    float deltaPitch = delta.Y * _globeSensitivity;

    FQuat targetQuat = CalculateNewGlobeQuat(deltaYaw, deltaPitch, _focusedGlobeRotation);

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

void AGalacticPlanetGlobe::OnFocusTimelineUpdate(float value)
{
    FQuat newQuat = FQuat::Slerp(_startGlobeQuat, _targetGlobeQuat, value);
    _globeRoot->SetWorldRotation(newQuat);
    FVector newLoc = FMath::Lerp(_startGlobeLoc, _targetGlobeLoc, value);
    _globeRoot->SetWorldLocation(newLoc);
}

void AGalacticPlanetGlobe::OnExitTimelineUpdate(float value)
{
    FQuat newGlobeQuat = FQuat::Slerp(_targetGlobeQuat, _startGlobeQuat, value);
    _globeRoot->SetWorldRotation(newGlobeQuat);
    FVector newGlobeLoc = FMath::Lerp(_targetGlobeLoc, _startGlobeLoc, value);
	_globeRoot->SetWorldLocation(newGlobeLoc);

    if (_rotatingBand)
    {
        FVector newBandLoc = FMath::Lerp(_startBandLoc, _targetBandLoc, value);
        _rotatingBand->SetRelativeLocation(newBandLoc);
		float newBandScale = FMath::Lerp(1.f, 1.2f, value);
		_rotatingBand->SetRelativeScale3D(FVector(newBandScale));
	}
}

void AGalacticPlanetGlobe::OnFocusTimelineFinished()
{
    // 앵커 및 링 초기 위치 설정
    _playerController->ProjectWorldLocationToScreen(_curSite->GetActorLocation(), _focusedSiteAnchor, true);
    _playerController->ProjectWorldLocationToScreen(_ring->GetActorLocation(), _ringScreenPos, true);

    // 원상태로 복구
    _ring->GetMesh()->SetGenerateOverlapEvents(true);
    _ring->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorTickEnabled(true);
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
        ExitFocus();
        return;
    }
    if (_mode == EPlanetGlobeMode::Browse)
    {
        StopInteracting();
        return;
    }
}

