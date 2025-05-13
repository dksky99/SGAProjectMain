// Fill out your copyright notice in the Description page of Project Settings.


#include "Stratagem.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AStratagem::DeployStratagem()
{
	if (_objectToSpawn)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;
		spawnParams.Instigator = Cast<APawn>(_owner); // 플레이어를 Instigator로 설정

		FVector targetLocation = GetActorLocation();

		// 스폰 위치 결정
		FVector dropOrigin;
		if (_isAttackStratagem)
		{
			dropOrigin = FVector(0, 0, 0); // 맵 중앙 기준
		}
		else
		{
			dropOrigin = targetLocation; // 신호기 바로 위
		}

		// 낙하 시작 위치 = 기준점 + 위로 높이
		FVector spawnLocation = dropOrigin + FVector(0, 0, 1500.f);

		// 방향 = 타겟 - 스폰 위치
		FVector direction = (targetLocation - spawnLocation).GetSafeNormal();
		FRotator spawnRotation = direction.Rotation();

		// 스폰
		AActor* spawned = GetWorld()->SpawnActor<AActor>(
			_objectToSpawn,
			spawnLocation,
			spawnRotation,
			spawnParams
		);

		// 속도 설정
		if (spawned)
		{
			if (UProjectileMovementComponent* projectile = spawned->FindComponentByClass<UProjectileMovementComponent>())
			{
				projectile->Velocity = direction * 8000.f;
			}
		}
	}

	DestroySelf();
}

void AStratagem::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	if (IsSurfaceAttachable(Hit))
	{
		// 1. 수직으로 회전 보정 (하늘 방향을 향하게)
		FVector currentForward = GetActorForwardVector();
		FRotator uprightRotation = FRotationMatrix::MakeFromXZ(currentForward, FVector::UpVector).Rotator();
		SetActorRotation(uprightRotation);

		// 2. 부착 처리 (회전은 위에서 직접 세팅했으므로 유지됨)
		AttachToComponent(Hit.Component.Get(), FAttachmentTransformRules::KeepWorldTransform);

		// 3. 이동 멈춤 및 타이머
		_projectileMovement->StopMovementImmediately();
		GetWorldTimerManager().SetTimer(_deployTimerHandle, this, &AStratagem::DeployStratagem, _deployDelay, false);
	}
}

bool AStratagem::IsSurfaceAttachable(const FHitResult& Hit)
{
	// 수직 위쪽 (0,0,1)과 얼마나 유사한가
	float dot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
	return dot > 0.5f; // 0.7 이상이면 거의 땅에 가까운 경사
}
