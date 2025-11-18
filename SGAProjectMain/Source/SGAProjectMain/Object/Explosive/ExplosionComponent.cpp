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
	for (const FOverlapResult& OverlapResult : Overlaps) 
	{
		if (AActor* HitActor = OverlapResult.GetActor()) 
		{
			if (UPrimitiveComponent* HitComp = OverlapResult.GetComponent())
			{
				// 콜리더 태그로 부위 판별 및 HP 조회 (태그 없으면 Core 로 간주)
				UStatComponent* StatComp = HitActor->FindComponentByClass<UStatComponent>();
				
				
				float PartHP = StatComp->GetCoreStat()->_curHP; // 태그가 없다면 코어
				
				//태그를 확인하고 그에 맞는 데이터로 반환하는 기능을 추가해서 바꿔놨습니다. 맞는 태그를 찾으면 그에맞는 파트 데이터를 반환받을 수 있습니다.
				for (auto tag : HitComp->ComponentTags)
				{
					auto data = StatComp->GetPartStat(tag);
					if (data == nullptr)
						continue;
					PartHP = data->_curHP;
					break;
				}

				// HP가 0 이하인 부위는 건너뜀
				if (PartHP <= 0.0f)                          
					continue;                                

				// 거리 기반 감쇠 및 실제 데미지 계산
				const FVector Center = GetOwner()->GetActorLocation();					// 폭발 중심        
				const FVector ColliderLoc = HitComp->GetComponentLocation();			// 맞은 컴포넌트
				float Distance = FVector::Dist(Center, ColliderLoc);					// 폭발과의 거리
				float Ratio = FMath::Clamp(1.0f - (Distance / _radius), 0.0f, 1.0f);	// 거리 감쇠 비율
				float ActualDmg = _damage * Ratio;										// 최종 적용할 데미지

				// 데미지 히트 정보 세팅 및 적용
				FHitResult HitInfo;                                              
				HitInfo.TraceStart = Center;                                     
				HitInfo.ImpactPoint = ColliderLoc;                               
				HitInfo.Component = HitComp;                                     
				HitInfo.bBlockingHit = true;                                     

				UGameplayStatics::ApplyPointDamage(                              
					HitActor,                                                    
					ActualDmg,                                                   
					(ColliderLoc - Center).GetSafeNormal(),                      
					HitInfo,                                                     
					GetOwner()->GetInstigatorController(),                       
					GetOwner(),                                                  
					UDamageType::StaticClass()                                   
				);                                                               
			}
		}
	}
}