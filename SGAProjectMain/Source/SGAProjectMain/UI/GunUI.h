// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunUI.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UGunUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetAmmo(int32 curAmmo, int32 maxAmmo);

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Ammo;
};
