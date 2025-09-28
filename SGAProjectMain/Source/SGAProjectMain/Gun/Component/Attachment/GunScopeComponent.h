// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunAttachmentComponent.h"
#include "GunScopeComponent.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SGAPROJECTMAIN_API UGunScopeComponent : public UGunAttachmentComponent
{
	GENERATED_BODY()
	
public:
	void InitializeAttachment(USkeletalMeshComponent* gunMesh) override;
	void OnAimChanged(bool isAiming) override;

	void ChangeScopeMode();
	
	int32 GetCurScopeMode() { return _curScopeMode; }
	TArray<int32> GetScopeModes() { return _scopeModes; }

protected:
	UPROPERTY(EditAnywhere)
	TArray<int32> _scopeModes;
	int32 _curScopeMode;

	int32 _curScopeIndex = 0;
};
