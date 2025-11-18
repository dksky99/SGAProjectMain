// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AbnormalityTable.generated.h"

/**
 * 
 */

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
UENUM(BlueprintType)
enum class EAbnormalityType : uint8
{
	None,
	Fire,
	Gas,
	Acid,
	bleeding,
	Shock,


	Max
};



USTRUCT(BlueprintType)
struct FAbnormalityData : public FTableRowBase
{
	GENERATED_BODY()
	


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbnormality _statusID = EAbnormality::Max;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName _name = TEXT("unit");

	//관통레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _penetrationLevel = 4;
	
	//일반피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _normalDamage = 0;

	//내구피해
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _durabilityDamage = 0;

	//지속시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _stateDuration = 0.f;

	//부여가중치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _stateWeight = 0.f;

	//상태이상의 분류.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbnormalityType _abnormalityType = EAbnormalityType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> _damageType;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _desc;
};



UCLASS()
class SGAPROJECTMAIN_API UAbnormalityTable : public UObject
{
	GENERATED_BODY()
	
};
