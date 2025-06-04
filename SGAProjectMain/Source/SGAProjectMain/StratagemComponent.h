// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StratagemComponent.generated.h"

USTRUCT(BlueprintType)
struct FStratagemSlot
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere)
	FName StratagemID;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AStratagem> StratagemClass;

	UPROPERTY(EditAnywhere)
	float Cooldown = 10.f;

	float LastUsedTime = -9999.f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStratagemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStratagemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void TryUseCurrentStratagem();
	
	TSubclassOf<class AStratagem> GetSelectedStratagemClass() const;
	void SelectStratagem(int32 Index);

	void CommitStratagemUse();
	int32 FindStratagemByInputSequence(const TArray<FKey>& InputSequence) const;

	bool IsStratagemOnCooldown(int32 SlotIndex) const;
	float GetRemainingCooldown(int32 SlotIndex) const;

	const TArray<FStratagemSlot>& GetStratagemSlots() const { return StratagemSlots; }
		
protected:
	UPROPERTY(EditAnywhere, Category = "Game/Stratagem")
	TArray<FStratagemSlot> StratagemSlots;

	UPROPERTY(VisibleAnywhere, Category = "Game/Stratagem")
	int32 CurrentSlotIndex = 0;
};
