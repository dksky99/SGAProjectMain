// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunAttachmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UGunAttachmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunAttachmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	virtual void InitializeAttachment(USkeletalMeshComponent* gunMesh) {}
	virtual void ActivateAttachment(bool isActive, bool isAiming = false) { OnAimChanged(isAiming); }
	virtual void OnAimChanged(bool isAiming) {}

	virtual void UpdateAttachment(USkeletalMeshComponent* gunMesh, FVector loc) {}
};
