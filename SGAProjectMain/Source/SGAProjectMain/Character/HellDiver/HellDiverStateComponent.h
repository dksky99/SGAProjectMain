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
	Gun,
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterStateChanged, ECharacterState, newState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStateChanged, EActionState, newState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponStateChanged, EWeaponType, newState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLifeStateChanged, ELifeState, newState);


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

	bool StartRolling();
	bool FinishRolling();

	ECharacterState GetCharacterState() { return _characterState; }
	void SetCharacterState(ECharacterState state) { _characterState=state; }
	EActionState GetActionState() { return _actionState; }
	void SetActionState(EActionState state) { _actionState = state; }
	ELifeState GetLifeState() { return _lifeState; }
	void SetLifeState(ELifeState state) { _lifeState = state; }

	bool IsRolling() { return _isRolling; }
	bool IsReloading() { return _isReloading; }
	void SetReloading(bool isReloading) { _isReloading = isReloading; }
	bool IsFiring() { return _isFiring; }
	void SetFiring(bool isFiring) { _isFiring = isFiring; }
	bool IsAiming() { return _isAiming; }
	void SetAiming(bool isAiming) { _isAiming = isAiming; }

	bool IsFocusing();
	bool IsCookingGrenade() { return _isCookingGrenade; }
	void SetCookingGrenade(bool isCookingGrenade) { _isCookingGrenade = isCookingGrenade; }
	bool IsInputtingStratagem(){ return _isInputtingStratagem; }
	void SetInputtingStratagem(bool isInputtingStratagem) { _isInputtingStratagem = isInputtingStratagem; }

	bool IsMotionChanging() { return _isMotionChange; }
	bool IsWeaponChanging() { return _isWeaponChange; }

	UFUNCTION()
	void MoveChangeFinish(FString newState);
	UFUNCTION()
	void LookChangeFinish(FString newState);

	bool StartTPSAiming();
	bool FinishTPSAiming();
	EWeaponType GetWeaponState() { return _weaponState; }

	void SetWeaponState(EWeaponType weapon) { _weaponState = weapon; }

	void SetEquipIndex(uint8 index) { _curEquipIndex = index; }
	uint8 GetEquipIndex() { return _curEquipIndex; }

	FOnCharacterStateChanged _characterStateChanged;
	FOnActionStateChanged _actionStateChanged;
	FOnWeaponStateChanged _weaponStateChanged;
	FOnLifeStateChanged _lifeStateChanged;
protected:
	bool TryMotionChange();
	bool TryWeaponChange();
protected:




	ECharacterState _characterState=ECharacterState::Standing;

	EActionState _actionState = EActionState::None;

	EWeaponType _weaponState = EWeaponType::None;

	ELifeState _lifeState = ELifeState::Alive;

	bool _isMoving = false;
	bool _isFiring = false;
	bool _isReloading = false;
	bool _isRolling = false;
	bool _isAiming = false;
	bool _isCookingGrenade = false;
	bool _isInputtingStratagem = false;

	bool _isMotionChange = false;
	bool _isWeaponChange = false;

	uint8 _curEquipIndex = 0;

	FString _waitingMove;
	FString _waitingLook;
		
};
