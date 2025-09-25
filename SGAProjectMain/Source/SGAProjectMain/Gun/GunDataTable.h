// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GunDataTable.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EGunGripType : uint8
{
	OneHanded,
	TwoHanded
};

UENUM(BlueprintType)
enum class EGunSlotType : uint8
{
	Primary,
	Secondary,
	Support
};

UENUM(BlueprintType)
enum class EGunCategory : uint8
{
	AssaultRifle	UMETA(DisplayName = "Assault Rifle"),
	MarksmanRifle	UMETA(DisplayName = "Marksman Rifle"),
	Shotgun			UMETA(DisplayName = "Shotgun"),
	Pistol			UMETA(DisplayName = "Pistol"),
	Support			UMETA(DisplayName = "Support")
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
enum class EReloadType : uint8
{
	Magazine,
	RoundsReload
};

UENUM(BlueprintType)
enum class EReloadStage : uint8
{
	Idle,
	RemoveMag,
	InsertMag,
	CloseBolt
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
struct FShotgunData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _pelletCount = 1;

	// 펠렛 분산 정도
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _horizontalSpread = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _verticalSpread = 0.f;
};

USTRUCT(BlueprintType)
struct FGunData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _name = FText::FromString(TEXT("gun"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGunGripType _gripType; // 한손 혹은 두손

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGunSlotType _slotType; // 어느 슬롯에 장착되는지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGunCategory _category; // 총의 종류


	// 데미지 요소
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGunDamageComponent> _damageComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _baseDamage = 80.0f; // 기본 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _vsDurableDamage = 15.0f; // 내구 데미지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPenetrateTrait _penetrateTrait = EPenetrateTrait::Light; // 관통력

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FShotgunData _shotgunData;		// 샷건에 필요한 데이터

	// 총알
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AGunBulletBase> _projectileClass;

	// 발사 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _fireInterval = 60.0f / 640.0f;


	// 재장전 요소
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGunAmmoComponent> _ammoComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReloadType _reloadType;	// 재장전 방식
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool _needAmmoBag = false;	// 재장전할 때 가방이 필요한지

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxAmmo = 45;		// 탄약
							
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _initialSpare = 6;	// 초기 탄창 혹은 여분의 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxSpare = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _refillAmount = 4;

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

	// 발사 모드 관련 요소
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGunFireComponent> _fireComponentClass;
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

	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* _icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UTexture2D* _previewImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _desc;
};

UCLASS()
class SGAPROJECTMAIN_API UGunDataTable : public UObject
{
	GENERATED_BODY()
	
};
