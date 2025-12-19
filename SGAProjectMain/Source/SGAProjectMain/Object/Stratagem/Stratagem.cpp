// Fill out your copyright notice in the Description page of Project Settings.

#include "Stratagem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../../Character/HellDiver/HellDiver.h"
#include "Offensive/BombardmentController.h"
#include "Offensive/EagleStrikeController.h"
#include "../../UI/StratagemEtaWidget.h"

AStratagem::AStratagem()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStratagem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1) 아직 ETA가 준비 안 됐으면, 몽타주의 현재 섹션이 Loop인지 확인
	if (!_isEtaReady)
	{
		if (_animMontage && _mesh)
		{
			if (UAnimInstance* animInst = _mesh->GetAnimInstance())
			{
				if (animInst->Montage_IsPlaying(_animMontage))
				{
					const FName currentSection = animInst->Montage_GetCurrentSection(_animMontage);
					if (currentSection == TEXT("Loop"))
					{
						_isEtaReady = true;
					}
				}
			}
		}
	}

	if (_impactRemain > 0.0f)
	{
		_impactRemain -= DeltaTime;

		if (_impactRemain <= 0.0f)
		{
			// 공격형: 첫 폭격까지 끝났고, 폭격 구간 시간이 남아 있다면 한 번만 교체
			if (_type == EStratagemType::Offensive && _bombardRemain > 0.0f)
			{
				_impactRemain = _bombardRemain;
				_bombardRemain = 0.0f;
			}
		}

		// 남은 시간이 완전히 0이 되면 제거
		if (_impactRemain <= 0.0f)
		{
			DestroySelf();
		}
	}
}

void AStratagem::DeployStratagem()
{
	if (!_objectToSpawn)
	{
		DestroySelf();
		return;
	}

	FActorSpawnParameters sp;
	sp.Owner = this;
	sp.Instigator = Cast<APawn>(_owner);

	const FVector targetLocation = GetActorLocation();

	if (_type == EStratagemType::Offensive)
	{
		// 공격형: 폭격 컨트롤러 처리 전용 헬퍼
		SetupOffensiveStratagem(targetLocation, sp);
	}
	else
	{
		// 비공격형: 드랍포드 실제 스폰 + 낙하 시작만 담당
		SpawnNonAttackDropPod(targetLocation, sp);
	}
}

void AStratagem::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	if (IsSurfaceAttachable(Hit))
	{
		// 부착 처리 (회전은 위에서 직접 세팅했으므로 유지됨)
		AttachToComponent(Hit.Component.Get(), FAttachmentTransformRules::KeepWorldTransform);

		// 이동 멈춤
		_projectileMovement->StopMovementImmediately();

		// 스트라타젬 방향이 위를 향하게 설정
		SetActorRotation(FRotationMatrix::MakeFromX(FVector::ZeroVector).Rotator());

		// 애니메이션 재생
		if (_animMontage)
		{
			if (UAnimInstance* animInst = _mesh->GetAnimInstance())
			{
				animInst->Montage_Play(_animMontage);
			}
		}

		const FVector targetLocation = GetActorLocation();

		if (_type == EStratagemType::Offensive)
		{
			// 공격형: 바로 배치(컨트롤러 스폰 + 타이밍 계산)
			DeployStratagem();
		}
		else
		{
			// 비공격형 전체 수명 계산 (부착 → 투하 딜레이 → 낙하)
			const float totalLifetime = ComputeNonAttackTotalLifetime(targetLocation);

			// ETA 표시용
			_impactRemain = totalLifetime;
			
			// _deployDelay 후에 드랍포드를 실제로 소환
			GetWorldTimerManager().SetTimer(
				_deployTimerHandle,
				this,
				&AStratagem::DeployStratagem,
				_deployDelay,
				false
			);
		}
	}
}

void AStratagem::ShowEtaAtScreenPosition(const FVector2D& screenPosition, int32 etaSec, APlayerController* ownerPC)
{
	if (_impactRemain <= 0.0f)
	{
		return;
	}

	if (!ownerPC)
	{
		return;
	}

	if (!_etaWidget)
	{
		if (!_etaWidgetClass)
		{
			// 클래스가 세팅되지 않았다면 아무것도 하지 않음
			return;
		}

		_etaWidget = CreateWidget<UStratagemEtaWidget>(ownerPC, _etaWidgetClass);
		if (_etaWidget)
		{
			_etaWidget->AddToViewport(150);
		}
	}

	if (_etaWidget)
	{
		_etaWidget->SetEtaSeconds(etaSec);
		_etaWidget->SetWidgetScreenPosition(screenPosition);
		_etaWidget->SetIndicatorVisible(true);
	}
}

void AStratagem::HideEta()
{
	if (_etaWidget)
	{
		_etaWidget->SetIndicatorVisible(false);
	}
}

bool AStratagem::IsSurfaceAttachable(const FHitResult& Hit)
{
	// 수직 위쪽 (0,0,1)과 얼마나 유사한가
	const float dot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
	return dot > 0.5f; // 0.7 이상이면 거의 땅에 가까운 경사
}

float AStratagem::ComputeNonAttackTotalLifetime(const FVector& targetLocation) const
{
	// 드랍포드 스폰 위치: 타깃 위치 위로 _dropHeight 만큼
	const FVector spawnLocation = targetLocation + FVector(0.0f, 0.0f, _dropHeight);

	const float speed = FMath::Max(_dropSpeed, 1.0f);
	const float distance = FVector::Dist(spawnLocation, targetLocation);
	const float travelTime = distance / speed;

	// 부착 시점 기준: _deployDelay 대기 + travelTime 낙하
	return _deployDelay + travelTime;
}

void AStratagem::SpawnNonAttackDropPod(const FVector& targetLocation, const FActorSpawnParameters& sp)
{
	// 비공격형: 신호기 바로 위에서 떨어뜨림
	const FVector spawnLocation = targetLocation + FVector(0.0f, 0.0f, _dropHeight);

	const FVector direction = (targetLocation - spawnLocation).GetSafeNormal();
	const FRotator spawnRotation = FRotator(0.0f, direction.Rotation().Yaw, 0.0f);

	const float speed = FMath::Max(_dropSpeed, 1.0f);

	if (AActor* spawned = GetWorld()->SpawnActor<AActor>(_objectToSpawn, spawnLocation, spawnRotation, sp))
	{
		if (UProjectileMovementComponent* projectile = spawned->FindComponentByClass<UProjectileMovementComponent>())
		{
			projectile->Velocity = direction * speed;
		}
	}
}

void AStratagem::SetupOffensiveStratagem(const FVector& targetLocation, const FActorSpawnParameters& sp)
{
	const FVector spawnLocation = targetLocation;
	const FRotator spawnRotation = FRotator::ZeroRotator;

	// 일단 액터로 스폰
	AActor* spawned = GetWorld()->SpawnActor<AActor>(_objectToSpawn, spawnLocation, spawnRotation, sp);
	if (!spawned)
	{
		return;
	}

	// 먼저 궤도 폭격 컨트롤러인지 확인
	if (ABombardmentController* bc = Cast<ABombardmentController>(spawned))
	{
		// _deployDelay 후 폭격 시작
		bc->InitializeBombardment(_deployDelay);

		const float firstImpactDelay = bc->GetEstimatedFirstImpactDelay();
		const float bombardDuration = bc->GetEstimatedBombardDuration();

		// 1단계: 첫 폭격까지
		_impactRemain = firstImpactDelay;

		// 2단계: 폭격 구간
		_bombardRemain = bombardDuration;

		return;
	}

	// 궤도 폭격이 아니라면, 이글 컨트롤러인지 확인
	if (AEagleStrikeController* ec = Cast<AEagleStrikeController>(spawned))
	{
		// 이글 공격 초기화
		ec->InitializeStrike(_deployDelay);

		// 첫 폭탄 ETA
		const float firstImpactDelay = ec->ComputeEtaToFirstImpact();

		_impactRemain = firstImpactDelay;
		_bombardRemain = 0.0f; // 이글은 "지속 폭격 시간" 개념이 별도로 없으면 0.0f

		return;
	}
}

void AStratagem::DestroySelf()
{
	HideEta();
	Super::DestroySelf();
}
