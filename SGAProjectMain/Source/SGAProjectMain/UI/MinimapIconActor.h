// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MinimapIconActor.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AMinimapIconActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMinimapIconActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void FindSceneCapturer();

	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	TObjectPtr<UStaticMeshComponent> _mesh;

	UPROPERTY()
	class ASceneCapturer* _sceneCapturer;
};
