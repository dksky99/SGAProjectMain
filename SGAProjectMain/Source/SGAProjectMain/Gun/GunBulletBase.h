// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBulletBase.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AGunBulletBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGunBulletBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	void InitializeProjectile();

private:
    UPROPERTY(VisibleAnywhere)
    class USphereComponent* _collisionComp;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* _projectileMovement;

    UPROPERTY(EditAnywhere, Category = "Damage")
    float Damage = 50.f;

};
