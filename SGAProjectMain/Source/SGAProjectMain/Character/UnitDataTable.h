// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UnitDataTable.generated.h"

/*
 	유닛에 대한 테이블 
	캐릭터에겐 부위별로 파괴시 사망하거나 특수한 효과를 주는 부위, 혹은 파괴되면 출혈로 몇초뒤에 사망하는부위, 아무 상관없는 부위로 구분을 지을 예정
	상관이있는부위는 별도의 hp를 갖고 따로 상관이없는 부위는 피격시 메인에서 체력이 소모됨.

	피해를 입는 방식은 파트의 내구력에 따라 (0~100) 입는 피해가 2종류 있는데 일반과 내구피해중에 내구력의 수치만큼 내구피해로 입음
	예를들어 30의 내구력이라면 일반피해의 70퍼센트와 내구피해의 30퍼센트를 받게됨.

	그리고 부위별로 영향력이 존재 그 영향력만큼 입은피해를 메인 hp를 감소. 


 */

 // 부위 식별용 열거형: 부위를 추가함. 벌레는 팔다리가 6개에 머리가슴배로이뤄져있다 그러니 마땅히 일단 10개로구분시키자.
UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Core,
	Head,
	Torso,
	Tail,
	LeftArm,
	RightArm,
	LeftLeg,
	RightLeg,
	LeftClaw,
	RightClaw,
	Max
};


UENUM(BlueprintType)
enum class EAbnormality : uint8
{
	Fire = 0,
	Burn = 1,
	Gas,
	AcidBubble,
	AcidStream,
	bleeding,
	Thornbush,
	LightStagger,
	StrongStagger,
	Shock,


	Max
};


USTRUCT(BlueprintType)
struct FUnitPartLayerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerName = ""; 
	// 파트별 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _partHP = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _partAV = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partDurability = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partInfluence = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partExplosionImmunity = 0.f;
};

USTRUCT(BlueprintType)
struct FUnitAbnormalResistData
{
	GENERATED_BODY()
	//휘청거림을 위한 저항력. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _stagger1 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _stagger2 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _stagger3 = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _stagger4 = 1;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _minBurn = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _maxBurn = 1;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _minGas = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _maxGas = 0.5;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _minStun = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _maxStun = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _minThermite = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _maxThermite = 1;

};

USTRUCT(BlueprintType)
struct FPartDefinitionRow : public FTableRowBase // FTableRowBase 상속
{
	GENERATED_BODY()

	// "껍질 -> 속살" 순서로 레이어를 배열에 추가합니다.
	// [0] = 껍질 (예: Steel Plate)
	// [1] = 속살 (예: Flesh)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FUnitPartLayerData> Layers;
};

USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _name = FText::FromString(TEXT("unit"));


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EBodyPart,FText> _PartIDs;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _defaultMovementSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _battleMovementSpeed = 500.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUnitAbnormalResistData _resistData;







	// 클래스나 블루프린트 지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ACharacterBase> _unitClass;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _desc;
};


UCLASS()
class SGAPROJECTMAIN_API UUnitDataTable : public UObject
{
	GENERATED_BODY()
	
};
