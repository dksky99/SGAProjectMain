// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiver.h"
#include "GameFramework/Character.h" // 이게 필요함
#include "HellDiverMovementComponent.h"
#include "HellDiverStateComponent.h"

#include "../../Object/Grenade/TimedGrenadeBase.h"
#include "../../Object/Stratagem/Stratagem.h"



AHellDiver::AHellDiver(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UHellDiverMovementComponent>(ACharacter::CharacterMovementComponentName))
{

    _stateComponent = CreateDefaultSubobject<UHellDiverStateComponent>("State");


}

UHellDiverStateComponent* AHellDiver::GetStateComponent()
{
    return _stateComponent;
}

void AHellDiver::EquipGrenade()
{
	if (_heldThrowable || !_grenadeClass)
		return; // 이미 들고있다

	GetStateComponent()->SetWeaponState(EWeaponType::Grenade);

	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = this;

	FTransform spawnTransform = GetHandSocketTransform();
	_equippedGrenade = GetWorld()->SpawnActor<ATimedGrenadeBase>(_grenadeClass, spawnTransform, params);
	_heldThrowable = _equippedGrenade;

	if (_heldThrowable)
	{
		// 손 소켓에 부착
		_heldThrowable->AttachToHand(TEXT("hand_R")); // 던질 수 있는 오브젝트에게 맡김
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
		_heldThrowable->Throw(); // AThrowable 기반 함수 호출
		_heldThrowable = nullptr;

		_equippedGrenade = nullptr;
		_equippedStratagem = nullptr;

		GetStateComponent()->SetWeaponState(EWeaponType::None);
	}
}

void AHellDiver::StartSprint()
{
    auto movement=GetMovementComponent();
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void AHellDiver::FinishSprint()
{
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
