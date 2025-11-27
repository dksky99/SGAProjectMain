// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Spewer.h"
#include "../../StatComponent.h"

AEnemy_Spewer::AEnemy_Spewer(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	_errorDegree = 15.0f;
}

FUnitPartStat* AEnemy_Spewer::GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent, FVector hitLoc)
{

	auto datas = _statComponent->GetPartData(part);
	if (datas == nullptr)
		return nullptr;
	//스퓨어는 헤드가 머리와 입으로나뉘어있다.
	if (part == EBodyPart::Head)
	{
		FVector location=OverlappedComponent->GetComponentLocation();
		if(hitLoc.Z>=location.Z)
			return &datas->PartStats[0];
		else
			return &datas->PartStats[1];
	}

	return &datas->PartStats[0];
}

bool AEnemy_Spewer::CheckAbleTryNear(AActor* target)
{

	if (target == nullptr)
		return false;
	return true;
}

bool AEnemy_Spewer::TryNear(AActor* target)
{
	if (CheckAbleTryNear(target) == false)
		return false;
	if (AttackMelee())
		return true;


	return false;
}

void AEnemy_Spewer::PartInit()
{
	ACharacterBase::PartInit();
	if (_statComponent == nullptr)
		return;
	auto partDatas = _statComponent->GetPartDatas();

	if (partDatas->IsEmpty())
		return;
	auto part = partDatas->Find(EBodyPart::Head);
	if (part)
	{

		if (part->PartStats.IsEmpty() == false)
		{
			//머리
			part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Spewer::Critical);
			if (part->PartStats.IsValidIndex(1))
			{
				//입
				part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Spewer::Critical);
			}
		}
		
	}
	part = partDatas->Find(EBodyPart::Torso);
	if (part)
	{
		//등
		if (part->PartStats.IsEmpty() == false)
			part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Spewer::Critical);
	}
	part = partDatas->Find(EBodyPart::Tail);
	if (part)
	{
		//엉덩이 
		if (part->PartStats.IsEmpty() == false)
		{
			part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Spewer::AcidbagDestroyed);
			part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Spewer::Critical);

		}
	}
}




