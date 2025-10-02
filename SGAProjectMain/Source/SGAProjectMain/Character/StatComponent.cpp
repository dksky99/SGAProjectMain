// Fill out your copyright notice in the Description page of Project Settings.


#include "StatComponent.h"
#include "CharacterStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterBase.h"

// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	_coreMaxHP = 100.0f;
	_coreHP = _coreMaxHP;
	_headMaxHP = 100.0f;
	_headHP = _headMaxHP;
	_torsoMaxHP = 100.0f;
	_torsoHP = _torsoMaxHP;
	_leftArmMaxHP = 20.0f;
	_leftArmHP = _leftArmMaxHP;
	_rightArmMaxHP = 20.0f;
	_rightArmHP = _rightArmMaxHP;
	_leftLegMaxHP = 20.0f;
	_leftLegHP = _leftLegMaxHP;
	_rightLegMaxHP = 20.0f;
	_rightLegHP = _rightLegMaxHP;
}


// Called when the game starts
void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	_owner = Cast<ACharacterBase>(GetOwner());

	if (_owner)
	{
		// 포인트 데미지 이벤트 바인딩
		_owner->OnTakePointDamage.AddDynamic(this, &UStatComponent::HandlePointDamage);
	}
	
}


void UStatComponent::Reset()
{
	_coreHP = _coreMaxHP;
}

bool UStatComponent::IsDead()
{
	if (_coreHP <= 0)
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

void UStatComponent::ChangeHp(float Amount)
{
	_coreHP = FMath::Clamp(_coreHP + Amount, 0.f, _coreMaxHP);
	if (_coreHP <= 0.f)
	{
		OnDeath.Broadcast();
	}
}

void UStatComponent::StartRegen()
{
	// 머리 복구
	_headHP = _headMaxHP;
	OnPartRestored.Broadcast(EBodyPart::Head);

	// 몸통 복구
	_torsoHP = _torsoMaxHP;
	OnPartRestored.Broadcast(EBodyPart::Torso);

	// 왼팔 복구
	_leftArmHP = _leftArmMaxHP;
	OnPartRestored.Broadcast(EBodyPart::LeftArm);

	// 오른팔 복구
	_rightArmHP = _rightArmMaxHP;
	OnPartRestored.Broadcast(EBodyPart::RightArm);

	// 왼다리 복구
	_leftLegHP = _leftLegMaxHP;
	OnPartRestored.Broadcast(EBodyPart::LeftLeg);

	// 오른다리 복구
	_rightLegHP = _rightLegMaxHP;
	OnPartRestored.Broadcast(EBodyPart::RightLeg);
}

void UStatComponent::ReceiveDirectDamage(float Damage)
{
	ProcessDamage(EBodyPart::Core, Damage);
}

void UStatComponent::ProcessDamage(EBodyPart Part, float Damage)
{
	// 부위별 현 체력 포인터 및 최대 체력 참조
	float* CurrentHP = nullptr;
	float MaxHP = 0.0f;

	// 각 부위별 데미지 브로드캐스트
	switch (Part)
	{
	case EBodyPart::Head:
		CurrentHP = &_headHP;
		MaxHP = _headMaxHP;
		if (_headHpChanged.IsBound())
			_headHpChanged.Broadcast(_headHP / _headMaxHP);
		break;
	case EBodyPart::Torso:
		CurrentHP = &_torsoHP;
		MaxHP = _torsoMaxHP;
		if (_torsoHpChanged.IsBound())
			_torsoHpChanged.Broadcast(_torsoHP / _torsoMaxHP);
		break;
	case EBodyPart::LeftArm:
		CurrentHP = &_leftArmHP;
		MaxHP = _leftArmMaxHP;
		if (_leftArmHpChanged.IsBound())
			_leftArmHpChanged.Broadcast(_leftArmHP / _leftArmMaxHP);
		break;
	case EBodyPart::RightArm:
		CurrentHP = &_rightArmHP;
		MaxHP = _rightArmMaxHP;
		if (_rightArmHpChanged.IsBound())
			_rightArmHpChanged.Broadcast(_rightArmHP / _rightArmMaxHP);
		break;
	case EBodyPart::LeftLeg:
		CurrentHP = &_leftLegHP;
		MaxHP = _leftLegMaxHP;
		if (_leftLegHpChanged.IsBound())
			_leftLegHpChanged.Broadcast(_leftLegHP / _leftLegMaxHP);
		break;
	case EBodyPart::RightLeg:
		CurrentHP = &_rightLegHP;
		MaxHP = _rightLegMaxHP;
		if (_rightLegHpChanged.IsBound())
			_rightLegHpChanged.Broadcast(_rightLegHP / _rightLegMaxHP);
		break;
	default:
		// 코어 직접 처리
		_coreHP = FMath::Max(0.0f, _coreHP - Damage);

		_coreHpChanged.Broadcast(_coreHP / _coreMaxHP);

		if (_coreHP == 0.0f)
			OnDeath.Broadcast();
		return;
	}

	// 부위가 흡수할 수 있는 데미지
	float DamageToPart = FMath::Min(Damage, *CurrentHP);
	// 파트가 흡수한 만큼만 코어도 흡수 (초과분은 버림)
	float DamageToCore = DamageToPart;

	// 체력 차감
	*CurrentHP = FMath::Max(0.f, *CurrentHP - DamageToPart);
	_coreHP = FMath::Max(0.0f, _coreHP - DamageToCore);

	_coreHpChanged.Broadcast(_coreHP / _coreMaxHP);

	// 코어 사망 우선 체크
	if (_coreHP == 0.0f)
	{
		OnDeath.Broadcast();
		return;
	}

	// 부위 파괴 이벤트
	if (*CurrentHP == 0.0f)
	{
		OnPartDestroyed.Broadcast(Part);
	}
}

