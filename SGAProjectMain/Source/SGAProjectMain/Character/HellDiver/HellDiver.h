// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "HellDiver.generated.h"



UCLASS()
class SGAPROJECTMAIN_API AHellDiver : public ACharacterBase
{
	GENERATED_BODY()
public:

	AHellDiver(const FObjectInitializer& ObjectInitializer);

	class UHellDiverStateComponent* GetStateComponent();

 

	void StartSprint();
	void FinishSprint();

	void StartCrouch();
	void FinishCrouch();


	void StartProne();
	void FinishProne();

	void Rolling();
	void FinishRolling();


	virtual void Landed(const FHitResult& Hit) override;

protected: 

	FTimerHandle _rollingTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStatComponent* _statComponent;

};
