// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunAttachmentComponent.h"
#include "GunLaserComponent.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UGunLaserComponent : public UGunAttachmentComponent
{
	GENERATED_BODY()

public:
	UGunLaserComponent();

	virtual void InitializeAttachment(USkeletalMeshComponent* gunMesh);
	virtual void OnAimChanged(bool isAiming);

	virtual void UpdateAttachment(USkeletalMeshComponent* gunMesh, FVector hitPoint);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* _laserNS;
	UPROPERTY()
	class UNiagaraComponent* _laserEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* _laserImpactNS;
	UPROPERTY()
	class UNiagaraComponent* _laserImpactEffect;
};
