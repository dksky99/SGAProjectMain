// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverInvenComponent.h"

#include "../../Gun/GunBase.h"
#include "../../Gun/GunDataTable.h"

// Sets default values for this component's properties
UHellDiverInvenComponent::UHellDiverInvenComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	_gunSlot.SetNum(3);
	// ...
}


// Called when the game starts
void UHellDiverInvenComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHellDiverInvenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHellDiverInvenComponent::SetGun(AGunBase* gun)
{
	int32 index = -1;

	if (gun->GetGunData()._slotType == EGunSlotType::Primary)
		index = 0;
	else if (gun->GetGunData()._slotType == EGunSlotType::Secondary)
		index = 1;
	else if (gun->GetGunData()._slotType == EGunSlotType::Support)
		index = 2;
	else
		return;

	if (_gunSlot[index] != nullptr) // 슬롯에 이미 총이 존재한다면
	{
		DropGun(index);
	}

	_gunSlot[index] = gun;
	EquipGun(index);
}

void UHellDiverInvenComponent::EquipGun(int32 index)
{
	_equippedGun = _gunSlot[index];
	_equippedGun->ActivateGun();
}

void UHellDiverInvenComponent::DropGun(int32 index)
{
	if (index < 0 || index > 3) return;

	if (!_gunSlot[index]) return;

	auto gun = _gunSlot[index];

	gun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	gun->SetActorEnableCollision(true);
	gun->SetOwner(nullptr);

	FVector dropLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 10.0f;
	gun->SetActorLocation(dropLocation);
}

bool UHellDiverInvenComponent::CanSwitchGun(int32 index)
{
	if (index > 2 || index < 0)
		return false;

	if (_gunSlot[index] == nullptr)
		return false;

	return true;
}

