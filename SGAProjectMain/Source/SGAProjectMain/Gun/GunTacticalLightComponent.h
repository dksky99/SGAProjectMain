// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunAttachmentComponent.h"
#include "GunDataTable.h"
#include "GunTacticalLightComponent.generated.h"

/**
 * 
 */

UCLASS()
class SGAPROJECTMAIN_API UGunTacticalLightComponent : public UGunAttachmentComponent
{
	GENERATED_BODY()
	
public:
	void InitializeAttachment(USkeletalMeshComponent* gunMesh) override;
	void ActivateAttachment(bool isActive, bool isAiming) override;
	void OnAimChanged(bool isAiming) override;

	void ChangeTacticalLightMode(bool isAiming);
	
	ETacticalLightMode GetCurLightMode() { return _curLightMode; }
	TArray<ETacticalLightMode> GetLightModes() { return _lightModes; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Game/Gun")
	class USpotLightComponent* _lightComp;
	
	UPROPERTY()
	TArray<ETacticalLightMode> _lightModes = { ETacticalLightMode::LightAuto, ETacticalLightMode::LightOn, ETacticalLightMode::LightOff };
	UPROPERTY()
	ETacticalLightMode _curLightMode = ETacticalLightMode::LightOff;
	int32 _curLightIndex = 0;
};
