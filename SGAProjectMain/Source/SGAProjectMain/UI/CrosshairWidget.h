// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Data/GunDataTable.h"
#include "CrosshairWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowHitMarker(EHitOutcome hitOutcome);
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* _hitMarker;

	UPROPERTY(EditAnywhere, Category = "Game/Icon")
	class UTexture2D* _fullDmgIcon;
	UPROPERTY(EditAnywhere, Category = "Game/Icon")
	class UTexture2D* _weakDmgIcon;
	UPROPERTY(EditAnywhere, Category = "Game/Icon")
	class UTexture2D* _ricochetIcon;

	UPROPERTY(EditAnywhere, Category = "Game/Icon")
	FVector2D _ricochetOffset;

	FTimerHandle _hitMarkerTimerHandle;
};
