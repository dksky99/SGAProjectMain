// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlayerCurrency.h"
#include "ShopItemTable.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EShopType : uint8
{
	None,
	Gun,
	Stratagem
};

USTRUCT()
struct FOwnedItem
{
	GENERATED_BODY()

	UPROPERTY()
	EShopType _type = EShopType::None;

	UPROPERTY()
	int32 _id = -1;

	bool operator==(const FOwnedItem& Other) const
	{
		return _type == Other._type && _id == Other._id;
	}
};

USTRUCT(BlueprintType)
struct FShopItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _itemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlayerCurrency _price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _condition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EShopType _shopType = EShopType::None;
};

UCLASS()
class SGAPROJECTMAIN_API UShopItemTable : public UObject
{
	GENERATED_BODY()
	
};
