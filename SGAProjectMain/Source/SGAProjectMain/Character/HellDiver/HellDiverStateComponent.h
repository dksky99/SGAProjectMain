// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterStateComponent.h"
#include "HellDiverStateComponent.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Standing,
	Sprinting,
	Crouching,
	Proning,
	knockdown,
	MAX
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	None,
	TPSAim,
	FPSAim,
	Stratagem,
	ViewMap,
	InterActing,
	MAX
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	PrimaryWeapon,
	SecondaryWeapon,
	Grenade,
	StratagemDevice,
	// 필요하면 추가
};
UENUM(BlueprintType)
enum class ELifeState : uint8
{
	Alive,
	Downed,
	Dead,
	Respawning
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

	
	bool StartSprint();
	bool FinishSprint();

	bool StartCrouch();
	bool FinishCrouch();
	

	bool StartProne();
	bool FinishProne();

	ECharacterState GetCharacterState() { return _characterState; }
	void SetCharacterState(ECharacterState state) { _characterState=state; }
	EActionState GetActionState() { return _actionState; }
	void SetActionState(EActionState state) { _actionState = state; }
	EWeaponType GetWeaponState() { return _weaponState; }
	void SetWeaponState(EWeaponType state) { _weaponState = state; }
	ELifeState GetLifeState() { return _lifeState; }
	void SetLifeState(ELifeState state) { _lifeState = state; }


	bool StartTPSAiming();
	bool FinishTPSAiming();


protected:




	ECharacterState _characterState=ECharacterState::Standing;

	EActionState _actionState = EActionState::None;

	EWeaponType _weaponState = EWeaponType::None;

	ELifeState _lifeState = ELifeState::Alive;

	bool _isFiring = false;
	bool _isReloading = false;
	bool _isRolling = false;
		
};
