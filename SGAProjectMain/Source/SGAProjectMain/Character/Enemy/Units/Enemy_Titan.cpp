// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Titan.h"
#include "../../StatComponent.h"

AEnemy_Titan::AEnemy_Titan(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

FUnitPartStat* AEnemy_Titan::GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent, FVector hitLoc)
{

	auto datas = _statComponent->GetPartData(part);
	if (datas == nullptr)
		return nullptr;
	//타이탄은 몸통은 등과 옆구리로 나뉘어져있다. 일단은 절반으로 나누자 나중에 시간나면 좀더 세밀하게 쪼개보자
	//다리는 첫번쨰 장갑이 파괴되야 맨살이 나온다 
	switch (part)
	{
	case EBodyPart::Torso:
		break;
	case EBodyPart::Tail:
	{
		FVector location = OverlappedComponent->GetComponentLocation();
		if (hitLoc.Z >= location.Z)
			return &datas->PartStats[0];
		else
			return &datas->PartStats[1];

	}
		break;
	case EBodyPart::LeftArm:
	case EBodyPart::RightArm:
	case EBodyPart::LeftLeg:
	case EBodyPart::RightLeg:
	{
		if ( datas->PartStats[0]._curHP>(int32)0)
			return &datas->PartStats[0];
		else
			return &datas->PartStats[1];
	}
		break;
	case EBodyPart::Core:
	case EBodyPart::Head:
	case EBodyPart::LeftClaw:
	case EBodyPart::RightClaw:
	case EBodyPart::Max:
	default:
	{
		return &datas->PartStats[0];

	}
		break;
	}

	return &datas->PartStats[0];
}
