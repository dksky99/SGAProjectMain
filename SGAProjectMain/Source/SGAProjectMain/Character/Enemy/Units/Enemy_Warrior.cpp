// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Warrior.h"

#include "../../CharacterStateComponent.h"
#include "../../StatComponent.h"

AEnemy_Warrior::AEnemy_Warrior(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
}

bool AEnemy_Warrior::CheckAbleTryMiddle(AActor* target)
{
    if (target == nullptr)
        return false;
    return false;
}


bool AEnemy_Warrior::TryMiddle(AActor* target)
{

    return false;
}

void AEnemy_Warrior::ResetUnit()
{
    Super::ResetUnit();
    _isBleeding = false;
}

void AEnemy_Warrior::Warrior_Bleeding()
{
    if (_isBleeding)
        return;

    _isBleeding = true;
    //체력을 추가체력만남기고 전부 소거, 
    _statComponent->GetCoreStat()->_curHP = _additiveHP;

    //죽을때까지 출혈피해를 가함.
    GetWorld()->GetTimerManager().SetTimer(_bleedingTimer, this, &AEnemy_Warrior::CalcBleedingDamage, 1.f, false);

}

void AEnemy_Warrior::CalcBleedingDamage()
{

    _statComponent->ChangeCoreHp(-_bleedingDamage);

    //안죽었다면 죽을때까지.
    if(_statComponent->IsDead()==false)
        GetWorld()->GetTimerManager().SetTimer(_bleedingTimer, this, &AEnemy_Warrior::CalcBleedingDamage, 1.f, false);
}

void AEnemy_Warrior::PartInit()
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
        //머리가 깨지면 출혈.
        if (part->PartStats.IsEmpty() == false)
            part->PartStats[0]._onPartDestroyed.AddDynamic(this, &AEnemy_Warrior::Warrior_Bleeding);
    }
}


