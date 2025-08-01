// Fill out your copyright notice in the Description page of Project Settings.


#include "TimedGrenadeBase.h"
#include "SGAProjectMain/SGAProjectMain.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DamageEvents.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

void ATimedGrenadeBase::StartCookingGrenade()
{
	_cookedTime = GetWorld()->GetTimeSeconds();
}

void ATimedGrenadeBase::UpdateCookingGrenade()
{
	if (IsFuseTimeRemaining() || _isExploded)
		return;

	ExplodeGrenade();
}

void ATimedGrenadeBase::Throw(FVector direction)
{
	float remainingFuseTime = GetRemainingFuseTime();
	if (remainingFuseTime < 0)
		ExplodeGrenade();
	else
		GetWorldTimerManager().SetTimer(_explosionTimerHandle, this, &ATimedGrenadeBase::ExplodeGrenade, remainingFuseTime, false);

	Super::Throw(direction);
}

void ATimedGrenadeBase::ExplodeGrenade()
{
	if (IsFuseTimeRemaining() || _isExploded)
		return;

	_isExploded = true;

	GetWorldTimerManager().ClearTimer(_explosionTimerHandle);  // 타이머 제거

	TArray<FOverlapResult> overlaps;
	FCollisionQueryParams params(NAME_None, false, this);

	bool bResult = GetWorld()->OverlapMultiByChannel(
		overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_GameDamage,
		FCollisionShape::MakeSphere(_explosionRadius),
		params
	);

	if (bResult)
	{
		for (const FOverlapResult& overlap : overlaps)
		{
			AActor* hitActor = overlap.GetActor();
			if (hitActor)
			{
				// 폭발 중심에서 대상까지 방향 계산
				const FVector actorLocation = hitActor->GetActorLocation();
				const FVector shotDirection = (actorLocation - GetActorLocation()).GetSafeNormal();

				// 포인트 데미지용 FHitResult 설정
				FHitResult pointHit;
				// TraceStart: 데미지 계산 및 VFX/사운드를 위한 시작 위치 (폭발 중심)
				pointHit.TraceStart = GetActorLocation();
				// ImpactPoint: 데미지가 적용될 위치 (피격 대상 위치)
				pointHit.ImpactPoint = actorLocation;
				// Component: 어느 콜리전 컴포넌트에 명중했는지 정보
				pointHit.Component = overlap.GetComponent();
				// bBlockingHit: 유효한 히트 이벤트임을 표시
				pointHit.bBlockingHit = true;

				// 폭발 중심과의 거리 계산
				float distance = FVector::Distance(GetActorLocation(), actorLocation);

				// 거리 비율로 감쇠값 구하기
				float distanceRatio = FMath::Clamp(1.0f - (distance / _explosionRadius), 0.0f, 1.0f);

				// 실제 데미지
				float actualDamage = _explosionDamage * distanceRatio;

				// ApplyPointDamage 호출로 부위별 데미지 처리 정보 전달
				UGameplayStatics::ApplyPointDamage(
					hitActor,                       // 데미지를 받을 액터
					actualDamage,					// 적용할 최종 데미지 값
					shotDirection,                  // 데미지가 들어온 방향 벡터
					pointHit,                       // 충돌 정보(FHitResult)
					GetInstigatorController(),      // 데미지를 유발한 컨트롤러
					this,                           // 데미지 발생 주체 액터
					UDamageType::StaticClass()      // 사용할 데미지 타입 클래스
				);
			}
		}
	}

	// 이펙트 재생
	if (_explosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _explosionEffect, GetActorLocation());
	}

	DestroySelf();
}

bool ATimedGrenadeBase::IsFuseTimeRemaining()
{
	return(GetWorld()->GetTimeSeconds() - _cookedTime) < _totalFuseTime;
}

float ATimedGrenadeBase::GetRemainingFuseTime()
{
	return _totalFuseTime - (GetWorld()->GetTimeSeconds() - _cookedTime);
}
