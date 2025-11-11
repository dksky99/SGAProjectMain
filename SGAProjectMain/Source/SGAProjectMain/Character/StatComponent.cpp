// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"
#include "CharacterStateComponent.h"
#include "UnitDataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../CGameInstance.h"
#include "../Object/CDamageType.h"
#include "CharacterBase.h"

// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
}

void UStatComponent::InitData(FProcessedUnitData* data)
{
	if (data == nullptr)
		return;
	_partDatas = data->_partDatas;
	_battleMovementSpeed= data->_battleMovementSpeed;
	_defaultMovementSpeed= data->_defaultMovementSpeed;
	

}


// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = Cast<ACharacterBase>(GetOwner());

	// 복합적인 피해 판정을 위해 데미지 이벤트와 데미지 타입을 써야해서 이 방식으론 불가. 데미지이벤트를 받아올 방법이 없다.
	// 그러니 TakeDamage에 데미지이벤트를 충돌한 컴포넌트를 포함하게 작성.
	//if (_owner)
	//{
	//	// 포인트 데미지 이벤트 바인딩
	//	_owner->OnTakePointDamage.AddDynamic(this, &UStatComponent::HandlePointDamage);
	//}
	
}


void UStatComponent::Reset()
{
	StartRegen();
}

bool UStatComponent::IsDead()
{
	if (_partDatas[EBodyPart::Core].PartStats.IsValidIndex(0)==false)
	{
		return true;
	}
	if (_partDatas[EBodyPart::Core].PartStats[0]._curHP <=0)
	{
		return true;
	}

	return false;
}

float UStatComponent::GetCurStateSpeed()
{
	
	return GetDefaultSpeed();
}

void UStatComponent::ChangeSpeed(float speed)
{
	float temp = FMath::Max(speed, 0.f);
	if (_owner->GetStateComponent()->IsSlow())
		temp *= 0.75f;
	_owner->GetCharacterMovement()->MaxWalkSpeed = temp;
}
/*
void UStatComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	EBodyPart Part = EBodyPart::Core;

	// 태그 검사로 부위 판별
	if (HitComponent->ComponentHasTag("Head"))
		Part = EBodyPart::Head;
	else if (HitComponent->ComponentHasTag("Torso"))
		Part = EBodyPart::Torso;
	else if (HitComponent->ComponentHasTag("LeftArm"))
		Part = EBodyPart::LeftArm;
	else if (HitComponent->ComponentHasTag("RightArm"))
		Part = EBodyPart::RightArm;
	else if (HitComponent->ComponentHasTag("LeftLeg"))
		Part = EBodyPart::LeftLeg;
	else if (HitComponent->ComponentHasTag("RightLeg"))
		Part = EBodyPart::RightLeg;

	ProcessDamage(Part, Damage);
}
*/
void UStatComponent::ChangeHp(FUnitPartStat* part,float Amount)
{
	//피해양과 영향력으로 코어에 들어가는 데미지를 정한다.
	part->_curHP= FMath::Clamp(part->_curHP + Amount, 0.f, part->_partHP);
	
	if (part->_curHP <= 0.f)
	{
		//파트가 파괴됨. 몇몇 파트는 파괴될 시 사망하는 치명효과가 있음. 파트자체에 델리게이트를 달아보자. 그러면 파트가 파괴될떄 다양한 효과를 낼 수 있을거같다.
		if(part->_onPartDestroyed.IsBound())
			part->_onPartDestroyed.Broadcast();
	}
}

void UStatComponent::ChangeCoreHp(float Amount)
{
	FUnitPartStat* part = &(_partDatas[EBodyPart::Core].PartStats[0]);
	part->_curHP = FMath::Clamp(part->_curHP + Amount, 0.f, part->_partHP);

	if (_coreHpChanged.IsBound())
		_coreHpChanged.Broadcast((float)(part->_curHP));
	if (part->_curHP <= 0.f)
	{
		//파트가 파괴됨. 몇몇 파트는 파괴될 시 사망하는 치명효과가 있음. 파트자체에 델리게이트를 달아보자. 그러면 파트가 파괴될떄 다양한 효과를 낼 수 있을거같다.
		if (part->_onPartDestroyed.IsBound())
			part->_onPartDestroyed.Broadcast();
		
	}
}

void UStatComponent::StartRegen()
{

	for (auto part : _partDatas)
	{
		for (auto layer : part.Value.PartStats)
		{

			layer._curHP = layer._partHP;
		}

	}
	
}

FUnitPartStat* UStatComponent::GetCoreStat()
{
	if(_partDatas.Find(EBodyPart::Core)==nullptr)
		return nullptr;
	if (_partDatas[EBodyPart::Core].PartStats.Num()==0)
		return nullptr;
	return &_partDatas[EBodyPart::Core].PartStats[0];

}


FUnitPartStatArrayWrapper* UStatComponent::GetPartData(EBodyPart part)
{
	FUnitPartStatArrayWrapper* PartDataPtr = _partDatas.Find(part);

	if (PartDataPtr && PartDataPtr->PartStats.Num() > 0)
	{
		return PartDataPtr;
	}

	FUnitPartStatArrayWrapper* CoreDataPtr = _partDatas.Find(EBodyPart::Core);

	if (CoreDataPtr && CoreDataPtr->PartStats.Num() > 0)
	{
		return CoreDataPtr;
	}


	return nullptr;
}

void UStatComponent::ProcessDamage(FUnitPartStat* part, struct FCDamageEvent* damageEvent)
{
	if (part == nullptr || damageEvent == nullptr)
		return;
	// 부위별 현 체력 포인터 및 최대 체력 참조
	int32* CurrentHP = &(part->_curHP);
	int32 MaxHP = part->_partHP;


	//데미지 기초계수
	float damage = 1.f;
	//장갑과 관통레벨에 따른 피해량 적용
	if (part->_partAV == damageEvent->PenetrationLevel)
		damage *= 0.65;
	else if (part->_partAV > damageEvent->PenetrationLevel)
		damage *= 0;
	//	폭발피해일시 폭발 저항력을 적용.
	if (damageEvent->IsExplosionDamage)
	{
		damage *= (1 - part->_partExplosionImmunity);
	}


	if (damage != 0.f)
	{
		//내구성에대한 판정. 
		damage *= (part->_partDurability * damageEvent->DurabilityDamage + (1.f - part->_partDurability) * damageEvent->BaseDamage);
	}


	
}

