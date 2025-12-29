// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetMissionIcon.h"
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

	void ChangeToFocusMode();
	void ChangeToBrowseMode();

	void SetOperationData(class UOperationDataAsset* operationData) { _operationData = operationData; }
	class UOperationDataAsset* GetOperationData() const { return _operationData; }

	void SetOperationIndex(int32 index) { _operationIndex = index; }
	int32 GetOperationIndex() const { return _operationIndex; }

protected:
	void ShowMissionIcons(bool bShow);

	UPROPERTY(EditAnywhere, Category = "Game/Data")
	class UOperationDataAsset* _operationData = nullptr;

	UPROPERTY(VisibleAnywhere)
	UDecalComponent* _regionDecal = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Game/Icons")
	UChildActorComponent* _mainIcon;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> _missionIcons;

	UPROPERTY(VisibleAnywhere, Category = "Game/Icons")
	UChildActorComponent* _iconA;
	UPROPERTY(VisibleAnywhere, Category = "Game/Icons")
	UChildActorComponent* _iconB;
	UPROPERTY(VisibleAnywhere, Category = "Game/Icons")
	UChildActorComponent* _iconC;

	UPROPERTY(EditAnywhere, Category = "Game/Icons") bool _bUseA = true;
	UPROPERTY(EditAnywhere, Category = "Game/Icons") bool _bUseB = false;
	UPROPERTY(EditAnywhere, Category = "Game/Icons") bool _bUseC = false;

	UPROPERTY(EditAnyWhere, Category = "Game/Icons")
	int32 _iconCount = 3;

	int32 _operationIndex = -1;
};
