// Fill out your copyright notice in the Description page of Project Settings.


#include "Stratagem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../../Character/HellDiver/HellDiver.h"

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

	if (_isAttackStratagem)
	{
		// 공격형: 컨트롤러를 신호기 위치에 바로 스폰
		const FVector spawnLocation = targetLocation;
		const FRotator spawnRotation = FRotator::ZeroRotator;

		GetWorld()->SpawnActor<AActor>(_objectToSpawn, spawnLocation, spawnRotation, sp);
		// 낙하/속도/웨이브/산포는 컨트롤러 내부에서 처리
	}
	else
	{
		// 비공격형:  신호기 바로 위에서 떨어뜨림
		const FVector dropOrigin = targetLocation;
		const FVector spawnLocation = dropOrigin + FVector(0.0f, 0.0f, 1500.0f);

		const FVector direction = (targetLocation - spawnLocation).GetSafeNormal();
		const FRotator spawnRotation = FRotator(0.0f, direction.Rotation().Yaw, 0.0f);

		if (AActor* spawned = GetWorld()->SpawnActor<AActor>(_objectToSpawn, spawnLocation, spawnRotation, sp))
		{
			if (UProjectileMovementComponent* projectile = spawned->FindComponentByClass<UProjectileMovementComponent>())
			{
				projectile->Velocity = direction * _dropSpeed;
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
		// 부착 처리 (회전은 위에서 직접 세팅했으므로 유지됨)
		AttachToComponent(Hit.Component.Get(), FAttachmentTransformRules::KeepWorldTransform);

		// 이동 멈춤
		_projectileMovement->StopMovementImmediately();

		// 스트라타젬 방향이 위를 향하게 설정
		SetActorRotation(FRotationMatrix::MakeFromX(FVector::ZeroVector).Rotator());

		// 애니메이션 재생
		if (_animMontage)
		{
			if (UAnimInstance* animInst = _mesh->GetAnimInstance() )
			{
				animInst->Montage_Play(_animMontage);
			}
		}

		// 타이머 등록
		GetWorldTimerManager().SetTimer(_deployTimerHandle, this, &AStratagem::DeployStratagem, _deployDelay, false);
	}
}

bool AStratagem::IsSurfaceAttachable(const FHitResult& Hit)
{
	// 수직 위쪽 (0,0,1)과 얼마나 유사한가
	float dot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
	return dot > 0.5f; // 0.7 이상이면 거의 땅에 가까운 경사
}

