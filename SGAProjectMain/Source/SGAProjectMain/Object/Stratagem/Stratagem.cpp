// Fill out your copyright notice in the Description page of Project Settings.


#include "Stratagem.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AStratagem::DeployStratagem()
{
	if (_objectToSpawn)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;

		FVector spawnLocation = GetActorLocation(); // 또는 _targetActor->GetActorLocation()
		FRotator spawnRotation = FRotator::ZeroRotator;

		GetWorld()->SpawnActor<AActor>(
			_objectToSpawn,
			spawnLocation,
			spawnRotation,
			spawnParams
		);
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
