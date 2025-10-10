// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GalacticPlanetGlobe.generated.h"

USTRUCT(BlueprintType)
struct FOperationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _latitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _longitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class APlanetOperationSite> OperationSiteClass;
};

UCLASS()
class SGAPROJECTMAIN_API AGalacticPlanetGlobe : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGalacticPlanetGlobe();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeOperations();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StartInteracting();
	void StopInteracting();

protected:
	FVector CalculateGlobePosition(float latitude, float longitude, float globeRadius);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _globeRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOperationData> _operations;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> _operationSites;

	UPROPERTY(EditAnywhere, Category = "Game/Mesh")
	UStaticMeshComponent* _mesh;

	UPROPERTY()
	APlayerController* _playerController;

	bool _isInteracting = false;

	float _curPitchDeg = 0.f;
};
