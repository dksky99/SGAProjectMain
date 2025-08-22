// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BombProjectile.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ABombProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABombProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 충돌 시 폭발 트리거
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);


protected:
	// 충돌 루트(간단한 스피어 콜리전)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Bomb")
	class UCapsuleComponent* _collisionComponent;

	// 렌더용 메시(콜리전 비활성)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Bomb")
	class UStaticMeshComponent* _mesh;

	// 낙하 이동(컨트롤러가 Velocity 세팅)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Bomb")
	class UProjectileMovementComponent* _projectileMovement;

	// 폭발 처리 전용 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Bomb")
	class UExplosionComponent* _explosionComponent;

	// 메시에만 적용할 롤 스핀 속도(도/초). 0.0f이면 회전 없음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Bomb")
	float _spinDegreesPerSecond = 90.0f;
};
