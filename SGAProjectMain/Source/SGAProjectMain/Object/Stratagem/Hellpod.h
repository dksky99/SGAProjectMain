// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hellpod.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellpod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHellpod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void DestroySelf();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Hellpod")
	UStaticMeshComponent* _mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	TSubclassOf<AActor> _hellpodToSpawn;

	UPROPERTY(VisibleAnywhere, Category = "Game/Hellpod")
	class UProjectileMovementComponent* _projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	float _damage = 100.f;

	bool isAlreadySpawned = false;
};
