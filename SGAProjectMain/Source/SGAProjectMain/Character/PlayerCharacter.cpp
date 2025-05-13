// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
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

#include "../GunBase.h"
#include "../UI/GunUI.h"

#include "HellDiver/HellDiver.h"
#include "HellDiver/HellDiverStateComponent.h"

#include "../Data/PlayerControlDataAsset.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));


	_springArm->SetupAttachment(GetCapsuleComponent());
	_camera->SetupAttachment(_springArm);



}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (_gunWidgetClass)
	{
		_gunWidget = CreateWidget<UGunUI>(GetWorld(), _gunWidgetClass);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetDefaultView();
	if (_gunClass)
	{
		_equippedGun = GetWorld()->SpawnActor<AGunBase>(_gunClass);
		if (_equippedGun)
		{
			_equippedGun->SetOwner(this);

			_stateComponent->SetWeaponState(EWeaponType::PrimaryWeapon); // 임시 세팅

			//if (_gunWidget)
			{
				_equippedGun->_ammoChanged.AddUObject(_gunWidget, &UGunUI::SetAmmo);
				_gunWidget->AddToViewport();
			}
		}
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
		enhancedInputComponent->BindAction(_reloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Reload);
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D moveVector = value.Get<FVector2D>();

	if (Controller != nullptr && moveVector.Length() > 0.01f)
	{
		if (_viewType == ECharacterViewType::Default)
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
	switch (_playerState)
	{
	case EPlayerState::Idle:
		_playerState = EPlayerState::Firing;
		_equippedGun->StartFire();
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
		_equippedGun->StopFire();
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

void APlayerCharacter::TrySprint(const FInputActionValue& value)
{
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
	_isAiming = true;

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

	if (_viewType == ECharacterViewType::Default)
	{
		SetTPSView();

	}
	else if (_viewType == ECharacterViewType::TPS)
	{
		SetFPSView();

	}
	else if (_viewType == ECharacterViewType::FPS)
	{
		SetDefaultView();
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
void APlayerCharacter::SetFPSView()
{
	if (_fpsControl == nullptr)
		return;
	SetViewData(_fpsControl);
	_viewType = ECharacterViewType::FPS;

}
void APlayerCharacter::SetTPSView()
{
	if (_tpsControl == nullptr)
		return;
	SetViewData(_tpsControl);
	_viewType = ECharacterViewType::TPS;
}
void APlayerCharacter::SetDefaultView()
{
	if (_defaultControl == nullptr)
		return;
	UE_LOG(LogTemp, Error, TEXT("DefaultView"));
	SetViewData(_defaultControl);
	_viewType = ECharacterViewType::Default;
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



	//카메라
	_springArm->TargetArmLength = characterControlData->TargetArmLength;
	_springArm->SetRelativeRotation(characterControlData->RelativeRotation);
	_springArm->SetRelativeLocation(characterControlData->RelativeLocation);
	_springArm->bUsePawnControlRotation = characterControlData->bUsePawnContolRotation;
	_springArm->bDoCollisionTest = characterControlData->bDoCollisionTest;
	_springArm->bInheritPitch = characterControlData->bInheritPitch;
	_springArm->bInheritYaw = characterControlData->bInheritYaw;
	_springArm->bInheritRoll = characterControlData->bInheritRoll;



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
void APlayerCharacter::StopAiming(const FInputActionValue& value)
{
	_isAiming = false;

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

void APlayerCharacter::Reload(const FInputActionValue& value)
{
	_equippedGun->Reload();
}
