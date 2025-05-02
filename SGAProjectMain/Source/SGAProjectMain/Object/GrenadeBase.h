// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GrenadeBase.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AGrenadeBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenadeBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void StartCookingGrenade();
	virtual void UpdateCookingGrenade();
	virtual void ThrowGrenade();
	virtual void ExplodeGrenade();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _totalFuseTime = 5.0f; // 터지기 까지 걸리는 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _cookedTime = 0.0f; // 쿠킹 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionRadius = 300.0f; // 폭발 반경

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionDamage = 100.0f; // 폭발 데미지

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	UStaticMeshComponent* _grenadeMesh; // 수류탄 메시

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	UParticleSystem* _explosionEffect; // 폭발 이펙트

	class APlayerCharacter* _owner;

	FTimerHandle _explosionTimerHandle;
};
