// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverInvenComponent.h"

#include "../../Gun/GunBase.h"
#include "../../Gun/GunDataTable.h"

#include "../../Object/Item/Backpack.h"
#include "../../Object/Item/SampleResources.h"
#include "../../Game/PreDeployment/PreDeploymentState.h"
#include "../../CGameInstance.h"

#include "HellDiver.h"

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

	_sampleBundle.Clear();

	_hellDiver = Cast<AHellDiver>(GetOwner());
	
	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	ApplyLoadOut(GI->GetPreDeployState());

	if (!_gunClass1) return;
	SpawnGun(_gunClass1);
	SpawnGun(_gunClass2);
	SpawnGun(_gunClass3);

	_hellDiver->InitWeapon();
}

void UHellDiverInvenComponent::ApplyLoadOut(UPreDeploymentState* preDeployState)
{
	if (!preDeployState) return;

	auto GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;
	
	_gunClass1 = GI->GetGunClassFromTable(preDeployState->GetPrimaryGunID());
	_gunClass2 = GI->GetGunClassFromTable(preDeployState->GetSecondaryGunID());
	_gunClass3 = GI->GetGunClassFromTable(preDeployState->GetSupportGunID());
}

// Called every frame
void UHellDiverInvenComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHellDiverInvenComponent::SpawnGun(TSubclassOf<class AGunBase> gunClass)
{
	AGunBase* gun = GetWorld()->SpawnActor<AGunBase>(gunClass);
	gun->SetOwner(_hellDiver);
	gun->InitializeGun();
	SetGun(gun);
}

int32 UHellDiverInvenComponent::SetGun(AGunBase* gun)
{
	int32 index = -1;

	if (gun->GetGunData()._slotType == EGunSlotType::Primary)
		index = 0;
	else if (gun->GetGunData()._slotType == EGunSlotType::Secondary)
		index = 1;
	else if (gun->GetGunData()._slotType == EGunSlotType::Support)
		index = 2;
	else
		return -1;

	if (_gunSlot[index] != nullptr) // 슬롯에 이미 총이 존재한다면
	{
		DropGun(index);
	}

	_gunSlot[index] = gun;
	PutBackWeapon(gun);
	return index;
}

void UHellDiverInvenComponent::EquipGun(int32 index)
{
	_equippedGun = _gunSlot[index];
	_equippedGun->ActivateGun();
	_equippedGun->AttachToHand();
}

void UHellDiverInvenComponent::DropGun(int32 index)
{
	if (index < 0 || index > 3) return;

	if (!_gunSlot[index]) return;

	auto gun = _gunSlot[index];

	gun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	gun->SetActorEnableCollision(true);
	gun->DeactivateGun();
	gun->SetOwner(nullptr);

	USkeletalMeshComponent* mesh = gun->GetMesh(); // 또는 CustomMesh 이름
	if (mesh)
	{
		mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		mesh->SetSimulatePhysics(true);
		mesh->SetEnableGravity(true);
	}

	gun->SetActorHiddenInGame(false);
	FVector dropLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 10.0f;
	gun->SetActorLocation(dropLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UHellDiverInvenComponent::CanSwitchGun(int32 index)
{
	if (index > 2 || index < 0)
		return false;

	if (_gunSlot[index] == nullptr)
		return false;

	return true;
}

void UHellDiverInvenComponent::EquipBackpack(ABackpack* backpack)
{
	_backpack = backpack;
}

void UHellDiverInvenComponent::DropBackpack()
{
	if (!_backpack) return;

	_backpack->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	_backpack->SetActorEnableCollision(true);
	_backpack->SetOwner(nullptr);

	USkeletalMeshComponent* mesh = _backpack->GetMesh();
	if (mesh)
	{
		mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		mesh->SetSimulatePhysics(true);
		mesh->SetEnableGravity(true);
	}

	FVector dropLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 10.0f;
	_backpack->SetActorLocation(dropLocation);

	_backpack = nullptr;
}

void UHellDiverInvenComponent::AddSample(FSampleBundle sample)
{
	_sampleBundle.AddSample(sample);
}

void UHellDiverInvenComponent::DropSample()
{
	FVector dropLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 10.0f;
	FRotator dropRotation = FRotator::ZeroRotator;

	ASampleResources* sampleBundleActor = GetOwner()->GetWorld()->SpawnActor<ASampleResources>(_sampleClass, dropLocation, dropRotation);

	if (sampleBundleActor)
	{
		sampleBundleActor->SetBundle(_sampleBundle); // 번들(개수) 데이터 전달
		_sampleBundle.Clear(); // 인벤토리 비우기
	}
}

void UHellDiverInvenComponent::PutBackWeapon(AGunBase* gun)
{
	gun->SetActorHiddenInGame(false);

	switch (gun->GetGunData()._slotType)
	{
	case EGunSlotType::Primary:
		PutBackMainWeapon();
		break;
	case EGunSlotType::Secondary:
		PutBackSubWeapon();
		break;
	case EGunSlotType::Support:
		PutBackSupportWeapon();
		break;
	default:
		break;
	}
}

void UHellDiverInvenComponent::PutBackMainWeapon()
{
	if (!_hellDiver)
	{
		return;
	}
	if (USkeletalMeshComponent* characterMesh = _hellDiver->GetMesh())
	{
		// 물리 & 충돌 비활성화
		USkeletalMeshComponent* gunMesh = _gunSlot[0]->GetMesh();
		if (!gunMesh)
		{
			return;
		}
		gunMesh->SetSimulatePhysics(false);
		gunMesh->SetEnableGravity(false);
		gunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		_gunSlot[0]->DetachRootComponentFromParent();
		_gunSlot[0]->AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("cc_mainweapon_socket"));

	}
}

void UHellDiverInvenComponent::PutBackSubWeapon()
{
	if (!_hellDiver)
	{
		return;
	}
	if (USkeletalMeshComponent* characterMesh = _hellDiver->GetMesh())
	{
		// 물리 & 충돌 비활성화
		USkeletalMeshComponent* gunMesh = _gunSlot[1]->GetMesh();
		if (!gunMesh)
		{
			return;
		}
		gunMesh->SetSimulatePhysics(false);
		gunMesh->SetEnableGravity(false);
		gunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


		_gunSlot[1]->DetachRootComponentFromParent();
		_gunSlot[1]->AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("cc_subweapon_socket"));

	}

}

void UHellDiverInvenComponent::PutBackSupportWeapon()
{
	if (!_hellDiver)
	{
		return;
	}
	if (USkeletalMeshComponent* characterMesh = _hellDiver->GetMesh())
	{
		// 물리 & 충돌 비활성화
		USkeletalMeshComponent* gunMesh = _gunSlot[2]->GetMesh();
		if (!gunMesh)
		{
			return;
		}
		gunMesh->SetSimulatePhysics(false);
		gunMesh->SetEnableGravity(false);
		gunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		_gunSlot[2]->DetachRootComponentFromParent();
		_gunSlot[2]->AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("cc_supportweapon_socket"));

	}
}

void UHellDiverInvenComponent::BringWeapon(AGunBase* gun)
{
	gun->AttachToHand();
}

