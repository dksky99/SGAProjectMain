// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterStateComponent.h"
#include "HellDiverStateComponent.generated.h"

UENUM()
enum class ECharacterState : uint8
{
	Standing,
	Sprinting,
	Crouching,
	Proning,
	knockdown,
	MAX
};

UENUM()
enum class EActionState : uint8
{
	None,
	TPSAim,
	FPSAim,
	ThrowAim,
	Stratagem,
	ViewMap,
	InterActing,
	MAX
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UHellDiverStateComponent : public UCharacterStateComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHellDiverStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartSprint();
	void FinishSprint();

	void StartCroutch();
	void FinishCroutch();
	
	void StartProne();
	void FinishProne();




protected:

	ECharacterState _characterState=ECharacterState::Standing;

	EActionState _actionState = EActionState::None;


		
};
