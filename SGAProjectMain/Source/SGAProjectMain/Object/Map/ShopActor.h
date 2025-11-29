// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Interactable.h"
#include "ShopActor.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AShopActor : public AInteractable
{
	GENERATED_BODY()
	
public:
	void Interact(class AHellDiver* player) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<class UUserWidget> _shopWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _gameIMC;
	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _widgetIMC;
};
