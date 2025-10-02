// Fill out your copyright notice in the Description page of Project Settings.


#include "HellPodBase.h"
#include "Components/PrimitiveComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimTypes.h"
#include "../../Item/ItemBase.h"

// Sets default values
AHellPodBase::AHellPodBase()
{
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(_mesh);

	_currentHp = _maxHp;
	_riseStopToleranceZ = 0;
}

// Called when the game starts or when spawned
void AHellPodBase::BeginPlay()
{
	Super::BeginPlay();

	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		anim->OnPlayMontageNotifyBegin.AddDynamic(this, &AHellPodBase::OnMontageNotifyBegin);
		PlayMontage(_openMontage);
		anim->Montage_JumpToSection(FName("Loop"), _openMontage);
		anim->Montage_Pause(_openMontage);
	}

	if (_mesh)
	{
		_meshHeight = _mesh->Bounds.BoxExtent.Z * 2.0f;
		_targetHeight = GetActorLocation().Z + _meshHeight + _riseStopToleranceZ;
	}
	
	SpawnAndAttachItems();

}

void AHellPodBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float riseSpeed = 300.0f;

	const float z = GetActorLocation().Z;
	if (z < _targetHeight)
	{
		const float step = FMath::Min(riseSpeed * DeltaSeconds, _targetHeight - z);
		SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, z + step));
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

void AHellPodBase::ApplyDamage(float damageAmount)
{
	if (damageAmount <= 0.0f) return;

	_currentHp = FMath::Max(0.0f, _currentHp - damageAmount);
	if (_currentHp == 0.0f)
	{
		OnPodDestroyed();
	}
}

void AHellPodBase::SpawnAndAttachItems()
{
	if (!_itemClass || !_mesh) return;

	const int32 count = GetSocketCount();
	_items.Reserve(count);

	for (int32 i = 1; i <= count; ++i)
	{
		const FName socketName = MakeSocketName(i);
		if (!_mesh->DoesSocketExist(socketName)) continue;

		AItemBase* item = SpawnAndAttachOne(socketName);
		if (!item) continue;

		item->_onPreDespawn.AddDynamic(this, &AHellPodBase::OnItemPreDespawned);

		_items.Add(item);
	}

	_remainingItems = _items.Num();
}

AItemBase* AHellPodBase::SpawnAndAttachOne(const FName& socketName)
{
	if (!_itemClass || !_mesh) return nullptr;

	const FTransform spawnTf = _mesh->GetSocketTransform(socketName, RTS_World);

	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = GetInstigator();
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AItemBase* newItem = GetWorld()->SpawnActor<AItemBase>(_itemClass, spawnTf, params);
	if (!newItem) return nullptr;

	newItem->AttachToComponent(_mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socketName);

	// 스폰 직후에는 떨어지지 않게 기본 잠금
	if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(newItem->GetRootComponent()))
	{
		prim->SetSimulatePhysics(false);
		//prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return newItem;
}

void AHellPodBase::DropAllItemsToGround()
{
	for (AItemBase* item : _items)
	{
		if (!item) continue;

		item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(item->GetRootComponent()))
		{
			prim->SetSimulatePhysics(true);
			prim->SetEnableGravity(true);
			//prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

void AHellPodBase::PlayMontage(UAnimMontage* montage)
{
	if (!_mesh || !montage) return;
	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		anim->Montage_Play(montage, 1.0f);
	}
}

void AHellPodBase::OnAllItemsConsumed()
{
	PlayMontage(_closeMontage);
}

void AHellPodBase::OnPodDestroyed()
{
	// 기본 구현: 파괴직전 남은 아이템 전부 드랍 후 즉시 소멸
	DropAllItemsToGround();
	DestroySelf();
}

FName AHellPodBase::MakeSocketName(int32 indexBased)
{
	const FString prefix = _socketName.ToString();
	return FName(*FString::Printf(TEXT("%s_%d"), *prefix, indexBased));
}

void AHellPodBase::OnMontageNotifyBegin(FName notifyName, const FBranchingPointNotifyPayload& payload)
{
	// 노티가 올라온 소스 애셋(시퀀스/몽타주)
	UAnimMontage* srcMontage = Cast<UAnimMontage>(payload.SequenceAsset);
	
	if (srcMontage == _closeMontage && notifyName == "Close")
	{
		DestroySelf();
	}
	else if (srcMontage == _openMontage && notifyName == "OpenCompleted")
	{
		if (UAnimInstance* anim = _mesh->GetAnimInstance())
		{
			anim->Montage_SetNextSection(FName("Loop"), FName("Loop"), _openMontage);
		}
		
		//PlayMontage(_openIdleMontage);
	}
}

void AHellPodBase::OnItemPreDespawned(AItemBase* item)
{
	if (item)
	{
		_items.RemoveSwap(item);
	}
	_remainingItems = _items.Num();

	if (_remainingItems == 0)
	{
		OnAllItemsConsumed();
	}
}

void AHellPodBase::DestroySelf()
{
	Destroy();
}


