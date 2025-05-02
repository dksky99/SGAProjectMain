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
	float _totalFuseTime = 5.0f; // ������ ���� �ɸ��� �ð�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _cookedTime = 0.0f; // ��ŷ �ð�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionRadius = 300.0f; // ���� �ݰ�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade")
	float _explosionDamage = 100.0f; // ���� ������

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade")
	UStaticMeshComponent* _grenadeMesh; // ����ź �޽�

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	UParticleSystem* _explosionEffect; // ���� ����Ʈ

	class APlayerCharacter* _owner;

	FTimerHandle _explosionTimerHandle;
};
