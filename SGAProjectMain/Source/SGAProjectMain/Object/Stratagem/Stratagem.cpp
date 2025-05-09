// Fill out your copyright notice in the Description page of Project Settings.


#include "Stratagem.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AStratagem::DeployStratagem()
{
	if (_objectToSpawn)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = this;

		FVector spawnLocation = GetActorLocation(); // �Ǵ� _targetActor->GetActorLocation()
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
		// 1. �������� ȸ�� ���� (�ϴ� ������ ���ϰ�)
		FVector currentForward = GetActorForwardVector();
		FRotator uprightRotation = FRotationMatrix::MakeFromXZ(currentForward, FVector::UpVector).Rotator();
		SetActorRotation(uprightRotation);

		// 2. ���� ó�� (ȸ���� ������ ���� ���������Ƿ� ������)
		AttachToComponent(Hit.Component.Get(), FAttachmentTransformRules::KeepWorldTransform);

		// 3. �̵� ���� �� Ÿ�̸�
		_projectileMovement->StopMovementImmediately();
		GetWorldTimerManager().SetTimer(_deployTimerHandle, this, &AStratagem::DeployStratagem, _deployDelay, false);
	}
}

bool AStratagem::IsSurfaceAttachable(const FHitResult& Hit)
{
	// ���� ���� (0,0,1)�� �󸶳� �����Ѱ�
	float dot = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
	return dot > 0.5f; // 0.7 �̻��̸� ���� ���� ����� ���
}
