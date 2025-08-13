// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SupplyHellpod.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ASupplyHellpod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASupplyHellpod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Game/HellPod")
	USkeletalMeshComponent* _mesh;

	TArray<class SupplyBox*> _items;
};
