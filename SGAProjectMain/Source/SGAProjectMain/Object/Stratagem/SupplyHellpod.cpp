// Fill out your copyright notice in the Description page of Project Settings.


#include "SupplyHellpod.h"
#include "../Item/SupplyBox.h"

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
}

void ASupplyHellpod::SpawnAndAttachSupplies()
{

	if (!_supplyBoxClass || !_mesh)
		return;

	FString base = TEXT("SupplySocket_");

	for (int32 i = 1; i <= 4; ++i)
	{
		FString nameStr = base + FString::FromInt(i);
		FName socketName(*nameStr); 
		if (!_mesh->DoesSocketExist(socketName))
		{
			continue;
		}

	}

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

void ASupplyHellpod::OnSupplyTaken(AActor* byActor)
{
}

void ASupplyHellpod::DestroySelf()
{
}

void ASupplyHellpod::DestroyAfterMontage(UAnimMontage* montage)
{
}

