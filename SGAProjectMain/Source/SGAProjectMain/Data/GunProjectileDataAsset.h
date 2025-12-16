// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GunProjectileDataAsset.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EGunProjectileType : uint8
{
	Standard,
	Explosive
};

USTRUCT(BlueprintType)
struct FArmorPenetration
{
	GENERATED_BODY()

	// 입사각에 따른 관통력 단계
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _direct = 2; // 25도 이내
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _slightAngle = 2; // 60도 이내
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _largeAngle = 2; // 80도 이내
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _extremeAngle = 0; // 80도 이상
};

USTRUCT(BlueprintType)
struct FGunProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGunProjectileType _type = EGunProjectileType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f; // 기본 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _vsDurableDamage = 15.0f; // 내구 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FArmorPenetration _armorPenetration; // 장갑 관통력

	// 초기 속력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _initialSpeed = 900.f; // m/s 단위. 실제 사용 시 *100 필요

	// 거리에 따른 감속량 -> 데미지에 비례
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;
	// 관통에 따른 감속량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloffPenetration = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _demolitionDamage = 10.f; // 철거 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _stagger = 15.f; // 비틀거림 유발
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _pushForce = 10.f; // 밀치기 위력
};

UCLASS()
class SGAPROJECTMAIN_API UGunProjectileDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGunProjectileData _projectileData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AGunBulletBase> _projectileClass;
};
