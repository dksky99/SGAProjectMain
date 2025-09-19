// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployDetailBase.h"
#include "PreDeployWeaponDetail.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployWeaponDetail : public UPreDeployDetailBase
{
	GENERATED_BODY()
	
public:
	virtual void SetDetail(int32 id) override;

private:
	float GetStatPercent(float stat, int32 tierSize, bool addOne = false);

	UPROPERTY(meta = (BindWidget))
	class UImage* _equipImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _damageText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _damageBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _maxAmmoText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _maxAmmoBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _recoilText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _recoilBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _fireRateText;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _fireRateBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _traitText;
};
