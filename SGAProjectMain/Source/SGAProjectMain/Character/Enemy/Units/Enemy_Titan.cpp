// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Titan.h"
#include "../../StatComponent.h"

AEnemy_Titan::AEnemy_Titan(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

void AEnemy_Titan::AcidbagDestroyed()
{

    //타이탄의 담즙낭 파괴시 폭발하진 않고 대신 산성토사 기술을 사용할 수 없어짐.
    
    if (GetWorld() && _spitTimer.IsValid())
    {
        // 타이머를 즉시 멈춥니다. 콜백 함수는 실행되지 않습니다.
        GetWorld()->GetTimerManager().ClearTimer(_spitTimer);

        _spitTimer.Invalidate();
    }
    _isSpitReady = false;
}

FUnitPartStat* AEnemy_Titan::GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent, FVector hitLoc)
{

	auto datas = _statComponent->GetPartData(part);
	if (datas == nullptr)
		return nullptr;
	//타이탄은 몸통은 등과 옆구리로 나뉘어져있다. 일단은 절반으로 나누자 나중에 시간나면 좀더 세밀하게 쪼개보자
	switch (part)
	{
        //몸통은 위아래로 등장갑과 옆구리살이있다
    case EBodyPart::Torso:
    {
        FVector location = OverlappedComponent->GetComponentLocation();
        if (hitLoc.Z >= location.Z)
            return &datas->PartStats[0];
        else
        {
            return &datas->PartStats[1];
        }
    }
    break;
    //꼬리는 위로 엉덩이장갑, 아래로 담즙낭과 그속에 내장이있다
	case EBodyPart::Tail:
	{
		FVector location = OverlappedComponent->GetComponentLocation();
		if (hitLoc.Z >= location.Z)
			return &datas->PartStats[0];
        else
        {
            if (datas->PartStats[1]._curHP > (int32)0)
                return &datas->PartStats[1];
            else
                return &datas->PartStats[2];
        }

	}
		break;
	//다리는 첫번쨰 장갑이 파괴되야 맨살이 나온다 
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

	//return &datas->PartStats[0];
}

void AEnemy_Titan::ResetUnit()
{
    Super::ResetUnit();
    _isBleeding = false;
    _isSpitReady = true;
}

void AEnemy_Titan::Titan_Bleeding()
{

    if (_isBleeding)
        return;
    _isBleeding = true;
    //체력을 추가체력만남기고 전부 소거, 
    _statComponent->GetCoreStat()->_curHP = _additiveHP;
}

void AEnemy_Titan::PartInit()
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
        //기본적인 사망효과.
        //머리
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Critical);
    }
    part = partDatas->Find(EBodyPart::Torso);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            //옆구리
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Critical);
        }
    }
    part = partDatas->Find(EBodyPart::Tail);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            //엉덩이 장갑.
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Critical);
            //담즙낭
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::AcidbagDestroyed);
            //내장
            if (part->PartStats.IsValidIndex(2))
                part->PartStats[2]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Critical);
        }
    }
    //다리들.
    part = partDatas->Find(EBodyPart::LeftArm);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Titan_Bleeding);
        }
    }
    part = partDatas->Find(EBodyPart::LeftLeg);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Titan_Bleeding);
        }
    }
    part = partDatas->Find(EBodyPart::RightArm);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Titan_Bleeding);
        }
    }
    part = partDatas->Find(EBodyPart::RightLeg);
    if (part)
    {
        if (part->PartStats.IsEmpty() == false)
        {
            if (part->PartStats.IsValidIndex(1))
                part->PartStats[1]._onPartDestroyed.AddDynamic(this, &AEnemy_Titan::Titan_Bleeding);
        }
    }
}
