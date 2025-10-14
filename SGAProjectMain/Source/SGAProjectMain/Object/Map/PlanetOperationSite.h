// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetObjectiveIcon.h"
#include "PlanetOperationSite.generated.h"

UCLASS()
class SGAPROJECTMAIN_API APlanetOperationSite : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlanetOperationSite();

protected:
	virtual void OnConstruction(const FTransform& Xform) override;
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ChangeToFocusedSite();

protected:
	void ShowObjectiveIcons(bool bShow);

	UPROPERTY(VisibleAnywhere)
	UDecalComponent* _regionDecal = nullptr;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> _objectiveIcons;

	UPROPERTY(VisibleAnywhere, Category = "Icons")
	UChildActorComponent* _mainIcon;

	UPROPERTY(VisibleAnywhere, Category = "Icons")
	UChildActorComponent* _iconA;
	UPROPERTY(VisibleAnywhere, Category = "Icons")
	UChildActorComponent* _iconB;
	UPROPERTY(VisibleAnywhere, Category = "Icons")
	UChildActorComponent* _iconC;

	UPROPERTY(EditAnywhere, Category = "Icons") bool bUseA = true;
	UPROPERTY(EditAnywhere, Category = "Icons") bool bUseB = false;
	UPROPERTY(EditAnywhere, Category = "Icons") bool bUseC = false;

	UPROPERTY(EditAnyWhere, Category = "Game/Icon")
	int32 _iconCount = 3;
};
