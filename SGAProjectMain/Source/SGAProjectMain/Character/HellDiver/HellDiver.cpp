// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiver.h"
#include "GameFramework/Character.h" // �̰� �ʿ���
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "HellDiverMovementComponent.h"
#include "HellDiverStateComponent.h"
#include "HellDiverStatComponent.h"

#include "../../Object/Grenade/TimedGrenadeBase.h"
#include "../../Object/Stratagem/Stratagem.h"

#include "../../Data/CollisionCameraDataAsset.h"

AHellDiver::AHellDiver(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UHellDiverMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    GetCharacterMovement()->JumpZVelocity = 300.0f;

    _stateComponent = CreateDefaultSubobject<UHellDiverStateComponent>("State");


    _statComponent = CreateDefaultSubobject<UHellDiverStatComponent>("Stat");
}

void AHellDiver::BeginPlay()
{
    Super::BeginPlay();
    SetCollisionState(_stateComponent->GetCharacterState());

    _stateComponent->_characterStateChanged.AddDynamic(this, &AHellDiver::SetCollisionState);

}

UHellDiverStateComponent* AHellDiver::GetStateComponent()
{
    return _stateComponent;
}

void AHellDiver::EquipGrenade()
{
	if (_heldThrowable || !_grenadeClass)
		return; // �̹� ����ִ�

	GetStateComponent()->SetWeaponState(EWeaponType::Grenade);

	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = this;

	FTransform spawnTransform = GetHandSocketTransform();
	_equippedGrenade = GetWorld()->SpawnActor<ATimedGrenadeBase>(_grenadeClass, spawnTransform, params);
	_heldThrowable = _equippedGrenade;

	if (_heldThrowable)
	{
		// �� ���Ͽ� ����
		_heldThrowable->AttachToHand(TEXT("hand_R")); // ���� �� �ִ� ������Ʈ���� �ñ�
	}
}

void AHellDiver::EquipStratagem()
{
	if (_heldThrowable || _equippedStratagem)
		return;

	GetStateComponent()->SetWeaponState(EWeaponType::StratagemDevice);

	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = this;

	FTransform spawnTransform = GetHandSocketTransform();
	_equippedStratagem = GetWorld()->SpawnActor<AStratagem>(_stratagemClass, spawnTransform, params);
	_heldThrowable = _equippedStratagem;

	if (_heldThrowable)
	{
		UE_LOG(LogTemp, Log, TEXT("Stratagem spawn"));
		_heldThrowable->AttachToHand(TEXT("hand_R"));
	}

}

void AHellDiver::OnThrowReleased()
{
	if (_heldThrowable)
	{
		FRotator throwRot = GetControlRotation();
		throwRot.Pitch += 20.0f; // ��ô ����
		FVector throwDirection = throwRot.Vector();

		_heldThrowable->Throw(throwDirection); // AThrowable ��� �Լ� ȣ��
		_heldThrowable = nullptr;

		_equippedGrenade = nullptr;
		_equippedStratagem = nullptr;

		GetStateComponent()->SetWeaponState(EWeaponType::None);
	}
}

void AHellDiver::StartSprint()
{
    if (_stateComponent->StartSprint() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());
    auto movement=GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetSprintSpeed();
}

void AHellDiver::FinishSprint()
{
    if (_stateComponent->FinishSprint() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::StartCrouch()
{
    if (_stateComponent->StartCrouch() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetCrouchSpeed();
}

void AHellDiver::FinishCrouch()
{
    if (_stateComponent->FinishCrouch() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::StartProne()
{
    if (_stateComponent->StartProne() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());

    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetProneSpeed();
}

void AHellDiver::FinishProne()
{
    if (_stateComponent->FinishProne() == false)
        return;
    SetCollisionState(_stateComponent->GetCharacterState());
    auto movement = GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = _statComponent->GetDefaultSpeed();
}

void AHellDiver::Rolling()
{
    if (this->CanJump() == false)
        return;
    if (_stateComponent->IsRolling())
        return;
    _stateComponent->StartRolling();
    Jump();

    FVector forward = GetActorForwardVector();

    float forwardBoost = 1000.0f; 
    FVector boost = forward * forwardBoost;

    // 4. ���� Velocity�� ������
    GetCharacterMovement()->Velocity += boost;

    StartProne();
}

void AHellDiver::FinishRolling()
{

    _stateComponent->FinishRolling();
}

void AHellDiver::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (_stateComponent->IsRolling())
    {
        // ���� �ð� �� ����
        GetWorld()->GetTimerManager().SetTimer(
            _rollingTimerHandle, this, &AHellDiver::FinishRolling, 0.2, false
        );

    }


}

FTransform  AHellDiver::GetHandSocketTransform() const
{
	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh && mesh->DoesSocketExist(TEXT("hand_R")))
	{
		return mesh->GetSocketTransform(TEXT("hand_R"));
	}
	return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip1SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("EquipSocket_l")))
    {
        return mesh->GetSocketTransform(TEXT("EquipSocket_l"));
    }
    return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip2SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("EquipSocket_r")))
    {
        return mesh->GetSocketTransform(TEXT("EquipSocket_r"));
    }
    return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip3SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("EquipSocket_c")))
    {
        return mesh->GetSocketTransform(TEXT("EquipSocket_c"));
    }
    return GetActorTransform(); // fallback
}

void AHellDiver::SetCollisionState(ECharacterState newState)
{


    switch (newState)
    {
    case ECharacterState::Standing:
    case ECharacterState::Sprinting:
        SetStandingCollisionCamera();
        break;
    case ECharacterState::Crouching:
        SetCrouchingCollisionCamera();
        break;
    case ECharacterState::Proning:
    case ECharacterState::knockdown:
        SetProningCollisionCamera();
        break;
    case ECharacterState::MAX:
    default:
        break;
    }

}

void AHellDiver::SetCollisionCamera(UCollisionCameraDataAsset data)
{
    

}

void AHellDiver::SetStandingCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_standingStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_standingStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_standingStance->_capsuleHalfHeight));
}

void AHellDiver::SetCrouchingCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_crouchingStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_crouchingStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_crouchingStance->_capsuleHalfHeight));
    
}

void AHellDiver::SetProningCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_proningStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_proningStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_proningStance->_capsuleHalfHeight));
}
