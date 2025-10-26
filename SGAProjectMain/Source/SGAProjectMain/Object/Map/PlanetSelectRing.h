// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetSelectRing.generated.h"

UCLASS()
class SGAPROJECTMAIN_API APlanetSelectRing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetSelectRing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PlaceOnSurface(FVector point, FVector normalVec, FVector upTangent);

	class UStaticMeshComponent* GetMesh() { return _mesh; }

protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* _mesh;

};
