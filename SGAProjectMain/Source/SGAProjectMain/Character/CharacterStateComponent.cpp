// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateComponent.h"
#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner =Cast<ACharacterBase>( GetOwner());
	
}


// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalcAbnormalityTime(DeltaTime);

	// ...
}

void UCharacterStateComponent::AddAbnormality(EAbnormality abnormality)
{


	uint32 temp = 1 << (uint8)abnormality;
	int damage = 0;
	float time = 0.f;
	EAbnormalityState state = (EAbnormalityState)temp;
	switch (state)
	{
	case EAbnormalityState::Fire:
		damage = 100;
		time = 3.f;
		break;
	case EAbnormalityState::Burn:
		damage = 400;
		time = 1.25f;
		break;
	case EAbnormalityState::Gas:
		damage = 25;
		time = 6.f;
		break;
	case EAbnormalityState::AcidBubble:
		damage = 3;
		time = 3.f;
		break;
	case EAbnormalityState::AcidStream:
		damage = 100;
		time = 0.5f;
		break;
	case EAbnormalityState::bleeding:
		damage = 1;
		time = 99999.f;
		break;
	case EAbnormalityState::Thornbush:
		damage = 5;
		time = 0.1f;
		break;
	case EAbnormalityState::LightStagger:
		time = 0.2f;
		break;
	case EAbnormalityState::StrongStagger:
		time = 1.5f;
		break;
	case EAbnormalityState::Shock:
		time = 3.f;
		break;
	default:
		break;
	}

	_owner->GetStatComponent()->ReceiveDirectDamage(damage);
	//틱이 꺼져있으면 활성화.
	if (PrimaryComponentTick.bCanEverTick == false)
		PrimaryComponentTick.bCanEverTick = true;
		

	
	//지속시간 초기화
	_remainTimes[state] = time;
	bool prevSlow = IsSlow();
	bool prevUnable = IsUnable();
	_activeAbnormalities |= temp;
	bool curSlow = IsSlow();
	bool curUnable = IsUnable();
	if (prevSlow != curSlow || curSlow == true)
	{
		ActiveSlow();

	}
	if (prevUnable != curUnable || curUnable == true)
	{
		ActiveUnable();

	}
	//코어에 직접적인 데미지 입히기.상태이상은 부위와 관련이 읎다.

}

void UCharacterStateComponent::SubAbnormality(EAbnormality abnormality)
{
	if (CheckAbnormality(abnormality) == false)
		return;
	uint32 temp = 1 << (uint8)abnormality;

	bool prevSlow = IsSlow();
	bool prevUnable = IsUnable();
	_activeAbnormalities &= ~temp;
	bool curSlow = IsSlow();
	bool curUnable = IsUnable();

	if (prevSlow != curSlow || curSlow == false)
	{
		DeactiveSlow();

	}
	if (prevUnable != curUnable || curUnable == false)
	{
		DeactiveUnable();

	}
}

void UCharacterStateComponent::SubAbnormality(EAbnormalityState abnormality)
{
	uint32 temp = (uint32)abnormality;
	if (CheckAbnormality(temp) == false)
		return;

	bool prevSlow = IsSlow();
	bool prevUnable = IsUnable();
	_activeAbnormalities &= ~temp;
	bool curSlow = IsSlow();
	bool curUnable = IsUnable();

	if (prevSlow != curSlow || curSlow == false)
	{
		DeactiveSlow();

	}
	if (prevUnable != curUnable || curUnable == false)
	{
		DeactiveUnable();

	}
}

bool UCharacterStateComponent::CheckAbnormality(EAbnormality abnormality)
{
	uint32 temp = 1 << (uint8)abnormality;
	return _activeAbnormalities & temp;

}

bool UCharacterStateComponent::CheckAbnormality(uint32 abnormality)
{
	//여러 상태들중 하나라도 겹치는게 있는지.

	return _activeAbnormalities & abnormality;
}

bool UCharacterStateComponent::IsMoving()
{
	if (_owner == nullptr)
		return false;
	return _owner->GetVelocity().Size2D() > 1.0f;;
}

bool UCharacterStateComponent::ActionBegin()
{
	if (_isActing)
		return false;
	_isActing = true;
	return true;
}

bool UCharacterStateComponent::IsUnable()
{
	uint32 temp=(uint32)EAbnormalityState::Shock + (uint32)EAbnormalityState::StrongStagger+ (uint32)EAbnormalityState::LightStagger;
	return CheckAbnormality(temp);
}

bool UCharacterStateComponent::IsSlow()
{

	uint32 temp = (uint32)EAbnormalityState::AcidBubble + (uint32)EAbnormalityState::AcidStream + (uint32)EAbnormalityState::Gas + (uint32)EAbnormalityState::Thornbush;
	return CheckAbnormality(temp);
}

void UCharacterStateComponent::ActiveSlow()
{
	float temp = _owner->GetCharacterMovement()->MaxWalkSpeed;
	//지금 속도에 25퍼센트가 감소될 예정.
	_owner->GetStatComponent()->ChangeSpeed(temp);
}

void UCharacterStateComponent::DeactiveSlow()
{
	float temp = _owner->GetCharacterMovement()->MaxWalkSpeed;
	temp = temp*4.f/ 3.f;
	//원상복구. 속도는 ChangeSpeed에서 슬로우상태면 자동으로 75퍼를 깎지만 지금은 슬로우가 아닐테니 현상태에서 원상복구로 들어감
	_owner->GetStatComponent()->ChangeSpeed(temp);
}

void UCharacterStateComponent::ActiveUnable()
{
	_owner->UnitUnable();
}

void UCharacterStateComponent::DeactiveUnable()
{
	_owner->UnitRecoverFromUnable();
}

void UCharacterStateComponent::CalcAbnormalityTime(float deltaTime)
{
	uint32 flags = _activeAbnormalities;

	int damage = 0;
	while (flags != 0)
	{
		uint32 active = flags & (~flags + 1);
		flags = flags & (flags - 1);          
		damage += CalcActivates(active, deltaTime);
	}

	_owner->GetStatComponent()->ReceiveDirectDamage(damage);
	if(_activeAbnormalities==0)
		PrimaryComponentTick.bCanEverTick = false;
}

int UCharacterStateComponent::CalcActivates(uint32 type,float deltaTime)
{
	EAbnormalityState temp = (EAbnormalityState)type;
	float prev = _remainTimes[temp];
	float cur = FMath::Max(prev - deltaTime, 0.f);
	_remainTimes[temp] = cur;
	if (cur <= 0.f)
	{
		//시간이 종료되면 목록에서 제거
		SubAbnormality(temp);
	}
	//0.25초에 1번 피해 
	if ((int)(prev * 4.f) == (int)(cur * 4.f))
	{
		return 0;
	}
	//피해 반환.
	switch (temp)
	{
	case EAbnormalityState::Fire:
		return 25;
		break;
	case EAbnormalityState::Burn:
		return 20;
		break;
	case EAbnormalityState::Gas:
		return 6;
		break;
	case EAbnormalityState::AcidStream:
		return 5;
		break;
	case EAbnormalityState::bleeding:
		break;
	case EAbnormalityState::None:
	case EAbnormalityState::AcidBubble:
	case EAbnormalityState::StrongStagger:
	case EAbnormalityState::LightStagger:
	case EAbnormalityState::Shock:
	default:
		return 0;
		break;
	}


	return 0;
}

void UCharacterStateComponent::Reset()
{


}

