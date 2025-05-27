// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunSettingUI.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UGunSettingUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateGunInfo(const struct FGunData& gunData, int32 curAmmo);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* _gunNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* _curAmmoText;
};
