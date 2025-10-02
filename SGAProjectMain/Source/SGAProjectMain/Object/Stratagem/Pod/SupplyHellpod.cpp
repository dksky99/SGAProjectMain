// Fill out your copyright notice in the Description page of Project Settings.


#include "SupplyHellpod.h"
#include "../../Item/SupplyBox.h"

// Sets default values
ASupplyHellpod::ASupplyHellpod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

	_currentHp = _maxHp;

}

// Called when the game starts or when spawned
void ASupplyHellpod::BeginPlay()
{
	Super::BeginPlay();
	
	// 소환 시 연출
	PlaySpawnMontage();

	// 보급품 스폰 및 부착
	SpawnAndAttachSupplies();
}

void ASupplyHellpod::ApplyDamage(float damageAmount)
{
	if (damageAmount <= 0.0f) return;

	_currentHp = FMath::Max(0.0f, _currentHp - damageAmount);

	// 체력 0.0f 시 파괴 처리: 남은 아이템 전부 떨구고 소멸
	if (_currentHp == 0.0f)
	{
		DropAllSuppliesToGround();
		DestroySelf();
	}
}

void ASupplyHellpod::SpawnAndAttachSupplies()
{
	if (!_supplyBoxClass || !_mesh) return;

	FString base = TEXT("SupplySocket_");

	for (int32 i = 1; i <= 4; ++i)
	{
		FString nameStr = base + FString::FromInt(i);
		FName socketName(*nameStr);

		if (!_mesh->DoesSocketExist(socketName))
		{
			// 소켓이 없으면 스킵
			continue;
		}

		ASupplyBox* spawned = SpawnAndAttachOne(socketName);
		if (!spawned) continue;

		// 아이템이 사라지기 직전 신호 바인딩
		if (AItemBase* item = Cast<AItemBase>(spawned))
		{
			item->_onPreDespawn.AddDynamic(this, &ASupplyHellpod::OnItemPreDespawned);
		}

		_items.Add(spawned);
	}

	_remainingItems = _items.Num();
}

void ASupplyHellpod::DropAllSuppliesToGround()
{

}

void ASupplyHellpod::PlaySpawnMontage()
{

}

void ASupplyHellpod::PlayDespawnAndDestroy()
{
}

ASupplyBox* ASupplyHellpod::SpawnAndAttachOne(const FName& socketName)
{
	return nullptr;
}

void ASupplyHellpod::OnItemPreDespawned(AItemBase* item)
{
}

void ASupplyHellpod::DestroySelf()
{
}

void ASupplyHellpod::DestroyAfterMontage(UAnimMontage* montage)
{
}

