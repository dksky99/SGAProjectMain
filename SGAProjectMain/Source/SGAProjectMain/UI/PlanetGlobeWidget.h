// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlanetGlobeWidget.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPlanetGlobeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOperation(class APlanetOperationSite* site, bool _visibility);

protected:
	UPROPERTY(meta=(BindWidget))
	class UVerticalBox* _operationBox;
	
};
