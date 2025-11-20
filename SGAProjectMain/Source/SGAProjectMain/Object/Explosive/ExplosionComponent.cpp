// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

#include "../../Character/StatComponent.h"
#include "../../SGAProjectMain.h"

// Sets default values for this component's properties
UExplosionComponent::UExplosionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UExplosionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UExplosionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UExplosionComponent::Explode()
{
	HandleExplosion();
}

void UExplosionComponent::HandleExplosion()
{
	// 1) Overlap 검사
	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params(NAME_None, false, GetOwner());
	bool bHit = GetWorld()->OverlapMultiByChannel(
		overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		ECC_GameDamage,
		FCollisionShape::MakeSphere(_radius),
		params
	);

	if (bHit)
	{
		ApplyDamageToOverlaps(overlaps);
	}

	// 2) 이펙트 재생
	if (_effect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			_effect,
			GetOwner()->GetActorLocation()
		);
	}
}

void UExplosionComponent::ApplyDamageToOverlaps(const TArray<FOverlapResult>& Overlaps)
{
	// 폭심지(폭발 중심)는 보통 이 컴포넌트를 가진 액터의 위치로 사용한다.
	const FVector center = GetOwner()->GetActorLocation();

	for (const FOverlapResult& overlap : Overlaps)
	{
		AActor* hitActor = overlap.GetActor();
		UPrimitiveComponent* hitComp = overlap.GetComponent();

		// 액터나 컴포넌트가 유효하지 않으면 스킵
		if (!IsValid(hitActor) || !IsValid(hitComp))
		{
			continue;
		}

		// 히트박스가 아닌 콜리전이라면 스킵 (예: HitBox 태그로 구분한다고 가정)
		if (!hitComp->ComponentTags.Contains("HitBox"))
		{
			continue;
		}
				
		// 폭심지와 콜리전 위치 사이 거리로 피해 비율 계산
		const FVector colliderLoc = hitComp->GetComponentLocation();
		const float distance = FVector::Dist(center, colliderLoc);
		const float ratio = FMath::Clamp(1.0f - (distance / _radius), 0.0f, 1.0f);
		const float actualDmg = _damage * ratio;

		// 실제 피해량이 0 이하라면 의미가 없으므로 스킵
		if (actualDmg <= 0.0f)
		{
			continue;
		}

		// FPointDamageEvent 기반 필드(FHitResult)를 채워준다.
		FHitResult hitInfo;
		hitInfo.TraceStart = center;
		hitInfo.ImpactPoint = colliderLoc;
		hitInfo.Component = hitComp;
		hitInfo.bBlockingHit = true;

		// 이번 콜리전에 대한 커스텀 데미지 이벤트를 새로 생성한다.
		FCDamageEvent damageEvent;
		damageEvent.HitInfo = hitInfo;
		// 폭심지 -> 콜리전 방향(넉백/이펙트 방향 등으로 활용 가능)
		damageEvent.ShotDirection = (colliderLoc - center).GetSafeNormal();

		// 폭발이 주는 실제 피해량을 BaseDamage로 전달
		damageEvent.BaseDamage = static_cast<int32>(actualDmg);
		// 필요하면 여기서 내구 피해/철거 피해를 따로 설정할 수 있다.
		damageEvent.DurabilityDamage = 0;
		damageEvent.DemolitionDamage = 0;
		damageEvent.PenetrationLevel = 0;

		// 폭발 피해 플래그와 이번에 맞은 콜리전 컴포넌트 설정
		damageEvent.IsExplosionDamage = true;
		damageEvent.ColComp = hitComp;

		// 데미지 타입 (나중에 폭발 전용 UCDamageType BP로 교체 가능)
		damageEvent.DamageTypeClass = UCDamageType::StaticClass();

		// 최종적으로 이 액터의 TakeDamage를 호출한다.
		// ACharacterBase라면 FCDamageEvent를 받아서 부위/장갑/폭발저항까지 모두 계산한다.
		hitActor->TakeDamage(
			actualDmg,
			damageEvent,
			GetOwner()->GetInstigatorController(),
			GetOwner()
		);
	}
}