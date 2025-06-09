// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStateComponent.generated.h"



UENUM(BlueprintType, meta = (Bitflags))
enum class EAbnormalityState : uint8
{
	None = 0 UMETA(Hidden),
	Poison = 1 << 0,
	Burn = 1 << 1,
	Shock = 1 << 2,
	Slow = 1 << 3,

};
ENUM_CLASS_FLAGS(EAbnormalityState) 


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddAbnormality(EAbnormalityState abnormality);
	void SubAbnormality(EAbnormalityState abnormality);

	bool CheckAbnormality(EAbnormalityState abnormality);


	bool IsMoving();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EAbnormalityState"))
	int32 _activeAbnormalities;

		
};
