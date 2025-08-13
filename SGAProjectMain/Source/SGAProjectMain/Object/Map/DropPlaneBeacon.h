// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerminalOperable.h"
#include "DropPlaneBeacon.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API ADropPlaneBeacon : public ATerminalOperable
{
	GENERATED_BODY()

public:
	ADropPlaneBeacon();
	
	virtual void OnCommandCompleted() override;
	void SetInteractable(bool isInteractable);

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Mesh")
	UStaticMeshComponent* _mesh;
};
