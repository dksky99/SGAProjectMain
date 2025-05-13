// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "HellDiver.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AHellDiver : public ACharacterBase
{
	GENERATED_BODY()
public:

	AHellDiver(const FObjectInitializer& ObjectInitializer);

	class UHellDiverStateComponent* GetStateComponent();

	void EquipGrenade();
	void EquipStratagem();
	void OnThrowReleased();

	void StartSprint();
	void FinishSprint();

protected:
	FTransform GetHandSocketTransform() const;

protected: 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	
	UPROPERTY()
	class AThrowable* _heldThrowable = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATimedGrenadeBase> _grenadeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwables")
	ATimedGrenadeBase* _equippedGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AStratagem> _stratagemClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwables")
	AStratagem* _equippedStratagem;

};
