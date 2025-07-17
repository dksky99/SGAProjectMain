// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Backpack.h"
#include "ReloadBackpack.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FReloadBackpackData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _maxBullet = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _refillBulletAmount = 4;
};

UCLASS()
class SGAPROJECTMAIN_API AReloadBackpack : public ABackpack
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

public:
	int32 GetCurBulletCount() { return _curBullet; }
	
private:
	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	FReloadBackpackData _reloadBackpackData;

	int32 _curBullet = 5;
};
