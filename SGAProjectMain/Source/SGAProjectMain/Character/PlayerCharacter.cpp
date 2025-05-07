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

	_springArm->TargetArmLength = 500.0f;
	_springArm->SetRelativeRotation(FRotator(-35.0f, 0.0f, 0.0f));
	_springArm->bUsePawnControlRotation = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (_gunClass)
	{
		_equippedGun = GetWorld()->SpawnActor<AGunBase>(_gunClass);
		if (_equippedGun)
		{
			_equippedGun->SetOwner(this);
			_stateComponent->SetWeaponState( EWeaponType::PrimaryWeapon);
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
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFiring);
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Triggered, this, &APlayerCharacter::WhileFiring);
		enhancedInputComponent->BindAction(_mouseLButtonAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopFiring);

		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Started, this, &APlayerCharacter::StartAiming);
		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Triggered, this, &APlayerCharacter::WhileAiming);
		enhancedInputComponent->BindAction(_mouseRButtonAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopAiming);
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D moveVector = value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (moveVector.Length() > 0.01f)
		{

			UpDown(moveVector.Y);
			RightLeft(moveVector.X);

			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
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
		
		// 카메라 오른쪽을 넘어감, 나는 정면
		if (_deltaAngle > 90.0f)
		{
			_isTurnRight = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
		// 카메라 왼쪽을 넘어감, 나는 정면
		else if (_deltaAngle < -90.0f)
		{
			_isTurnLeft = true;
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
		// 움직이거나, 공격중일때..
		else if (GetCharacterMovement()->Velocity.Size() > 0.1f //|| IsAttack()
			)
		{
			GetCharacterMovement()->bUseControllerDesiredRotation = true;
		}
		// 카메라, 정면 각도 차이의 절대값이 0.1 미만
		else if (FMath::Abs(_deltaAngle) < 0.1f)
		{
			_isTurnLeft = false;
			_isTurnRight = false;
			GetCharacterMovement()->bUseControllerDesiredRotation = false;
		}



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
		_stateComponent->StartSprint();
		break;
	case ECharacterState::Sprinting:
		break;
	case ECharacterState::Crouching:
		_stateComponent->FinishCrouch();
		break;
	case ECharacterState::Proning:
		_stateComponent->FinishProne();
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
		_stateComponent->FinishSprint();
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
		_stateComponent->StartCrouch();
		break;
	case ECharacterState::Crouching:
		_stateComponent->FinishCrouch();
		break;
	case ECharacterState::Sprinting:
	case ECharacterState::knockdown:
	case ECharacterState::MAX:
	default:
		break;
	}
}

void APlayerCharacter::TryProne(const FInputActionValue& value)
{
	switch (_stateComponent->GetCharacterState())
	{

	case ECharacterState::Standing:
	case ECharacterState::Crouching:
		_stateComponent->StartProne();
		break;
	case ECharacterState::Proning:
		_stateComponent->FinishProne();
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
}
void APlayerCharacter::SetFPSView()
{
	if (_defaultControl == nullptr)
		return;
	SetViewData(_fpsControl);

}
void APlayerCharacter::SetTPSView()
{
	if (_defaultControl == nullptr)
		return;
	SetViewData(_tpsControl);
}
void APlayerCharacter::SetDefaultView()
{
	if (_defaultControl == nullptr)
		return;
	SetViewData(_defaultControl);
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
	_springArm->bUsePawnControlRotation = characterControlData->bUseControlRotationPitch;
	_springArm->bDoCollisionTest = characterControlData->bDoCollisionTest;
	_springArm->bInheritPitch = characterControlData->bInheritPitch;
	_springArm->bInheritYaw = characterControlData->bInheritYaw;
	_springArm->bInheritRoll = characterControlData->bInheritRoll;



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
