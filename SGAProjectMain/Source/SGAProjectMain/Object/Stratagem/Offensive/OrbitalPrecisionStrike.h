// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrbitalPrecisionStrike.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AOrbitalPrecisionStrike : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrbitalPrecisionStrike();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


protected:
	UPROPERTY(VisibleAnywhere, Category = "Game/Offensive")
	UStaticMeshComponent* _mesh;

	UPROPERTY(VisibleAnywhere, Category = "Game/Offensive")
	class UProjectileMovementComponent* _projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Offensive")
	class UParticleSystem* _explosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Offensive")
	float _damage = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Offensive")
	float _radius = 300.f;
};
