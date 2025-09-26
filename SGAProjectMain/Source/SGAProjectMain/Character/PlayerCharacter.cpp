// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/CapsuleComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"

#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"

#include "Components/SphereComponent.h"
#include "../Object/Item/ItemBase.h"
#include "../Object/Map/TerminalConsole.h"

#include "../Gun/GunBase.h"
#include "../Gun/ExplosiveGun.h"
#include "../UI/UIManager.h"
#include "../UI/GunWidget.h"
#include "../UI/GunSettingWidget.h"
#include "../UI/StratagemWidget.h"
#include "../UI/MiniMapWidget.h"
#include "../UI/SceneCapturer.h"
#include "../UI/StaminaBarWidget.h"
#include "../UI/CompassWidget.h"
#include "../UI/SampleWidget.h"
#include "../UI/MissionWidget.h"
#include "../UI/InventoryWheelWidget.h"

#include "../Object/Explosive/Grenade/TimedGrenadeBase.h"
#include "../Object/Stratagem/Stratagem.h"
#include "../StratagemComponent.h"

#include "HellDiver/HellDiver.h"
#include "HellDiver/HellDiverStateComponent.h"
#include "HellDiver/PakourComponent.h"
#include "HellDiver/HellDiverStatComponent.h"
#include "HellDiver/HellDiverInvenComponent.h"

#include "../Data/PlayerControlDataAsset.h"
#include "../Data/CollisionCameraDataAsset.h"

#include "../Controller/MainPlayerController.h"
#include "../Controller/CameraContainActor.h"
#include "Perception/AIPerceptionSystem.h"


#include "StimPackComponent.h"


APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	_cameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));

	_tpsSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPSSpringArm"));
	_tpsZoomSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TPSZoomSpringArm"));
	_fpsSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FPSSpringArm"));
	_tpsCameraActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("TPSCamera")) ;
	_tpsZoomCameraActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("TPSZoomCamera")) ;
	_fpsCameraActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("FPSCamera")) ;

	SetRootComponent(GetCapsuleComponent());
	GetMesh()->SetupAttachment(RootComponent);
	_cameraRoot->SetupAttachment(RootComponent); 
	//_cameraRoot->SetupAttachment(GetMesh(), FName("CameraSocket"));
	_tpsSpringArm->SetupAttachment(_cameraRoot);
	_tpsZoomSpringArm->SetupAttachment(_cameraRoot);
	_fpsSpringArm->SetupAttachment(GetMesh(), FName("head"));


	_tpsCameraActor->SetupAttachment(_tpsSpringArm);
	_tpsZoomCameraActor->SetupAttachment(_tpsZoomSpringArm);
	_fpsCameraActor->SetupAttachment(_fpsSpringArm);
	_tpsCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());
	_tpsZoomCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());
	_fpsCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());

	_itemDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemDetectionSphere"));
	_itemDetectionSphere->SetupAttachment(RootComponent);
	_itemDetectionSphere->SetSphereRadius(500.f);
	_itemDetectionSphere->SetGenerateOverlapEvents(true);

	_itemInteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemInteractionSphere"));
	_itemInteractionSphere->SetupAttachment(RootComponent);
	_itemInteractionSphere->SetSphereRadius(100.f);
	_itemInteractionSphere->SetGenerateOverlapEvents(true);

	_aimOffset_.X = 0.0f;
	_aimOffset_.Y = 0.0f;
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (_gunWidgetClass)
		_gunWidget = CreateWidget<UGunWidget>(GetWorld(), _gunWidgetClass);
	if (_stgWidgetClass)
		_stratagemWidget = CreateWidget<UStratagemWidget>(GetWorld(), _stgWidgetClass);
	if (_minimapWidgetClass)
		_minimapWidget = CreateWidget<UMiniMapWidget>(GetWorld(), _minimapWidgetClass);
	if (_staminaBarWidgetClass)
		_staminaBarWidget = CreateWidget<UStaminaBarWidget>(GetWorld(), _staminaBarWidgetClass);
	if (_compassWidgetClass)
		_compassWidget = CreateWidget<UCompassWidget>(GetWorld(), _compassWidgetClass);
	if (_sampleWidgetClass)
		_sampleWidget = CreateWidget<USampleWidget>(GetWorld(), _sampleWidgetClass);
	if (_missionWidgetClass)
		_missionWidget = CreateWidget<UMissionWidget>(GetWorld(), _missionWidgetClass);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	auto statComponent = GetStatComponent();

	SetDefaultVIew();
	//InitView();
	SetTPSView();

	if (_itemDetectionSphere)
	{
		_itemDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnItemInRange);
		_itemDetectionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnItemOutOfRange);
	}
	
	if (_itemInteractionSphere)
	{
		_itemInteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnItemInteractable);
		_itemInteractionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnItemNonInteractable);
	}

	if (_stratagemWidget)
	{
		_stratagemWidget->InitializeWidget(_stratagemComponent->GetStratagemSlots());
		_stratagemWidget->AddToViewport();
		_stratagemWidget->OpenWidget(false);
	}

	if (_minimapWidget)
	{
		_minimapWidget->AddToViewport();
		_minimapWidget->SetVisibility(ESlateVisibility::Hidden);

		// 월드에서 씬캡쳐러 찾기
		for (TActorIterator<ASceneCapturer> IT(GetWorld());IT; ++IT)
		{
			ASceneCapturer* sceneCapturer = *IT;
			if (sceneCapturer)
			{
				_sceneCapturer = sceneCapturer;
				
				_sceneCapturer->_cursorUpdateEvent.AddUObject(_minimapWidget, &UMiniMapWidget::SetCursorText);
				_sceneCapturer->_pingRelativeUpdateEvent.AddUObject(_minimapWidget, &UMiniMapWidget::SetPingImage);
				_sceneCapturer->_pingOnOffEvent.AddUObject(_minimapWidget, &UMiniMapWidget::ShowPingImage);
				
				break;
			}
		}
	}

	if (_staminaBarWidget)
	{
		statComponent->_staminaChanged.AddUObject(_staminaBarWidget, &UStaminaBarWidget::SetStamina);
		_staminaBarWidget->AddToViewport();
		_staminaBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (_compassWidget)
	{
		_compassWidget->AddToViewport();

		if (_sceneCapturer)
		{
			_sceneCapturer->_pingLocationUpdateEvent.AddUObject(_compassWidget, &UCompassWidget::SetPingLocation);
			_sceneCapturer->_pingOnOffEvent.AddUObject(_compassWidget, &UCompassWidget::ShowPingImage);
		}
	}

	if (_sampleWidget)
		_sampleWidget->AddToViewport();

	if (_missionWidget)
		_missionWidget->AddToViewport();


	UAIPerceptionSystem::GetCurrent(GetWorld())->UnregisterSource(*this);


	//if (_sceneUIClass)
	//	UI->GetOrShowSceneUI(_sceneUIClass);

	// 다음 프레임에 실행 (모든 액터 생성 완료 후)
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APlayerCharacter::CheckInitialOverlaps);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto statComponent = GetStatComponent();

	FindBestItem();

	if (_stratagemComponent && _stratagemWidget)
	{
		for (int32 i = 0; i < _stratagemComponent->GetStratagemSlots().Num(); ++i)
		{
			if (_stratagemComponent->IsStratagemOnCooldown(i))
			{
				float remaining = _stratagemComponent->GetRemainingCooldown(i);
				_stratagemWidget->SetWidgetCooldownState(i, remaining);
			}
		}
	}

	if (_stratagemWidget)
	{
		if (_staminaBarWidget->GetVisibility() == ESlateVisibility::Visible)
		{
			// 현재 달리는 상태가 아니고 스태미나가 꽉 차있으면
			if (_stateComponent->GetCharacterState() != ECharacterState::Sprinting && statComponent->IsMaxStamina())
			{
				_staminaBarWidget->SetVisibility(ESlateVisibility::Hidden); // 위젯 감추기
			}
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (enhancedInputComponent)
	{
		enhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		enhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Completed, this, &APlayerCharacter::MoveFinish);
		enhancedInputComponent->BindAction(_lookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TryPakour);
		enhancedInputComponent->BindAction(_sprintAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TrySprint);
		enhancedInputComponent->BindAction(_sprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopSprint);
		enhancedInputComponent->BindAction(_crouchAction, ETriggerEvent::Started, this, &APlayerCharacter::TryCrouch);
		enhancedInputComponent->BindAction(_proneAction, ETriggerEvent::Started, this, &APlayerCharacter::TryProne);
		enhancedInputComponent->BindAction(_rollingAction, ETriggerEvent::Started, this, &APlayerCharacter::TryRolling);
		enhancedInputComponent->BindAction(_testingViewAction, ETriggerEvent::Started, this, &APlayerCharacter::TryChangeControl);
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFiring);
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Triggered, this, &APlayerCharacter::WhileFiring);
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopFiring);
		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Started, this, &APlayerCharacter::StartAiming);
		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Triggered, this, &APlayerCharacter::WhileAiming);
		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopAiming);
		enhancedInputComponent->BindAction(_reloadAction, ETriggerEvent::Started, this, &APlayerCharacter::HoldReload);
		enhancedInputComponent->BindAction(_reloadAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseReload);
		enhancedInputComponent->BindAction(_weapon1ChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchWeapon1);
		enhancedInputComponent->BindAction(_weapon2ChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchWeapon2);
		enhancedInputComponent->BindAction(_weapon3ChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::SwitchWeapon3);
		enhancedInputComponent->BindAction(_grenadeAction, ETriggerEvent::Triggered, this, &AHellDiver::EquipGrenade);
		enhancedInputComponent->BindAction(_lightChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::TryChangeLightMode); // 마우스 휠 아래로
		enhancedInputComponent->BindAction(_scopeChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::TryChangeScopeMode); // 마우스 휠 위로
		enhancedInputComponent->BindAction(_aimChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::ChangeAimingView); // 마우스 휠 누름
		enhancedInputComponent->BindAction(_strataInputModeAction, ETriggerEvent::Started, this, &APlayerCharacter::BeginStratagemInputMode);
		enhancedInputComponent->BindAction(_strataInputModeAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndStratagemInputMode);
		enhancedInputComponent->BindAction(_strataWAction, ETriggerEvent::Started, this, &APlayerCharacter::OnStrataKeyW);
		enhancedInputComponent->BindAction(_strataAAction, ETriggerEvent::Started, this, &APlayerCharacter::OnStrataKeyA);
		enhancedInputComponent->BindAction(_strataSAction, ETriggerEvent::Started, this, &APlayerCharacter::OnStrataKeyS);
		enhancedInputComponent->BindAction(_strataDAction, ETriggerEvent::Started, this, &APlayerCharacter::OnStrataKeyD);
		enhancedInputComponent->BindAction(_interactAction, ETriggerEvent::Started, this, &APlayerCharacter::Interact);
		enhancedInputComponent->BindAction(_stimPackAction, ETriggerEvent::Started, this, &APlayerCharacter::OnUseStimPack);
		enhancedInputComponent->BindAction(_mapAction, ETriggerEvent::Started, this, &APlayerCharacter::OpenMap);
		enhancedInputComponent->BindAction(_inventoryAction, ETriggerEvent::Started, this, &APlayerCharacter::HoldInvenKey);
		enhancedInputComponent->BindAction(_inventoryAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseInvenKey);
	}
}

FTransform APlayerCharacter::GetLeftHandPos()
{

	return FTransform();
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	UAIPerceptionSystem::GetCurrent(GetWorld())->RegisterSource(*this);
}

void APlayerCharacter::UnPossessed()
{
	Super::UnPossessed();

	UAIPerceptionSystem::GetCurrent(GetWorld())->UnregisterSource(*this);
}

FRotator APlayerCharacter::Focusing()
{

	//플레이어 컨트롤러에서 스크린에서 월드좌표받기 
	APlayerController* pc = Cast<APlayerController>(GetController());
	if (pc == nullptr)
	{
		//UE_LOG(LogTemp, Display, TEXT("Legacy"));
		return Focusing_Legacy();
	}
	
	//상태에 따라 기준이 되는 본을 바꿔야함.
	FTransform SpineTransform; 
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::None:
	case EWeaponType::Grenade:
	case EWeaponType::StratagemDevice:

		SpineTransform = GetMesh()->GetSocketTransform(TEXT("FocusingSocket"), RTS_World);


		break;
	case EWeaponType::Gun:
		SpineTransform = GetMesh()->GetSocketTransform(TEXT("weapon_r_muzzle"), RTS_World);
		break;
	default:
		break;

	}
	const FVector SpineLoc = SpineTransform.GetLocation();

	FVector CameraLoc, CameraForward;
	if (APlayerController* controller = Cast<APlayerController>(GetController()))
	{
		CameraLoc = controller->PlayerCameraManager->GetCameraLocation();
		CameraForward = controller->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();
	}
	else
	{
		CameraLoc = GetCurCamera()->GetComponentLocation();
		CameraForward = GetCurCamera()->GetComponentRotation().Vector().GetSafeNormal();
	}

	// 1. 화면의 정중앙 좌표 구하기
	int32 ViewportSizeX, ViewportSizeY;
	pc->GetViewportSize(ViewportSizeX, ViewportSizeY);
	float ScreenCenterX = (float)ViewportSizeX / 2.f;
	float ScreenCenterY = (float)ViewportSizeY / 2.f;

	FVector AimTarget;
	// 2. 화면 좌표를 월드 방향으로 변환
	FVector WorldLocation, WorldDirection;
	if (pc->DeprojectScreenPositionToWorld(ScreenCenterX, ScreenCenterY, WorldLocation, WorldDirection))
	{

		// 3. 라인 트레이스를 정중앙 방향으로 쏨
		FVector TraceEnd = WorldLocation + WorldDirection * 10000;
		FHitResult HitResult;

		FCollisionQueryParams Params;
		Params.bReturnPhysicalMaterial = false;
		Params.AddIgnoredActor(this); // 자기 자신 무시

		if (pc->GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility, Params))
		{
			// 4. 명중 시 → 충돌 지점 반환
			AimTarget= HitResult.ImpactPoint;
			//UE_LOG(LogTemp, Display, TEXT("NotLegacy"));
		}
		else
		{

			AimTarget= TraceEnd;
		}
		// 5. 미명중 시 → 끝 지점 반환
	}
	else
	{
		AimTarget= CameraLoc + CameraForward * 10000.f;
	}


	
	FVector SpineFwd = SpineTransform.GetRotation().Vector().GetSafeNormal();
	FVector SpineUp = SpineTransform.GetRotation().GetUpVector();
	FVector TargetDirection = (AimTarget - SpineLoc).GetSafeNormal();

	// 좌우(Yaw) 필요성 판단
	FVector SpineFwdFlat = FVector::VectorPlaneProject(SpineFwd, SpineUp).GetSafeNormal();
	FVector TargetDirFlat = FVector::VectorPlaneProject(TargetDirection, SpineUp).GetSafeNormal();
	float YawDot = FVector::DotProduct(SpineFwdFlat, TargetDirFlat);
	bool bNeedsYaw = YawDot < 0.999f;

	// 상하(Pitch) 필요성 판단
	FVector SpineRight = SpineTransform.GetRotation().GetRightVector();
	FVector SpineFwdNoYaw = FVector::VectorPlaneProject(SpineFwd, SpineRight).GetSafeNormal();
	FVector TargetDirNoYaw = FVector::VectorPlaneProject(TargetDirection, SpineRight).GetSafeNormal();
	float PitchDot = FVector::DotProduct(SpineFwdNoYaw, TargetDirNoYaw);
	bool bNeedsPitch = PitchDot < 0.999f;

	// 전체 회전 판단 (Roll은 여전히 유용)
	float DotValue = FVector::DotProduct(SpineFwd, TargetDirection);
	FVector CrossValue = FVector::CrossProduct(SpineFwd, TargetDirection);
	float RotationDir = FVector::DotProduct(CrossValue, SpineUp);

	FRotator ResultRot = FRotator::ZeroRotator;

	if (DotValue > 0.999f) // 거의 일치하면 굳이 회전하지 않음
	{
		return FRotator::ZeroRotator;
	}

	ResultRot.Roll = 1.4f - DotValue;

	if (bNeedsYaw)
	{
		ResultRot.Yaw = 1.0f * (RotationDir >= 0 ? 1.f : -1.f);
	}
	if (bNeedsPitch)
	{
		ResultRot.Pitch = 1.0f * ((SpineFwd.Z <= TargetDirection.Z) ? 1.f : -1.f);
	}

	//UE_LOG(LogTemp, Display, TEXT("pitch : %f Yaw : %f Roll : %f Dot : %f"), ResultRot.Pitch, ResultRot.Yaw, ResultRot.Roll, DotValue);
	return ResultRot;
}



FRotator APlayerCharacter::Focusing_Legacy()
{
	//상태에 따라 기준이 되는 본을 바꿔야함.
	FTransform SpineTransform;
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::None:
	case EWeaponType::Grenade:
	case EWeaponType::StratagemDevice:

		SpineTransform = GetMesh()->GetSocketTransform(TEXT("FocusingSocket"), RTS_World);


		break;
	case EWeaponType::Gun:
		SpineTransform = GetMesh()->GetSocketTransform(TEXT("weapon_r_muzzle"), RTS_World);
		break;
	default:
		break;

	}
	const FVector SpineLoc = SpineTransform.GetLocation();

	FVector CameraLoc, CameraForward;
	if (APlayerController* controller = Cast<APlayerController>(GetController()))
	{
		CameraLoc = controller->PlayerCameraManager->GetCameraLocation();
		CameraForward = controller->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();
	}
	else
	{
		CameraLoc = GetCurCamera()->GetComponentLocation();
		CameraForward = GetCurCamera()->GetComponentRotation().Vector().GetSafeNormal();
	}
	//플레이어 컨트롤러에서 스크린에서 월드좌표받기 


	FVector AimTarget = CameraLoc + CameraForward * 10000.f;
	FVector SpineFwd = SpineTransform.GetRotation().Vector().GetSafeNormal();
	FVector SpineUp = SpineTransform.GetRotation().GetUpVector();
	FVector TargetDirection = (AimTarget - SpineLoc).GetSafeNormal();

	// 좌우(Yaw) 필요성 판단
	FVector SpineFwdFlat = FVector::VectorPlaneProject(SpineFwd, SpineUp).GetSafeNormal();
	FVector TargetDirFlat = FVector::VectorPlaneProject(TargetDirection, SpineUp).GetSafeNormal();
	float YawDot = FVector::DotProduct(SpineFwdFlat, TargetDirFlat);
	bool bNeedsYaw = YawDot < 0.999f;

	// 상하(Pitch) 필요성 판단
	FVector SpineRight = SpineTransform.GetRotation().GetRightVector();
	FVector SpineFwdNoYaw = FVector::VectorPlaneProject(SpineFwd, SpineRight).GetSafeNormal();
	FVector TargetDirNoYaw = FVector::VectorPlaneProject(TargetDirection, SpineRight).GetSafeNormal();
	float PitchDot = FVector::DotProduct(SpineFwdNoYaw, TargetDirNoYaw);
	bool bNeedsPitch = PitchDot < 0.999f;

	// 전체 회전 판단 (Roll은 여전히 유용)
	float DotValue = FVector::DotProduct(SpineFwd, TargetDirection);
	FVector CrossValue = FVector::CrossProduct(SpineFwd, TargetDirection);
	float RotationDir = FVector::DotProduct(CrossValue, SpineUp);

	FRotator ResultRot = FRotator::ZeroRotator;

	if (DotValue > 0.999f) // 거의 일치하면 굳이 회전하지 않음
	{
		return FRotator::ZeroRotator;
	}

	ResultRot.Roll = 1.4f - DotValue;

	if (bNeedsYaw)
	{
		ResultRot.Yaw = 1.0f * (RotationDir >= 0 ? 1.f : -1.f);
	}
	if (bNeedsPitch)
	{
		ResultRot.Pitch = 1.0f * ((SpineFwd.Z <= TargetDirection.Z) ? 1.f : -1.f);
	}

	//UE_LOG(LogTemp, Display, TEXT("pitch : %f Yaw : %f Roll : %f Dot : %f"), ResultRot.Pitch, ResultRot.Yaw, ResultRot.Roll, DotValue);
	return ResultRot;
}



void APlayerCharacter::Move(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)// 스트라타젬입력 모드에서는 동작안함
		return;
	if (_stateComponent->GetActionState() == EActionState::InterActing) // 상호작용 중일 때 동작 안 함
		return;
	if (GetCharacterMovement()->IsFalling())
		return;
	if (_stateComponent->IsActionable() == false)
		return;
	if (auto explosiveGun = Cast<AExplosiveGun>(_invenComponent->GetEquippedGun())) // 현재 총이 폭발성일 경우
	{
		if (_stateComponent->IsReloading()) return; // 장전 중에는 움직일 수 없음
	}
	FVector2D moveVector = value.Get<FVector2D>();

	if (Controller != nullptr && moveVector.Length() > 0.01f)
	{
		if (_stateComponent->IsFocusing())
		{
			FocusMove(moveVector);
		}
		else
		{
			DefaultMove(moveVector);

		}


		//FVector MoveDirection = FVector(moveVector.X, moveVector.Y, 0.0f);
		////X축 이동방향으로 캐릭터를회전하기위한 회전 매트릭스 계산식. 
		//GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
		//
		////이동
		//AddMovementInput(MoveDirection, 1);
	}
	//else
	//{
	//	UE_LOG(LogTemp, Display, TEXT("MoveFinish"));
	//	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	//	GetCharacterMovement()->bOrientRotationToMovement = false;
	//	// 멈추는 경우
	//	_vertical = 0.0f;
	//	_horizontal = 0.0f;
	//
	//}
}
void APlayerCharacter::MoveFinish(const FInputActionValue& value)
{

	_vertical = 0.0f;
	_horizontal = 0.0f;
	if (_stateComponent->IsFocusing())
	{

	}
	else
	{

		ViewTurnBack();
		// 멈추는 경우

	}
}
void APlayerCharacter::Look(const FInputActionValue& value)
{
	if (_isDraggingMap)
		return;

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	FVector2D lookAxisVector = value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(lookAxisVector.X);
		AddControllerPitchInput(lookAxisVector.Y);


		



		_deltaAngle = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, GetControlRotation().Yaw);
		

		if (_isViewTurnCenter && _deltaAngle * _deltaAngle <= 0.01)
		{
			_isViewTurnCenter = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;

		}
		const bool bIsMoving = GetVelocity().Size2D() > 1.0f;
		DefaultLook(); // 멈췄을 때 ±90도 넘는 회전 처리
		
	}
}


void APlayerCharacter::TryPakour(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{

		switch (_stateComponent->GetCharacterState())
		{
		case ECharacterState::Sprinting:
		case ECharacterState::Standing:
		case ECharacterState::Crouching:
		{
			//UE_LOG(LogTemp, Display, TEXT("TriggerPakour"));
			_pakourComponent->TriggerPakour();

		}
			break;
		case ECharacterState::Proning:
			FinishProne();
		case ECharacterState::Knockdown:
		case ECharacterState::MAX:
		default:
			break;
		}

	}
}

void APlayerCharacter::StartFiring(const FInputActionValue& value)
{
	if (_stateComponent->IsCheckingMap()) // 지도를 보고있는 상태이고
	{
		if (_sceneCapturer->PingOnMap()) // 핑을 찍을 수 있는 상태라면
			return; // 아래부분 전부 실행 안 함

		// 핑을 찍을 수 없다면 맵을 닫고 다음 행동
		_stateComponent->SetCheckingMap(false);
		_minimapWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	switch (_stateComponent->GetCharacterState())
	{
	case ECharacterState::Sprinting:
		FinishSprint();
		break;
	case ECharacterState::Standing:
	case ECharacterState::Crouching:
	case ECharacterState::Proning:
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
	if (GetCharacterMovement()->bOrientRotationToMovement==true)
	{
		ViewTurnBack();
	}
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::Gun:
		if (_isGunSettingMode)
			return;
		Super::StartFiring();
		break;

	case EWeaponType::Grenade:
		_stateComponent->SetCookingGrenade(true);
		Cast<ATimedGrenadeBase>(_heldThrowable)->StartCookingGrenade();
		StartThrowPreview();
		break;

	case EWeaponType::StratagemDevice:
		_stateComponent->SetInputtingStratagem(true);
		StartThrowPreview();
		break;
	}

	/*switch (_playerState)
	{
	case EPlayerState::Idle:
		if (GetStateComponent()->GetWeaponState() == EWeaponType::PrimaryWeapon)
		{
			_playerState = EPlayerState::Firing;
			_stateComponent->SetFiring(true);
			_equippedGun->StartFire();
		}
		else if (GetStateComponent()->GetWeaponState() == EWeaponType::Grenade)
		{
			_playerState = EPlayerState::CookingGrenade;
			_equippedGrenade->StartCookingGrenade();
			StartThrowPreview();
		}
		else if (GetStateComponent()->GetWeaponState() == EWeaponType::StratagemDevice)
		{
			StartThrowPreview();
		}

		break;

	case EPlayerState::Firing:
		break;

	case EPlayerState::CookingGrenade:
		break;

	case EPlayerState::StratagemInputting:
		if (GetStateComponent()->GetWeaponState() == EWeaponType::StratagemDevice)
		{

		}
		break;

	case EPlayerState::Rolling:
		break;

	case EPlayerState::Reloading:
		break;
	}*/
}

void APlayerCharacter::WhileFiring(const FInputActionValue& value)
{
	if (_stateComponent->IsCookingGrenade())
		if (_heldThrowable)
			Cast<ATimedGrenadeBase>(_heldThrowable)->UpdateCookingGrenade();

	/*switch (_playerState)
	{
	case EPlayerState::Idle:
		break;

	case EPlayerState::Firing:
		break;

	case EPlayerState::CookingGrenade:
		if(_equippedGrenade)
			_equippedGrenade->UpdateCookingGrenade();
		break;

	case EPlayerState::StratagemInputting:
		break;

	case EPlayerState::Rolling:
		break;

	case EPlayerState::Reloading:
		break;
	}*/
}

void APlayerCharacter::StopFiring(const FInputActionValue& value)
{
	if (_stateComponent->IsFiring())
	{
		Super::StopFiring();
		return;
	}
	else if (_stateComponent->IsCookingGrenade())
	{
		_stateComponent->SetCookingGrenade(false);
		Throwing();
		StopThrowPreview();
		return;
	}
	else if (_stateComponent->IsInputtingStratagem())
	{
		_stateComponent->SetInputtingStratagem(false);
		Throwing();
		StopThrowPreview();
		return;
	}

	/*switch (_playerState)
	{
	case EPlayerState::Idle:
		if (GetStateComponent()->GetWeaponState() == EWeaponType::StratagemDevice)
		{
			OnThrowReleased();
			StopThrowPreview();
		}
		break;

	case EPlayerState::Firing:
		_playerState = EPlayerState::Idle;
		_stateComponent->SetFiring(false);
		_equippedGun->StopFire();
		break;

	case EPlayerState::CookingGrenade:
		_playerState = EPlayerState::Idle;
		OnThrowReleased();
		StopThrowPreview();
		break;

	case EPlayerState::StratagemInputting:
		_playerState = EPlayerState::Idle;
		OnThrowReleased();
		StopThrowPreview();
		break;

	case EPlayerState::Rolling:
		break;

	case EPlayerState::Reloading:
		break;
	}*/
}

void APlayerCharacter::TrySprint(const FInputActionValue& value)
{
	if (_viewType!=ECharacterViewType::TPS)
		return;
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
		StartSprint();
		if (_staminaBarWidget)
			_staminaBarWidget->SetVisibility(ESlateVisibility::Visible);
		break;
	case ECharacterState::Sprinting:
		_pakourComponent->TriggerPakour();
		break;
	case ECharacterState::Crouching:
		FinishCrouch();
		break;
	case ECharacterState::Proning:
		FinishProne();
		break;
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}
void APlayerCharacter::StartAiming(const FInputActionValue& value)
{
	if (_isGunSettingMode)
	{
		TryChangeFireMode(value);
		return;
	}

	if (_stateComponent->IsCheckingMap())
	{
		_sceneCapturer->StartDraggingMap();
		_isDraggingMap = true;
		return;
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	if (GetCharacterMovement()->bOrientRotationToMovement == true)
	{
		ViewTurnBack();
	}
	if (_stateComponent->GetCharacterState() == ECharacterState::Sprinting)
		FinishSprint();
	_stateComponent->SetAiming(true);
	SetTPSZoomView();
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::Gun:
		Super::StartAiming();
		break;

	case EWeaponType::Grenade:
		break;

	case EWeaponType::StratagemDevice:
		break;

	case EWeaponType::None:

		break;
	}
}

void APlayerCharacter::StopSprint(const FInputActionValue& value)
{
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Sprinting:
		FinishSprint();
		break;
	case ECharacterState::Standing:
	case ECharacterState::Crouching:
	case ECharacterState::Proning:
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}
void APlayerCharacter::WhileAiming(const FInputActionValue& value)
{
	if (_isGunSettingMode || _stateComponent->IsCheckingMap())
		return;

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::Gun:
		break;

	case EWeaponType::Grenade:
		break;

	case EWeaponType::StratagemDevice:
		break;

	case EWeaponType::None:
		break;
	}
}

void APlayerCharacter::TryCrouch(const FInputActionValue& value)
{
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
	case ECharacterState::Proning:
		StartCrouch();
		break;
	case ECharacterState::Crouching:
		FinishCrouch();
		break;
	case ECharacterState::Sprinting:
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}

void APlayerCharacter::TryChangeControl(const FInputActionValue& value)
{

	if (_viewType == ECharacterViewType::TPS)
	{
		SetTPSZoomView();

	}
	else if (_viewType == ECharacterViewType::TPSZoom)
	{
		SetFPSView();

	}
	else if (_viewType == ECharacterViewType::FPS)
	{
		SetTPSView();
	}


}

void APlayerCharacter::TryProne(const FInputActionValue& value)
{
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
	case ECharacterState::Crouching:
		StartProne();
		break;
	case ECharacterState::Proning:
		FinishProne();
		break;
	case ECharacterState::Sprinting:
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}

void APlayerCharacter::TryRolling(const FInputActionValue& value)
{


	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
	case ECharacterState::Sprinting:
	case ECharacterState::Crouching:
		Rolling();
		break;
	case ECharacterState::Proning:
		FinishProne();
		break;
	case ECharacterState::Knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}
void APlayerCharacter::SetDefaultVIew()
{

	//폰에 관한 설정
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


	//캐릭터 무브먼트 설정
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0, 360, 0);

	_tpsSpringArm->bUsePawnControlRotation = true;
	_tpsZoomSpringArm->bUsePawnControlRotation = true;
	_fpsSpringArm->bUsePawnControlRotation = true;
	_tpsSpringArm->bInheritPitch		= true;
	_tpsSpringArm->bInheritYaw			= true;
	_tpsSpringArm->bInheritRoll			= true;
	_tpsZoomSpringArm->bInheritPitch	= true;
	_tpsZoomSpringArm->bInheritYaw		= true;
	_tpsZoomSpringArm->bInheritRoll		= true;
	_fpsSpringArm->bInheritPitch		= true;
	_fpsSpringArm->bInheritYaw			= true;
	_fpsSpringArm->bInheritRoll			= true;
}

void APlayerCharacter::SetMovingView()
{

	_tpsSpringArm->bUsePawnControlRotation = _defaultControl->bUsePawnContolRotation;
}

void APlayerCharacter::FinishMoving()
{
}

void APlayerCharacter::InitView()
{


	//카메라
	if (_defaultControl != nullptr)
	{
		_tpsSpringArm->bUsePawnControlRotation = _defaultControl->bUsePawnContolRotation;
		_tpsSpringArm->bDoCollisionTest = _defaultControl->bDoCollisionTest;
		_tpsSpringArm->bInheritPitch = _defaultControl->bInheritPitch;
		_tpsSpringArm->bInheritYaw = _defaultControl->bInheritYaw;
		_tpsSpringArm->bInheritRoll = _defaultControl->bInheritRoll;

	}
	//카메라
	if (_tpsControl != nullptr)
	{
		_tpsZoomSpringArm->bUsePawnControlRotation = _tpsControl->bUsePawnContolRotation;
		_tpsZoomSpringArm->bDoCollisionTest = _tpsControl->bDoCollisionTest;
		_tpsZoomSpringArm->bInheritPitch = _tpsControl->bInheritPitch;
		_tpsZoomSpringArm->bInheritYaw = _tpsControl->bInheritYaw;
		_tpsZoomSpringArm->bInheritRoll = _tpsControl->bInheritRoll;

	}
	//1인칭
	if (_fpsControl != nullptr)
	{
		_fpsSpringArm->bUsePawnControlRotation =	_fpsControl->bUsePawnContolRotation;
		_fpsSpringArm->bDoCollisionTest =			_fpsControl->bDoCollisionTest;
		_fpsSpringArm->bInheritPitch =				_fpsControl->bInheritPitch;
		_fpsSpringArm->bInheritYaw =				_fpsControl->bInheritYaw;
		_fpsSpringArm->bInheritRoll =				_fpsControl->bInheritRoll;

	}

}
void APlayerCharacter::SetFPSView()
{
	if (_fpsControl == nullptr)
		return;
	//SetViewData(_fpsControl);
	_viewType = ECharacterViewType::FPS;
	ChangeViewCamera(_viewType);

}
void APlayerCharacter::SetTPSZoomView()
{
	if (_tpsControl == nullptr)
		return;
	//SetViewData(_tpsControl);
	_viewType = ECharacterViewType::TPSZoom;
	ChangeViewCamera(_viewType);
}
void APlayerCharacter::SetTPSView()
{
	if (_defaultControl == nullptr)
		return;
	//SetViewData(_defaultControl);
	_viewType = ECharacterViewType::TPS;
	ChangeViewCamera(_viewType);
}
void APlayerCharacter::SetViewData(const UPlayerControlDataAsset* characterControlData)
{

	//폰에 관한 설정
	bUseControllerRotationPitch = characterControlData->bUseControlRotationPitch;
	bUseControllerRotationYaw = characterControlData->bUseControlRotationYaw;
	bUseControllerRotationRoll = characterControlData->bUseControlRotationRoll;


	//캐릭터 무브먼트 설정
	GetCharacterMovement()->bOrientRotationToMovement = characterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = characterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = characterControlData->RotationRate;






}
void APlayerCharacter::FocusMove(FVector2D moveVector)
{
	if (moveVector.SquaredLength() > 1.0f)
	{
		moveVector.Normalize();
	}

	const FRotator controlRot = Controller->GetControlRotation();
	const FRotator yawRotation(0, controlRot.Yaw, 0);

	const FVector forward = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector right = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	_vertical = moveVector.Y;
	_horizontal = moveVector.X;

	AddMovementInput(forward, moveVector.Y);
	AddMovementInput(right, moveVector.X);
}

void APlayerCharacter::DefaultMove(FVector2D moveVector)
{
	if (moveVector.SquaredLength() > 1.0f)
	{
		moveVector.Normalize();
	}

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	const FRotator controlRot = Controller->GetControlRotation();
	const FRotator yawRotation(0.f, controlRot.Yaw, 0.f);

	const FVector forward = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector right = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	const FVector desiredMoveDir = forward * moveVector.Y + right * moveVector.X;

	if (!desiredMoveDir.IsNearlyZero())
	{
		AddMovementInput(desiredMoveDir.GetSafeNormal(), 1.0f);

		const FVector localMoveDir = GetActorTransform().InverseTransformVectorNoScale(desiredMoveDir);
		_vertical = localMoveDir.X;
		_horizontal = localMoveDir.Y;
	}
}

void APlayerCharacter::MovingLook()
{
	const FVector velocity = GetVelocity();
	if (velocity.Size2D() < 1.0f) return; // 멈춰 있으면 회전 안 함

	const FRotator controlRot = Controller->GetControlRotation();
	const FRotator yawRotation(0, controlRot.Yaw, 0);

	const FVector forward = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector right = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	// 현재 입력 벡터 방향으로 이동 방향 결정
	FVector moveDir = forward * _vertical + right * _horizontal;

	if (!moveDir.IsNearlyZero())
	{
		FRotator moveRot = moveDir.Rotation();
		SetActorRotation(FRotator(0.f, moveRot.Yaw, 0.f)); // 즉시 회전
	}
}
void APlayerCharacter::DefaultLook()
{
	const float speed = GetVelocity().Size2D();
	const FRotator actorRot = GetActorRotation();
	const FRotator controlRot = GetControlRotation();

	const float standard = _deltaAngle;

	if (_stateComponent->GetCharacterState() == ECharacterState::Proning||_stateComponent->IsRolling())
	{
		//UE_LOG(LogTemp, Error, TEXT("ProningLook"));



		return;
	}

	if ( FMath::Abs(standard) > 50.0f||GetCharacterMovement()->Velocity.Size() > 0.01f )
	{
		float targetYaw = FMath::RoundToFloat(controlRot.Yaw / 90.f) * 90.f;
		//UE_LOG(LogTemp, Error, TEXT("DeltaAngle :%f"), standard);

		_isTurnLeft = (standard < -50.f);
		_isTurnRight = (standard > 50.0f);
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

	}
	else if (FMath::Abs(standard) < 1.0f)
	{
		//UE_LOG(LogTemp, Error, TEXT("DeltaAngle :%f"), standard);
		_isTurnLeft = false;
		_isTurnRight = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}
}


void APlayerCharacter::CalcPitch()
{
	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다.
	FTransform spineTransform = GetMesh()->GetSocketTransform(TEXT("spine_01"), RTS_World);
	//각자 메시에서 실제 각방향의 축을 확인.
	FVector spineUp = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();


	//조준점과 가장 가깝고 영향이 큰 본. 
	FTransform aimTransform = GetMesh()->GetSocketTransform(TEXT("spine_03"), RTS_World);
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());
	//본의 정면 벡터를 가져옴.
	FVector aimFwd = temp.GetUnitAxis(EAxis::Y).GetSafeNormal();
	//비교할 방향. 컨트롤러의 방향이나 조준선.
	FVector controlForward = GetCenterLoc() - aimTransform.GetLocation();
	//DrawDebugDirectionalArrow(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation()+aimFwd*50.f, 50.0f, FColor::Green, false, 0.1f, 0, 2.0f);
	//DrawDebugDirectionalArrow(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation()+ controlForward *50.f, 50.0f, FColor::Yellow, false, 0.1f, 0, 2.0f);
	//기준이 될 선.
	FVector charForward = aimFwd;
	controlForward = controlForward.GetSafeNormal();
	charForward = charForward.GetSafeNormal();
	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward= FVector::VectorPlaneProject(charForward, spineRight).GetSafeNormal();
	controlForward= FVector::VectorPlaneProject(controlForward, spineRight).GetSafeNormal();
	


	DrawDebugLine(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation() + controlForward * 500.f, FColor::Yellow, false, 0.1f, 0, 2.0f);
	
	//두 선의 내적으로 일치하는정도를 확인.
	float dot = FVector::DotProduct(charForward, controlForward);
	//라디안으로 변환
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	//각도로 변환
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	//둘을 외적.
	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다. 외적결과와의 내적이니 일치하거나 반대방향이거나 둘중하나가 나옴.
	
	float directionSign = FVector::DotProduct(crossProduct, spineRight);

	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = angleInDegree * FMath::Sign(directionSign);

	_pitch = signedAngle;
	UE_LOG(LogTemp, Display, TEXT("pitch : %f "), _pitch);

}


void APlayerCharacter::CalcYaw()
{

	//메시의 방향을 명백히 해줄 척추의 윗쪽 축과 오른족 축을 기준으로 삼는다.
	FTransform spineTransform = GetMesh()->GetSocketTransform(TEXT("spine_01"), RTS_World);
	//각자 메시에서 실제 각방향의 축을 확인.
	FVector spineUp = spineTransform.GetUnitAxis(EAxis::X).GetSafeNormal(); // 캐릭터 상방
	FVector spineRight = spineTransform.GetUnitAxis(EAxis::Z).GetSafeNormal();
	FVector spineFwd = spineTransform.GetUnitAxis(EAxis::Y).GetSafeNormal();


	//조준점과 가장 가깝고 영향이 큰 본. 
	FTransform aimTransform = GetMesh()->GetSocketTransform(TEXT("spine_03"), RTS_World);
	FTransform temp = spineTransform;
	temp.SetLocation(aimTransform.GetLocation());
	//본의 정면 벡터를 가져옴.
	FVector aimFwd = temp.GetUnitAxis(EAxis::Y).GetSafeNormal();

	//비교할 방향. 컨트롤러의 방향이나 조준선.
	FVector controlForward = GetCenterLoc() - aimTransform.GetLocation();
	//기준이 될 선.
	FVector charForward = aimFwd;
	controlForward=controlForward.GetSafeNormal();
	charForward=charForward.GetSafeNormal();
	UE_LOG(LogTemp, Display, TEXT("controlForward : %f %f %f"), controlForward.X, controlForward.Y, controlForward.Z);

	UE_LOG(LogTemp, Display, TEXT("charForward : %f %f %f"), charForward.X, charForward.Y, charForward.Z);

	// 두 선을 기준이되는 축을 법선으로하는 평면에 투영.
	charForward = FVector::VectorPlaneProject(charForward, spineUp).GetSafeNormal();
	controlForward = FVector::VectorPlaneProject(controlForward, spineUp).GetSafeNormal();

	DrawDebugLine(GetWorld(), aimTransform.GetLocation(), aimTransform.GetLocation() + controlForward * 500.f, FColor::Red, false, 0.1f, 0, 2.0f);


	float dot = FVector::DotProduct(charForward, controlForward);
	float angleInRadians = FMath::Acos(FMath::Clamp(dot, -1.0f, 1.0f));
	float angleInDegree = FMath::RadiansToDegrees(angleInRadians);

	FVector crossProduct = FVector::CrossProduct(charForward, controlForward);

	// 외적 결과 벡터와 평면의 법선(spineUp)을 내적하여 방향을 확인합니다.
	float directionSign = FVector::DotProduct(crossProduct, spineUp);

	// --- 3. 최종 부호 있는 각도 계산 ---
	// DirectionSign이 양수이면 오른쪽(+), 음수이면 왼쪽(-)입니다.
	float signedAngle = angleInDegree * FMath::Sign(directionSign);


	_yaw = signedAngle;
	UE_LOG(LogTemp, Display, TEXT("Yaw : %f "), _yaw);

}


void APlayerCharacter::ChangeViewCamera(ECharacterViewType type)
{
	UChildActorComponent* temp=_tpsCameraActor;
	USpringArmComponent* temp2 = nullptr;
	switch (type)
	{
	case ECharacterViewType::TPS:
		temp = _tpsCameraActor;
		temp2 = _tpsSpringArm;
		_stateComponent->SetAiming(false);
		break;
	case ECharacterViewType::TPSZoom:
		temp = _tpsZoomCameraActor;
		temp2 = _tpsZoomSpringArm;
		
		_stateComponent->SetAiming(true);
		break;
	case ECharacterViewType::FPS:
		temp = _fpsCameraActor;
		temp2 = _fpsSpringArm;
		break;
	default:
		break;
	}
	temp->SetActive(true);
	temp2->SetActive(true);
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && temp && temp->GetChildActor())
	{
		PC->SetViewTargetWithBlend(temp->GetChildActor(), _cameraBlendTime);
	}
	DeactiveAnotherCamera();

}
void APlayerCharacter::DeactiveAnotherCamera()
{
	switch (_viewType)
	{
	case ECharacterViewType::TPS:
		_tpsZoomCameraActor->SetActive(false);
		_tpsZoomSpringArm->SetActive(false);
		_fpsCameraActor->SetActive(false);
		_fpsSpringArm->SetActive(false);
		break;
	case ECharacterViewType::TPSZoom:
		_tpsCameraActor->SetActive(false);
		_tpsSpringArm->SetActive(false);
		_fpsCameraActor->SetActive(false);
		_fpsSpringArm->SetActive(false);
		break;
	case ECharacterViewType::FPS:
		_tpsCameraActor->SetActive(false);
		_tpsSpringArm->SetActive(false);
		_tpsZoomCameraActor->SetActive(false);
		_tpsZoomSpringArm->SetActive(false);
		break;
	case ECharacterViewType::MAX:
	default:
		break;
	}
}
UChildActorComponent* APlayerCharacter::GetCurCamera()
{
	UChildActorComponent* curCamera=_tpsCameraActor;
	switch (_viewType)
	{
	case ECharacterViewType::TPS:
		curCamera = _tpsCameraActor;
		break;
	case ECharacterViewType::TPSZoom:
		curCamera = _tpsZoomCameraActor;
		break;
	case ECharacterViewType::FPS:
		curCamera = _fpsCameraActor;
		break;
	case ECharacterViewType::MAX:
	default:
		break;
	}

	return curCamera;

}
void APlayerCharacter::UpdateCameraOcclusion()
{
	FVector CameraLocation = _camera->GetComponentLocation(); // or CustomCamera
	FVector HeadLocation = GetMesh()->GetSocketLocation("head") + FVector(0, 0, 10.f); // 중심 지점

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceMultiByChannel(
		Hits,
		CameraLocation,
		HeadLocation,
		ECC_Visibility,
		Params
	);

	// 숨길 컴포넌트 처리
	for (const FHitResult& Hit : Hits)
	{
		if (UPrimitiveComponent* Comp = Hit.GetComponent())
		{
			if (!_fadedComponents.Contains(Comp))
			{
				Comp->SetRenderCustomDepth(true); // 또는 투명 머티리얼로 교체
				_fadedComponents.Add(Comp);
			}
		}
	}

	// 이전 프레임에 있었지만 지금은 없는 → 복원
	for (int32 i = _fadedComponents.Num() - 1; i >= 0; --i)
	{
		if (!Hits.ContainsByPredicate([&](const FHitResult& Hit) { return Hit.GetComponent() == _fadedComponents[i]; }))
		{
			_fadedComponents[i]->SetRenderCustomDepth(false); // 또는 원래 머티리얼로 복원
			_fadedComponents.RemoveAt(i);
		}
	}

}
FVector APlayerCharacter::GetCenterLoc()
{
	//플레이어 컨트롤러에서 스크린에서 월드좌표받기 
	APlayerController* pc = Cast<APlayerController>(GetController());
	FVector CameraLoc, CameraForward;
	FVector AimTarget;
	float traceRange = 10000.f;
	if (pc == nullptr)
	{

		CameraLoc = GetCurCamera()->GetComponentLocation();
		CameraForward = GetCurCamera()->GetComponentRotation().Vector().GetSafeNormal();

		// 3. 라인 트레이스를 정중앙 방향으로 쏨
		FVector TraceEnd = CameraLoc + CameraForward * traceRange;
		FHitResult HitResult;

		FCollisionQueryParams Params;
		Params.bReturnPhysicalMaterial = false;
		Params.AddIgnoredActor(this); // 자기 자신 무시

		if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLoc, TraceEnd, ECC_Visibility, Params))
		{
			// 4. 명중 시 → 충돌 지점 반환
			AimTarget = HitResult.ImpactPoint;
			//UE_LOG(LogTemp, Display, TEXT("NotLegacy"));
		}
		else
		{

			AimTarget = TraceEnd;
		}

	}
	else
	{
		CameraLoc = pc->PlayerCameraManager->GetCameraLocation();
		CameraForward = pc->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();

		// 1. 화면의 정중앙 좌표 구하기
		int32 ViewportSizeX, ViewportSizeY;
		pc->GetViewportSize(ViewportSizeX, ViewportSizeY);
		float ScreenCenterX = (float)ViewportSizeX / 2.f;
		float ScreenCenterY = (float)ViewportSizeY / 2.f;

		// 2. 화면 좌표를 월드 방향으로 변환
		FVector WorldLocation, WorldDirection;
		if (pc->DeprojectScreenPositionToWorld(ScreenCenterX, ScreenCenterY, WorldLocation, WorldDirection))
		{

			// 3. 라인 트레이스를 정중앙 방향으로 쏨
			FVector TraceEnd = WorldLocation + WorldDirection * traceRange;
			FHitResult HitResult;

			FCollisionQueryParams Params;
			Params.bReturnPhysicalMaterial = false;
			Params.AddIgnoredActor(this); // 자기 자신 무시

			if (pc->GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility, Params))
			{
				// 4. 명중 시 → 충돌 지점 반환
				AimTarget = HitResult.ImpactPoint;
				//UE_LOG(LogTemp, Display, TEXT("NotLegacy"));
			}
			else
			{

				AimTarget = TraceEnd;
			}
			// 5. 미명중 시 → 끝 지점 반환
		}
		else
		{
			AimTarget = CameraLoc + CameraForward * traceRange;
		}

	}

	//UE_LOG(LogTemp, Display, TEXT("Center Loc: %f %f %f"), AimTarget.X, AimTarget.Y, AimTarget.Z);



	return AimTarget;
}
FVector APlayerCharacter::GetTargetLoc()
{
	return GetCenterLoc();
}
void APlayerCharacter::ViewTurnBack()
{
	UE_LOG(LogTemp, Display, TEXT("TurnBack"));
	AddActorWorldRotation(FRotator(0, 1, 0));
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	// 멈추는 경우
	_isViewTurnCenter = true;
}
void APlayerCharacter::SetStandingCollisionCamera()
{
	Super::SetStandingCollisionCamera();


	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = NAME_None;
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__; // 유니크한 ID

	UKismetSystemLibrary::MoveComponentTo(
		_cameraRoot,
		_standingStance->_cameraOffset,
		_cameraRoot->GetRelativeRotation(),
		true, true,
		0.2f, false,
		EMoveComponentAction::Move,
		LatentInfo
	);


}
void APlayerCharacter::SetCrouchingCollisionCamera()
{
	Super::SetCrouchingCollisionCamera();
	
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = NAME_None;
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__; // 유니크한 ID

	UKismetSystemLibrary::MoveComponentTo(
		_cameraRoot,
		_crouchingStance->_cameraOffset,
		_cameraRoot->GetRelativeRotation(),
		true, true,
		0.2f, false,
		EMoveComponentAction::Move,
		LatentInfo
	);
}
void APlayerCharacter::SetProningCollisionCamera()
{
	Super::SetProningCollisionCamera();

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = NAME_None;
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__; // 유니크한 ID

	UKismetSystemLibrary::MoveComponentTo(
		_cameraRoot,
		_proningStance->_cameraOffset,
		_cameraRoot->GetRelativeRotation(),
		true, true,
		0.2f, false,
		EMoveComponentAction::Move,
		LatentInfo
	);
}

void APlayerCharacter::OpenMap()
{
	if (_stateComponent->IsCheckingMap()) // 맵을 보고있을 경우
	{
		_minimapWidget->SetVisibility(ESlateVisibility::Hidden);
		_stateComponent->SetCheckingMap(false);
		_isDraggingMap = false;
		_sceneCapturer->ResetMap();
		_minimapWidget->ResetMap();
	}
	else // 맵이 닫혀있을 경우
	{
		_minimapWidget->SetVisibility(ESlateVisibility::Visible);
		_stateComponent->SetCheckingMap(true);
	}
}

void APlayerCharacter::AddMissionSlot(UTexture2D* texture, FString name)
{
	_missionWidget->AddMissionSlot(texture, name);
}

void APlayerCharacter::OnPreSwitchGun(AGunBase* prevGun)
{
	prevGun->_ammoChanged.RemoveAll(_gunWidget);
	prevGun->_magChanged.RemoveAll(_gunWidget);
}

void APlayerCharacter::OnPostSwitchGun(AGunBase* newGun)
{
	newGun->_ammoChanged.AddUObject(_gunWidget, &UGunWidget::SetAmmo);
	newGun->_magChanged.AddUObject(_gunWidget, &UGunWidget::SetMag);

	if (_gunWidget)
		_gunWidget->SetGun(newGun->GetGunData()._icon);
}

void APlayerCharacter::InitWeapon()
{
	Super::InitWeapon();

	auto equippedGun = _invenComponent->GetEquippedGun();

	if (_gunWidget)
	{
		equippedGun->_ammoChanged.AddUObject(_gunWidget, &UGunWidget::SetAmmo);
		equippedGun->_magChanged.AddUObject(_gunWidget, &UGunWidget::SetMag);
		_statComponent->_coreHpChanged.AddUObject(_gunWidget, &UGunWidget::SetHp);
		_stimPackComponent->_stimPackChanged.AddUObject(_gunWidget, &UGunWidget::SetStimPack);
		_grenadeChanged.AddUObject(_gunWidget, &UGunWidget::SetGrenade);

		_gunWidget->AddToViewport();

		equippedGun->ActivateGun();
		_stimPackComponent->BroadcastStimPackChanged();
		_gunWidget->SetGun(equippedGun->GetGunData()._icon);
		if (_grenadeChanged.IsBound())
			_grenadeChanged.Broadcast(_curGrenade, _maxGrenade);
	}
}

void APlayerCharacter::SwitchGun(int32 index, const FInputActionValue& value)
{
	if (_isGunSettingMode)
		return;

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	Super::SwitchGun(index);
}

void APlayerCharacter::PickupGun(AGunBase* newGun)
{
	if (_isGunSettingMode)
		return;

	int32 index = _invenComponent->SetGun(newGun);
	if (index == -1) return;

	auto previousGun = _invenComponent->GetEquippedGun();
	previousGun->_ammoChanged.RemoveAll(_gunWidget);
	previousGun->_magChanged.RemoveAll(_gunWidget);

	newGun->_ammoChanged.AddUObject(_gunWidget, &UGunWidget::SetAmmo);
	newGun->_magChanged.AddUObject(_gunWidget, &UGunWidget::SetMag);
	Super::PickupGun(newGun);

	if (_gunWidget)
		_gunWidget->SetGun(newGun->GetGunData()._icon);
}

void APlayerCharacter::HoldInvenKey()
{
	if (!_invenWidgetClass) return;

	_invenWidget = CreateWidget<UInventoryWheelWidget>(GetWorld(), _invenWidgetClass);
	_invenWidget->InitializeWheel(_invenComponent);
	_invenWidget->AddToViewport();
}

void APlayerCharacter::ReleaseInvenKey()
{
	if (!_invenWidget) return;

	const int32 curIndex = _invenWidget->GetCurIndex();

	_invenWidget->RemoveFromParent();
	_invenWidget = nullptr;

	ExecuteInvenAction(curIndex);
}

void APlayerCharacter::ExecuteInvenAction(int32 index)
{
	switch (index)
	{
	case 0: 
		_invenComponent->DropSample();
		_sampleWidget->SetSampleCount(_invenComponent->GetSampleBundle());
		break;
	case 1:
		_invenComponent->DropBackpack();
		 break;
	case 2:
		break;
	case 3:
		_invenComponent->DropGun(2);
		break;
	default: break;
	}
}

void APlayerCharacter::AddSample(FSampleBundle sample)
{
	Super::AddSample(sample);

	if (_sampleWidget)
	{
		const FSampleBundle& sample = _invenComponent->GetSampleBundle();
		_sampleWidget->SetSampleCount(sample);
	}
}

void APlayerCharacter::BeginStratagemInputMode(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::None)
	{
		_stateComponent->SetActionState(EActionState::Stratagem);
		_stratagemInputBuffer.Empty();
	}

	if (_stratagemWidget)
		_stratagemWidget->OpenWidget(true);

	SetTPSZoomView();
}

void APlayerCharacter::EndStratagemInputMode(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)
	{
		_stateComponent->SetActionState(EActionState::None);
		_stratagemInputBuffer.Empty(); // 조합 초기화
	}

	_stratagemWidget->OpenWidget(false);

	SetTPSView();
}

void APlayerCharacter::OnStrataKeyW(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)
	{
		StratagemInputting();
		_stratagemInputBuffer.Add(EKeys::W);
		CheckStratagemInputCombo();
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
	{
		if (!_curTerminal) // 현재 조작 중인 콘솔이 없다면 -> 오류
		{
			EndTerminalInputMode();
			return;
		}

		_curTerminal->ReceiveInput(EKeys::W);
	}
}

void APlayerCharacter::OnStrataKeyA(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)
	{
		StratagemInputting();
		_stratagemInputBuffer.Add(EKeys::A);
		CheckStratagemInputCombo();
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
	{
		if (!_curTerminal) // 현재 조작 중인 콘솔이 없다면 -> 오류
		{
			EndTerminalInputMode();
			return;
		}

		_curTerminal->ReceiveInput(EKeys::A);
	}
}

void APlayerCharacter::OnStrataKeyS(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)
	{
		StratagemInputting();
		_stratagemInputBuffer.Add(EKeys::S);
		CheckStratagemInputCombo();
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
	{
		if (!_curTerminal) // 현재 조작 중인 콘솔이 없다면 -> 오류
		{
			EndTerminalInputMode();
			return;
		}

		_curTerminal->ReceiveInput(EKeys::S);
	}
}

void APlayerCharacter::OnStrataKeyD(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::Stratagem)
	{
		StratagemInputting();
		_stratagemInputBuffer.Add(EKeys::D);
		CheckStratagemInputCombo();
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
	{
		if (!_curTerminal) // 현재 조작 중인 콘솔이 없다면 -> 오류
		{
			EndTerminalInputMode();
			return;
		}

		_curTerminal->ReceiveInput(EKeys::D);
	}
}

void APlayerCharacter::OnUseStimPack(const FInputActionValue& value)
{
	UseStimPack();
}

void APlayerCharacter::CheckStratagemInputCombo()
{
	const TArray<FStratagemSlot>& slots = _stratagemComponent->GetStratagemSlots();
	bool bIsPrefixMatch = false;

	for (int32 i = 0; i < slots.Num(); ++i)
	{
		if (_stratagemComponent->IsStratagemOnCooldown(i))
			continue;

		TSubclassOf<AStratagem> stratagemClass = slots[i].StratagemClass;
		if (!stratagemClass) continue;

		const AStratagem* CDO = stratagemClass->GetDefaultObject<AStratagem>();
		const TArray<FKey>& combo = CDO->GetInputSequence();

		// 완전 일치 → 장비
		if (_stratagemInputBuffer == combo)
		{
			_stratagemComponent->SelectStratagem(i);
			EquipStratagem();

			_stratagemInputBuffer.Empty();

			if (_stateComponent->GetActionState() == EActionState::Stratagem)
			{
				_stateComponent->SetActionState(EActionState::None);
			}
			_stratagemWidget->SetWidgetOperatingState(i);
			return;
		}

		// 사용 가능한 스트라타젬이 있는지 확인
		if (_stratagemInputBuffer.Num() <= combo.Num())
		{
			bool bPrefixMatch = true;
			for (int32 j = 0; j < _stratagemInputBuffer.Num(); ++j)
			{
				if (_stratagemInputBuffer[j] != combo[j])
				{
					bPrefixMatch = false;
					break;
				}
			}
			if (bPrefixMatch)
			{
				bIsPrefixMatch = true;
			}

			_stratagemWidget->UpdateWidget(i, _stratagemInputBuffer.Num(), bPrefixMatch);
		}
	}

	if (!bIsPrefixMatch)
	{
		_stratagemInputBuffer.Empty(); // 조합 초기화
		_stratagemWidget->OpenWidget(false);
	}
}

void APlayerCharacter::BeginTerminalInputMode(ATerminalConsole* terminal)
{
	_stateComponent->SetActionState(EActionState::InterActing);
	_curTerminal = terminal;
}

void APlayerCharacter::EndTerminalInputMode()
{
	_stateComponent->SetActionState(EActionState::None);
	_curTerminal = nullptr;
}

void APlayerCharacter::Interact(const FInputActionValue& value)
{
	if (_bestItem)
	{
		_bestItem->Interact(this);
	}
}

void APlayerCharacter::OnItemInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AInteractable* item = Cast<AInteractable>(OtherActor))
	{
		_detectedItems.AddUnique(item);
		item->ShowDefaultMark();

		UE_LOG(LogTemp, Warning, TEXT("Overlapped with %s"), *GetNameSafe(OtherActor));
	}
}

void APlayerCharacter::OnItemOutOfRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (AInteractable* item = Cast<AInteractable>(OtherActor))
	{
		_detectedItems.Remove(item);
		item->HideMark();

		if (_bestItem == item)
			_bestItem = nullptr;
	}
}

void APlayerCharacter::OnItemInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AInteractable* item = Cast<AInteractable>(OtherActor))
	{
		_interactableItems.AddUnique(item);
	}
}

void APlayerCharacter::OnItemNonInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (AInteractable* item = Cast<AInteractable>(OtherActor))
	{
		_interactableItems.Remove(item);

		if (_detectedItems.Contains(item))
			item->ShowDefaultMark();
		else
			item->HideMark();

		if (_bestItem == item)
			_bestItem = nullptr;
	}
}

void APlayerCharacter::CheckInitialOverlaps()
{
	// 감지 가능 범위 체크
	TArray<AActor*> detectedOverlaps;
	_itemDetectionSphere->GetOverlappingActors(detectedOverlaps, AInteractable::StaticClass());

	for (AActor* actor : detectedOverlaps)
	{
		if (auto item = Cast<AInteractable>(actor))
		{
			_detectedItems.AddUnique(item);
			item->ShowDefaultMark();
		}
	}

	// 상호작용 가능 범위 체크
	TArray<AActor*> interactableOverlaps;
	_itemInteractionSphere->GetOverlappingActors(interactableOverlaps, AInteractable::StaticClass());

	for (AActor* actor : interactableOverlaps)
	{
		if (AInteractable* item = Cast<AInteractable>(actor))
		{
			_interactableItems.AddUnique(item);
		}
	}
}

void APlayerCharacter::FindBestItem()
{
	// 원래는 이 방법을 사용하였으나, 함수가 틱에서 실행되는 것으로 변경되면서, 성능을 위해 방식을 조금 수정
	//TArray<AActor*> overlapped;
	//_itemDetectionSphere->GetOverlappingActors(overlapped, AInteractable::StaticClass());

	// 키를 누를 때마다 GetOverlappingActors()를 실행하는 대신,
	// 아이템이 감지될 때마다 델리게이트로 배열에 추가되고, 그 배열을 검사하는 방식

	//0) 만약 겹친 아이템이 없으면
	if (_interactableItems.Num() == 0)
	{
		_bestItem = nullptr; // 상호작용 불가능
		return;
	}

	// 1) 완전히 겹친 아이템(혹은 거의 동일 위치)에 대해서는 바로 픽업
	for (auto item : _interactableItems)
	{
		/*AInteractable* item = Cast<AInteractable>(actor);
		if (!item) continue;*/
		float dist = FVector::Dist(GetActorLocation(), item->GetActorLocation());
		if (dist <= KINDA_SMALL_NUMBER)
		{
			AInteractable* prevBestItem = _bestItem;
			_bestItem = item;
			_bestItem->ShowKeyButtonMark();

			if (prevBestItem != _bestItem) // 만약 베스트 아이템이 바뀌었다면 UI 갱신
			{
				if (prevBestItem && _interactableItems.Contains(prevBestItem)) // 아직도 상호작용이 가능한 경우라면
					prevBestItem->ShowDefaultMark(); // 다시 일반 상호작용 마크로
			}

			return;  // 가장 먼저 발견된 겹친 아이템만 저장
		}
	}

	// 2) 겹치지 않은 아이템들에 대해 기존 스코어 로직 실행
	//AInteractable* bestItem = nullptr; // 최종 선택할 아이템 포인터
	float bestScore = -1.0f; // 비교용 스코어(클수록 우선)
	const FVector forward = GetActorForwardVector().GetSafeNormal();
	const FVector playerLoc = GetActorLocation();

	for (auto item : _interactableItems)
	{
		/*AInteractable* item = Cast<AInteractable>(actor);
		if (!item) continue;*/
		FVector toItem = item->GetActorLocation() - playerLoc;
		float dist = toItem.Size();

		FVector dir = toItem / dist;
		// 플레이어의 전방 벡터와 아이템 방향 벡터의 내적 계산
		float forwardDot = FVector::DotProduct(forward, dir);
		float score = (forwardDot > 0.0f) ? (forwardDot / dist) : 0.0f;

		if (score > bestScore)
		{
			AInteractable* prevBestItem = _bestItem;
			bestScore = score;
			_bestItem = item;
			_bestItem->ShowKeyButtonMark();

			if (prevBestItem != _bestItem)
			{
				if (prevBestItem && _detectedItems.Contains(prevBestItem))
					prevBestItem->ShowDefaultMark();
			}
		}
	}
}

void APlayerCharacter::StopAiming(const FInputActionValue& value)
{
	if (_isGunSettingMode)
		return;

	if (_stateComponent->IsCheckingMap())
	{
		_sceneCapturer->StopDraggingMap();
		_isDraggingMap = false;
		return;
	}

	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	_stateComponent->SetAiming(false);
	SetTPSView();
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::Gun:
		Super::StopAiming();
		break;

	case EWeaponType::Grenade:
		break;

	case EWeaponType::StratagemDevice:
		break;

	case EWeaponType::None:
		break;
	}
}

void APlayerCharacter::HoldReload(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	_reloadPressedTime = GetWorld()->GetTimeSeconds();
	_isGunSettingMode = false;

	GetWorldTimerManager().SetTimer(_gunSettingTimer, this, &APlayerCharacter::EnterGunSetting, 0.7f, false);
}

void APlayerCharacter::ReleaseReload(const FInputActionValue& value)
{
	if (_stateComponent->GetActionState() == EActionState::InterActing)
		return;

	GetWorldTimerManager().ClearTimer(_gunSettingTimer);

	if (_stateComponent->GetWeaponState() != EWeaponType::Gun)
		return;

	if (_invenComponent->GetEquippedGun())
	{
		if (_isGunSettingMode)
		{
			UE_LOG(LogTemp, Log, TEXT("Exit Gun Setting"));
			
			GetGameInstance()->GetSubsystem<UUIManager>()->ClosePopUp("GunSetting");
			
			_isGunSettingMode = false;
			return;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Reload"));
			Super::Reload();
			return;
		}
	}
}

void APlayerCharacter::EnterGunSetting()
{
	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::Gun)
		return;

	if (_invenComponent->GetEquippedGun())
	{
		_isGunSettingMode = true;

		if (_stateComponent->IsAiming())
			return;

		if (_stateComponent->IsFiring())
			return;

		if (auto widget = GET_WIDGET(UGunSettingWidget, "GunSetting"))
		{
			widget->InitializeWidget(_invenComponent->GetEquippedGun());
		}

		UE_LOG(LogTemp, Log, TEXT("Enter Gun Setting"));
	}
}

void APlayerCharacter::TryChangeFireMode(const FInputActionValue& value)
{
	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::Gun)
		return;

	auto equippedGun = _invenComponent->GetEquippedGun();

	if (equippedGun && _isGunSettingMode)
	{
		equippedGun->ChangeFireMode();
		if (auto widget = GET_WIDGET(UGunSettingWidget, "GunSetting"))
		{
			widget->UpdateFireModePanel(equippedGun->GetCurFireMode());
		}
	}
}

void APlayerCharacter::TryChangeLightMode(const FInputActionValue& value)
{
	if (_stateComponent->IsCheckingMap())
	{
		_sceneCapturer->ChangeOrthoWidth(false); // 마우스 휠 다운 -> 축소
		return;
	}

	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::Gun)
		return;

	auto equippedGun = _invenComponent->GetEquippedGun();

	if (equippedGun && _isGunSettingMode)
	{
		equippedGun->ChangeTacticalLightMode();

		if (auto widget = GET_WIDGET(UGunSettingWidget, "GunSetting"))
		{
			widget->UpdateLightModePanel(equippedGun->GetCurLightMode());
		}
	}
}

void APlayerCharacter::TryChangeScopeMode(const FInputActionValue& value)
{
	if (_stateComponent->IsCheckingMap())
	{
		_sceneCapturer->ChangeOrthoWidth(true); // 마우스 휠 업 -> 확대
		return;
	}

	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::Gun)
		return;

	auto equippedGun = _invenComponent->GetEquippedGun();

	if (equippedGun && _isGunSettingMode)
	{
		equippedGun->ChangeScopeMode();
		if (auto widget = GET_WIDGET(UGunSettingWidget, "GunSetting"))
		{
			widget->UpdateScopeModePanel(equippedGun->GetCurScopeMode());
		}
	}
}

void APlayerCharacter::ChangeAimingView(const FInputActionValue& value)
{
	UE_LOG(LogTemp, Log, TEXT("TryAimChange"));

	bool isAiming = _stateComponent->IsAiming();

	if (!isAiming) return;

	if (_viewType == ECharacterViewType::TPSZoom)
	{
		SetFPSView();
		return;
	}

	if (_viewType == ECharacterViewType::FPS)
	{
		SetTPSZoomView();
		return;
	}
}
