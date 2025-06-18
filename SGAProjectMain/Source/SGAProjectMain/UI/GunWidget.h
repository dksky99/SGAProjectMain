// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GunWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UGunWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetAmmo(int32 curAmmo, int32 maxAmmo);
	void SetMag(int32 curMag, int32 maxMag);
	void SetHp(float ratio);
	void SetGrenade(int32 curGrenade, int32 maxGrenade);
	void SetInjector(int32 curInjector, int32 maxInjector);

private:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _curAmmoPB;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* _curHpPB;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _grenadeCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _magCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* _injectorCount;
};
