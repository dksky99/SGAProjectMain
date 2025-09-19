// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunBase.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AShotgun : public AGunBase
{
	GENERATED_BODY()
	
protected:
	virtual void ExecuteShot() override;

private:
	// ÇÑ ¹ø¿¡ ¹ß»çµÇ´Â Æç·¿ ¼ö
	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	int32 _pelletCount = 9;

	// Æç·¿ ºÐ»ê Á¤µµ
	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	float _horizontalSpread = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Game/GunData")
	float _verticalSpread = 120.0f;
};
