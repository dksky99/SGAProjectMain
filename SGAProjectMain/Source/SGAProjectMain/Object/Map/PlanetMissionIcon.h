// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetMissionIcon.generated.h"

UCLASS()
class SGAPROJECTMAIN_API APlanetMissionIcon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetMissionIcon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetMissionData(class UMissionDataAsset* mission);
	class UMissionDataAsset* GetMissionData() { return _missionData; }

protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* _mesh;

	UPROPERTY(EditAnywhere)
	class UMissionDataAsset* _missionData;

};
