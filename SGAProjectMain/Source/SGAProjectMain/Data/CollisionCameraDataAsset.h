// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CollisionCameraDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCollisionCameraDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float _capsuleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision")
    float _capsuleHalfHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    FVector _cameraOffset;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
    float _springArmLength;
};
