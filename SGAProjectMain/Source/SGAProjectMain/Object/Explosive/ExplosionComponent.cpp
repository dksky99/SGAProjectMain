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
	// 액터별로 모든 Overlap 결과를 모아둠
	TMap<AActor*, TArray<FOverlapResult>> OverlapsPerActor;
	for (const FOverlapResult& Ov : Overlaps)
	{
		if (AActor* Actor = Ov.GetActor())
			OverlapsPerActor.FindOrAdd(Actor).Add(Ov);
	}

	for (auto & Pair : OverlapsPerActor)
	{
		AActor* Actor = Pair.Key;
		auto& OvArray = Pair.Value;

		// 폭발 중심에서 가장 가까운 거리 순으로 정렬
		OvArray.Sort([this](const FOverlapResult& A, const FOverlapResult& B) {
			return FVector::Dist(GetOwner()->GetActorLocation(), A.GetActor()->GetActorLocation())
				< FVector::Dist(GetOwner()->GetActorLocation(), B.GetActor()->GetActorLocation());
			});

		// 첫 번째 유효 콜리더 찾기 (체력 0인 부위 스킵)
		FOverlapResult* Selected = nullptr;

		if (UStatComponent* Stat = Actor->FindComponentByClass<UStatComponent>())
		{
			for (FOverlapResult& Ov : OvArray)
			{
				if (UPrimitiveComponent* Comp = Ov.GetComponent())
				{
					// 콜리더 태그로 부위 판별
					float PartHP = Stat->GetCoreHP(); // 태그가 없다면 코어
					if (Comp->ComponentHasTag("Head"))			  PartHP = Stat->GetHeadHP();
					else if (Comp->ComponentHasTag("Torso"))	  PartHP = Stat->GetTorsoHP();
					else if (Comp->ComponentHasTag("LeftArm"))    PartHP = Stat->GetLeftArmHP();
					else if (Comp->ComponentHasTag("RightArm"))   PartHP = Stat->GetRightArmHP();
					else if (Comp->ComponentHasTag("LeftLeg"))    PartHP = Stat->GetLeftLegHP();
					else if (Comp->ComponentHasTag("RightLeg"))   PartHP = Stat->GetRightLegHP();

					// HP가 0 이하인 부위는 건너뜀
					if (PartHP <= 0.0f)
						continue;

					Selected = &Ov;
					break;
				}
			}
		}

		if (!Selected)
			continue;

		// 거리 기반 감쇠 및 데미지 적용
		float Distance = FVector::Distance(GetOwner()->GetActorLocation(), Selected->GetComponent()->GetComponentLocation());
		float Ratio = FMath::Clamp(1.0f - (Distance / _radius), 0.0f, 1.0f);
		float ActualDmg = _damage * Ratio;

		FHitResult Hit;
		Hit.TraceStart = GetOwner()->GetActorLocation();
		Hit.ImpactPoint = Selected->GetComponent()->GetComponentLocation();
		Hit.Component = Selected->GetComponent();
		Hit.bBlockingHit = true;

		UGameplayStatics::ApplyPointDamage(
			Actor,
			ActualDmg,
			(Actor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal(),
			Hit,
			GetOwner()->GetInstigatorController(),
			GetOwner(),
			UDamageType::StaticClass()
		);
	}
}

