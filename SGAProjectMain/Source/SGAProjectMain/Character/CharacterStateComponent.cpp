// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStateComponent.h"
#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "../Object/CDamageType.h"
#include "../CGameInstance.h"
// Sets default values for this component's properties
UCharacterStateComponent::UCharacterStateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetComponentTickEnabled(false);

	// ...
}


// Called when the game starts
void UCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner =Cast<ACharacterBase>( GetOwner());
	
}

void UCharacterStateComponent::CheckTickOnOff()
{
	if (_activeAbnormalities != 0)
		return;

	if (_activeAbnormalitiesWeight != 0)
		return;

	SetComponentTickEnabled(false);

}


// Called every frame
void UCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CalcAbnormalityWeight(DeltaTime);
	CalcAbnormalityTime(DeltaTime);
	CheckTickOnOff();
	// ...
}

void UCharacterStateComponent::AddAbnormality(EAbnormality abnormality)
{


	uint32 temp = 1 << (uint8)abnormality;
	int damage = 0;
	float time = 0.f;
	EAbnormalityState state = (EAbnormalityState)temp;
	UCGameInstance* gm=GetWorld()->GetGameInstance<UCGameInstance>();
	
	const FProcessedAbnormalityDefinitionData* data= gm->GetProcessedAbnormalityDefinitionData(abnormality);
	if (data == nullptr)
		return;

	float curWeight = _remainWeights.FindOrAdd(state, 0.f);
	float minState = 0.5f;
	float maxState = 1.f;
	switch (data->_abnormalityType)
	{
	case EAbnormalityType::Fire:
		minState = _resistData._minBurn;
		maxState = _resistData._maxBurn;
		break;
	case EAbnormalityType::Gas:
		minState = _resistData._minGas;
		maxState = _resistData._maxGas;
		break;
	case EAbnormalityType::Shock:
		minState = _resistData._minStun;
		maxState = _resistData._maxStun;
		break;
	case EAbnormalityType::None:
	case EAbnormalityType::Acid:
	case EAbnormalityType::bleeding:
	case EAbnormalityType::Max:
	default:
		break;
	}
	//가중치 추가
	curWeight = FMath::Clamp(curWeight + data->_stateWeight, 0.f, maxState );
	
	_remainWeights[state] = curWeight;

		//GetStatComponent()->ReceiveDirectDamage(damage);
	//틱이 꺼져있으면 활성화.
	if (!IsComponentTickEnabled()==false)
	{
		SetComponentTickEnabled(true);
	}
		

	_activeAbnormalitiesWeight |= temp;
	
	//임계치가 최소임계치를 넘어 상태이상이 생김.
	if (curWeight > minState)
	{
		_remainTimes[state] = data->_stateDruration;
		_activeAbnormalities |= temp;

	}
	bool prevSlow = IsSlow();
	bool prevUnable = IsUnable();
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

	UCharacterMovementComponent* MoveComp = _owner->GetCharacterMovement();
	if (MoveComp)
	{

		MoveComp->SetAvoidanceEnabled(false);
	}

	_isActing = true;
	return true;
}

bool UCharacterStateComponent::IsUnable()
{
	uint32 temp=(uint32)EAbnormalityState::Shock | (uint32)EAbnormalityState::StrongStagger;
	return CheckAbnormality(temp);
}

bool UCharacterStateComponent::IsSlow()
{

	uint32 temp = (uint32)EAbnormalityState::AcidBubble | (uint32)EAbnormalityState::AcidStream | (uint32)EAbnormalityState::Gas | (uint32)EAbnormalityState::Thornbush | (uint32)EAbnormalityState::LightStagger ;
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

	while (flags != 0)
	{
		uint32 active = flags & (~flags + 1);
		flags = flags & (flags - 1);          
		CalcActivates(active, deltaTime);
	}

	//_owner->GetStatComponent()->ReceiveDirectDamage(damage);
	if (_activeAbnormalities == 0)
	{
		PrimaryComponentTick.bCanEverTick = false; // (옵션, 명시적 설정)
		SetComponentTickEnabled(false);
	}
}

void UCharacterStateComponent::CalcAbnormalityWeight(float deltaTime)
{

	uint32 flags = _activeAbnormalitiesWeight;

	while (flags != 0)
	{
		uint32 active = flags & (~flags + 1);
		flags = flags & (flags - 1);

		CalcActivatesWeight(active, deltaTime);
	}

}

void UCharacterStateComponent::CalcActivatesWeight(uint32 type, float deltaTime)
{
	EAbnormalityState temp = (EAbnormalityState)type;
	float* foundWeight = _remainWeights.Find(temp);
	if (foundWeight == nullptr)
		return ;
	float prev = *foundWeight;
	float cur = FMath::Max(prev - deltaTime, 0.f);
	*foundWeight = cur;
	if (cur <= 0.f)
	{
		//시간이 종료되면 목록에서 제거

		_activeAbnormalitiesWeight &= ~type;
	}
}

int UCharacterStateComponent::CalcActivates(uint32 type,float deltaTime)
{
	EAbnormalityState temp = (EAbnormalityState)type;
	float* foundRemain = _remainTimes.Find(temp);
	if (foundRemain == nullptr)
		return 0;
	float prev = *foundRemain;
	float cur = FMath::Max(prev - deltaTime, 0.f);
	*foundRemain = cur;
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


	int32 abnormalityIndex = FMath::FloorLog2(type);
	CalcDamage((EAbnormality)abnormalityIndex);
	return 1;
}

void UCharacterStateComponent::CalcDamage(EAbnormality state)
{
	UCGameInstance* gm = GetWorld()->GetGameInstance<UCGameInstance>();
	const FCDamageEvent* de = gm->GetAbnormalDamageEventData(state);
	if (de == nullptr)
		return;

	float actualDamage = _owner->TakeDamage(
		0.f,
		*de, // FCDamageEvent를 FDamageEvent const&로 전달
		nullptr,
		nullptr
	);
}

void UCharacterStateComponent::Reset()
{
	for (auto& temp : _remainTimes)
	{
		temp.Value = 0.f;
	}
	for (auto& temp : _remainWeights)
	{
		temp.Value = 0.f;
	}
	_activeAbnormalities = 0;
	_activeAbnormalitiesWeight = 0;
}

void UCharacterStateComponent::CheckStagger( const struct FCDamageEvent* damageEvent )
{
	if (damageEvent->Stagger >= _resistData._strongStaggerWeight)
	{
		_owner->StrongStagger(_resistData._strongStaggerTime);
		_owner->KnockBack(damageEvent->PushForce * damageEvent->ShotDirection * -1.f);

	}
	if (damageEvent->Stagger >= _resistData._lowStaggerWeight)
	{
		_owner->WeakStagger(_resistData._lowStaggerTime);
	}
}

