// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../GunDataTable.h"
#include "GunDamageComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta = (BlueprintSpawnableComponent))
class SGAPROJECTMAIN_API UGunDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunDamageComponent();

	virtual void SetDamageData(const FGunData& gunData);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void FireShot(FVector muzzleLocation, FRotator muzzleRotation, bool isAiming);
	virtual void DoFireShot(FVector fireLocation, FVector fireDirection) {};

	virtual float CalculateDamage(float distance);

protected:
	UPROPERTY()
	class AGunBase* _gun;

	float _baseDamage = 80.0f; // 기본 데미지
	float _vsDurableDamage = 15.0f; // 내구 데미지

	EPenetrateTrait _penetrateTrait = EPenetrateTrait::Light; // 관통력

	float _falloff25 = 0.04f;
	float _falloff50 = 0.072f;
	float _falloff100 = 0.133f;

	float _shakeAmount = 0.0f;
};
