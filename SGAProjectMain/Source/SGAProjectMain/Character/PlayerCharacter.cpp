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

#include "../Gun/GunBase.h"
#include "../UI/GunWidget.h"
#include "../UI/GunSettingWidget.h"

#include "../Object/Grenade/TimedGrenadeBase.h"
#include "../Object/Stratagem/Stratagem.h"

#include "HellDiver/HellDiver.h"
#include "HellDiver/HellDiverStateComponent.h"

#include "../Data/PlayerControlDataAsset.h"
#include "../Data/CollisionCameraDataAsset.h"

#include "../Controller/MainPlayerController.h"
#include "../Controller/CameraContainActor.h"

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
	_tpsSpringArm->SetupAttachment(_cameraRoot);
	_tpsZoomSpringArm->SetupAttachment(_cameraRoot);
	_fpsSpringArm->SetupAttachment(_cameraRoot);


	_tpsCameraActor->SetupAttachment(_tpsSpringArm);
	_tpsZoomCameraActor->SetupAttachment(_tpsZoomSpringArm);
	_fpsCameraActor->SetupAttachment(_fpsSpringArm);
	_tpsCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());
	_tpsZoomCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());
	_fpsCameraActor->SetChildActorClass(ACameraContainActor::StaticClass());


}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	_gunSlot.SetNum(3);

	if (_gunWidgetClass)
	{
		_gunWidget = CreateWidget<UGunWidget>(GetWorld(), _gunWidgetClass);
	}

	if (_gunSettingWidgetClass)
	{
		_gunSettingWidget = CreateWidget<UGunSettingWidget>(GetWorld(), _gunSettingWidgetClass);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitView();
	SetTPSView();
	if (_gunClass1 && _gunClass2)
	{
		// 임시 세팅
		_gunSlot[0] = SpawnGun(_gunClass1);
		_gunSlot[1] = SpawnGun(_gunClass2);
		_gunSlot[2] = SpawnGun(_gunClass3);

		EquipGun(_gunSlot[0]);
	}

	if (_gunWidget)
	{
		_equippedGun->_ammoChanged.AddUObject(_gunWidget, &UGunWidget::SetAmmo);
		_gunWidget->AddToViewport();
	}

	if (_gunSettingWidget)
	{
		_gunSettingWidget->AddToViewport();
		_gunSettingWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (enhancedInputComponent)
	{
		enhancedInputComponent->BindAction(_moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		enhancedInputComponent->BindAction(_lookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		enhancedInputComponent->BindAction(_jumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TryJump);
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
		enhancedInputComponent->BindAction(_stratagemAction, ETriggerEvent::Triggered, this, &AHellDiver::EquipStratagem);
		enhancedInputComponent->BindAction(_lightChangeAction, ETriggerEvent::Started, this, &APlayerCharacter::TryChangeLightMode);
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D moveVector = value.Get<FVector2D>();

	if (Controller != nullptr && moveVector.Length() > 0.01f)
	{
		if (_viewType == ECharacterViewType::TPS)
		{
			DefaultMove(moveVector);
		}
		else
		{
			FocusMove(moveVector);

		}


		//FVector MoveDirection = FVector(moveVector.X, moveVector.Y, 0.0f);
		////X축 이동방향으로 캐릭터를회전하기위한 회전 매트릭스 계산식. 
		//GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
		//
		////이동
		//AddMovementInput(MoveDirection, 1);
	}
	else
	{
		// 멈추는 경우
		_vertical = 0.0f;
		_horizontal = 0.0f;

	}
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D lookAxisVector = value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(lookAxisVector.X);
		AddControllerPitchInput(lookAxisVector.Y);

		 _deltaAngle = FMath::FindDeltaAngleDegrees(GetActorRotation().Yaw, GetControlRotation().Yaw);
		
		//// 카메라 오른쪽을 넘어감, 나는 정면
		//if (_deltaAngle > 90.0f)
		//{
		//	_isTurnRight = true;
		//	GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//}
		//// 카메라 왼쪽을 넘어감, 나는 정면
		//else if (_deltaAngle < -90.0f)
		//{
		//	_isTurnLeft = true;
		//	GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//}
		//// 움직이거나, 공격중일때..
		//else if (GetCharacterMovement()->Velocity.Size() > 0.1f //|| IsAttack()
		//	)
		//{
		//	GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//}
		//// 카메라, 정면 각도 차이의 절대값이 0.1 미만
		//else if (FMath::Abs(_deltaAngle) < 0.1f)
		//{
		//	_isTurnLeft = false;
		//	_isTurnRight = false;
		//	GetCharacterMovement()->bUseControllerDesiredRotation = false;
		//}
		//
		//
		//
	}
}

void APlayerCharacter::TryJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void APlayerCharacter::StartFiring(const FInputActionValue& value)
{
	switch (_stateComponent->GetCharacterState())
	{
	case ECharacterState::Sprinting:
		FinishSprint();
		break;
	case ECharacterState::Standing:
	case ECharacterState::Crouching:
	case ECharacterState::Proning:
	case ECharacterState::knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}

	switch (_playerState)
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
		}
		else if (GetStateComponent()->GetWeaponState() == EWeaponType::StratagemDevice)
		{
			_playerState = EPlayerState::StratagemInputting;
		}

		break;

	case EPlayerState::Firing:
		break;

	case EPlayerState::CookingGrenade:
		break;

	case EPlayerState::StratagemInputting:
		break;

	case EPlayerState::Rolling:
		break;

	case EPlayerState::Reloading:
		break;
	}
}

void APlayerCharacter::WhileFiring(const FInputActionValue& value)
{
	switch (_playerState)
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
	}
}

void APlayerCharacter::StopFiring(const FInputActionValue& value)
{
	switch (_playerState)
	{
	case EPlayerState::Idle:
		break;

	case EPlayerState::Firing:
		_playerState = EPlayerState::Idle;
		_stateComponent->SetFiring(false);
		_equippedGun->StopFire();
		break;

	case EPlayerState::CookingGrenade:
		_playerState = EPlayerState::Idle;
		OnThrowReleased();
		break;

	case EPlayerState::StratagemInputting:
		_playerState = EPlayerState::Idle;
		OnThrowReleased();
		break;

	case EPlayerState::Rolling:
		break;

	case EPlayerState::Reloading:
		break;
	}
}

void APlayerCharacter::TrySprint(const FInputActionValue& value)
{
	if (_stateComponent->IsFiring())
		return;
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
		StartSprint();
		break;
	case ECharacterState::Sprinting:
		break;
	case ECharacterState::Crouching:
		FinishCrouch();
		break;
	case ECharacterState::Proning:
		FinishProne();
		break;
	case ECharacterState::knockdown:
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

	_stateComponent->SetAiming(true);
	SetTPSZoomView();
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::PrimaryWeapon:
		_equippedGun->StartAiming();
		break;

	case EWeaponType::SecondaryWeapon:
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
	case ECharacterState::knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}
void APlayerCharacter::WhileAiming(const FInputActionValue& value)
{
	if (_isGunSettingMode)
		return;

	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::PrimaryWeapon:
		break;

	case EWeaponType::SecondaryWeapon:
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
	case ECharacterState::knockdown:
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
	case ECharacterState::knockdown:
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
	case ECharacterState::knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
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
	SetViewData(_fpsControl);
	_viewType = ECharacterViewType::FPS;
	ChangeViewCamera(_viewType);

}
void APlayerCharacter::SetTPSZoomView()
{
	if (_tpsControl == nullptr)
		return;
	SetViewData(_tpsControl);
	_viewType = ECharacterViewType::TPSZoom;
	ChangeViewCamera(_viewType);
}
void APlayerCharacter::SetTPSView()
{
	if (_defaultControl == nullptr)
		return;
	UE_LOG(LogTemp, Error, TEXT("DefaultView"));
	SetViewData(_defaultControl);
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

	//캐릭터의 로테이션 얻기
	const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0, rotation.Yaw, 0);

	//정면과 우측 벡터 구하기
	const FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);


	_vertical = moveVector.Y;
	_horizontal = moveVector.X;

	//이동
	AddMovementInput(forwardDirection, moveVector.Y);
	AddMovementInput(rightDirection, moveVector.X);
}
void APlayerCharacter::DefaultMove(FVector2D moveVector)
{


	if (moveVector.SquaredLength() > 1.0f)
	{
		moveVector.Normalize();
	}

	// 컨트롤러의 Yaw 회전값만 추출
	const FRotator rotation = Controller->GetControlRotation();
	const FRotator yawRotation(0.f, rotation.Yaw, 0.f);

	// 전방, 우측 벡터 계산 (카메라 기준)
	const FVector forwardDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	// 입력 벡터를 바탕으로 이동 방향 결정
	const FVector desiredMoveDirection =
		forwardDirection * moveVector.Y + rightDirection * moveVector.X;

	if (!desiredMoveDirection.IsNearlyZero())
	{
		// 회전 관련 처리 (이전과 동일)
		const FRotator targetRotation = desiredMoveDirection.Rotation();
		const FRotator currentRotation = GetActorRotation();
		const float rotationSpeed = 10.0f;

		const FRotator newRotation = currentRotation;
		//	FMath::RInterpTo(
		//	currentRotation,
		//	FRotator(0.f, targetRotation.Yaw, 0.f),
		//	GetWorld()->GetDeltaSeconds(),
		//	rotationSpeed
		//);

		SetActorRotation(newRotation);

		// 이동 처리
		AddMovementInput(desiredMoveDirection.GetSafeNormal(), 1.0f);

		// 월드 방향 → 로컬 방향으로 변환
		const FVector localMoveDir = GetActorTransform().InverseTransformVectorNoScale(desiredMoveDirection);

		// 애니메이션 파라미터 갱신
		_vertical = localMoveDir.X;   // 전/후 방향
		_horizontal = localMoveDir.Y; // 좌/우 방향
	}
	else
	{
		_vertical = 0.0f;
		_horizontal = 0.0f;
	}

}

void APlayerCharacter::ChangeViewCamera(ECharacterViewType type)
{
	UChildActorComponent* temp=_tpsCameraActor;

	switch (type)
	{
	case ECharacterViewType::TPS:
		temp = _tpsCameraActor;
		break;
	case ECharacterViewType::TPSZoom:
		temp = _tpsZoomCameraActor;
		break;
	case ECharacterViewType::FPS:
		temp = _fpsCameraActor;
		break;
	default:
		break;
	}
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && temp && temp->GetChildActor())
	{
		PC->SetViewTargetWithBlend(temp->GetChildActor(), _cameraBlendTime);
	}


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

void APlayerCharacter::SwitchWeapon(int32 index, const FInputActionValue& value)
{
	if (index > 3 || index < 0)
		return;

	if (index != 3 && _gunSlot[index] == nullptr)
		return;

	if (_isGunSettingMode)
	{
		_equippedGun->ExitGunSettingMode();
		_isGunSettingMode = false;
	}

	if (_stateComponent->IsReloading())
		_equippedGun->CancelReload();

	bool wasAiming = _stateComponent->IsAiming();
	bool wasFiring = _stateComponent->IsFiring();
	_playerState = EPlayerState::Idle;

	if (index == 3)
	{
		// Grenade
	}
	else
	{
		_equippedGun->_ammoChanged.Clear();
		_equippedGun->DeactivateGun();

		EquipGun(_gunSlot[index]);

		_equippedGun->_ammoChanged.AddUObject(_gunWidget, &UGunWidget::SetAmmo);
		_equippedGun->ActivateGun();
	}

	if (wasAiming) // 에임 중이었을 경우 유지
		StartAiming(value);

	if (wasFiring) // 사격 중이었을 경우 유지
		StartFiring(value);
}

void APlayerCharacter::StopAiming(const FInputActionValue& value)
{
	if (_isGunSettingMode)
		return;

	_stateComponent->SetAiming(false);
	SetTPSView();
	switch (_stateComponent->GetWeaponState())
	{
	case EWeaponType::PrimaryWeapon:
		_equippedGun->StopAiming();
		break;

	case EWeaponType::SecondaryWeapon:
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
	_reloadPressedTime = GetWorld()->GetTimeSeconds();
	_isGunSettingMode = false;

	GetWorldTimerManager().SetTimer(_gunSettingTimer, this, &APlayerCharacter::EnterGunSetting, 0.7f, false);
}

void APlayerCharacter::ReleaseReload(const FInputActionValue& value)
{
	GetWorldTimerManager().ClearTimer(_gunSettingTimer);

	if (_stateComponent->GetWeaponState() != EWeaponType::PrimaryWeapon)
		return;

	if (_equippedGun)
	{
		if (_isGunSettingMode)
		{
			UE_LOG(LogTemp, Log, TEXT("Exit Gun Setting"));
			
			if (_gunSettingWidget)
			{
				_gunSettingWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
			//_equippedGun->ExitGunSettingMode();
			_isGunSettingMode = false;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Reload"));
			_equippedGun->StopAiming();
			_equippedGun->StopFire();
			_equippedGun->Reload();
		}
	}
}

void APlayerCharacter::EnterGunSetting()
{
	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::PrimaryWeapon)
		return;

	if (_equippedGun)
	{
		_isGunSettingMode = true;

		if (_stateComponent->IsAiming())
			return;

		if (_stateComponent->IsFiring())
			return;

		if (_gunSettingWidget)
		{
			_gunSettingWidget->InitializeWidget(_equippedGun);

			_gunSettingWidget->SetVisibility(ESlateVisibility::Visible);
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

	if (_stateComponent->GetWeaponState() != EWeaponType::PrimaryWeapon)
		return;

	if (_equippedGun && _isGunSettingMode)
	{
		_equippedGun->ChangeFireMode();
		_gunSettingWidget->UpdateFireModePanel(_equippedGun->GetCurFireMode());
	}
}

void APlayerCharacter::TryChangeLightMode(const FInputActionValue& value)
{
	if (_stateComponent->IsAiming())
		return;

	if (_stateComponent->IsFiring())
		return;

	if (_stateComponent->GetWeaponState() != EWeaponType::PrimaryWeapon)
		return;

	if (_equippedGun && _isGunSettingMode)
	{
		_equippedGun->ChangeTacticalLightMode();
		_gunSettingWidget->UpdateLightModePanel(_equippedGun->GetCurLightMode());
	}
}
