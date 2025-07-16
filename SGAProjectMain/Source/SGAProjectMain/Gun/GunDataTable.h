// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GunDataTable.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGunType : uint8
{
	OneHanded,
	TwoHanded
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	FireAuto,
	FireSemi,
	FireBurst,
	FireBoltAction
};

UENUM(BlueprintType)
enum class EReloadStage : uint8
{
	None,
	RemoveMag,
	InsertMag,
	CloseBolt,
	RoundsReload // 한 발씩 장전
};

UENUM(BlueprintType)
enum class ETacticalLightMode : uint8
{
	LightAuto,
	LightOn,
	LightOff
};

UENUM(BlueprintType)
enum class EPenetrateTrait : uint8
{
	Light,
	Medium,
	Heavy,
	AntiTank
};

USTRUCT(BlueprintType)
struct FGunData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName _name = "gun";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGunType _type; // 한손 혹은 두손

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* _icon;

	// 데미지 요소
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f; // 기본 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _vsDurableDamage = 15.0f; // 내구 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPenetrateTrait _penetrateTrait = EPenetrateTrait::Light; // 관통력

	// 발사 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _fireInterval = 60.0f / 640.0f;

	// 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxAmmo = 45;

	// 탄창
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _initialMag = 6;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxMag = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _refillMagAmount = 4;

	// 인체공학성
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//float _ergo = 54;

	// 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _recoil = 14.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _verticalRecoil = 5.f;        // 수직 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _horizontalRecoil = 6.f;      // 수평 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _shakeAmount = 4.f;           // 탄퍼짐

	// 거리에 따른 데미지 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff25 = 0.04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff50 = 0.072f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _falloff100 = 0.133f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EFireMode> _fireModes = { EFireMode::FireAuto };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ETacticalLightMode> _lightModes = { ETacticalLightMode::LightOff };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> _scopeModes = {};

	// 클래스나 블루프린트 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AGunBase> _gunClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AImpactMarker> _impactMarkerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> _crosshairClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* _laserFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* _laserImpactFX;
};

UCLASS()
class SGAPROJECTMAIN_API UGunDataTable : public UObject
{
	GENERATED_BODY()
	
};
