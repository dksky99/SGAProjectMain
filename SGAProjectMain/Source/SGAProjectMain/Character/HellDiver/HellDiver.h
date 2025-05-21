// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CharacterBase.h"
#include "HellDiverStateComponent.h"
#include "HellDiver.generated.h"



UCLASS()
class SGAPROJECTMAIN_API AHellDiver : public ACharacterBase
{
	GENERATED_BODY()
public:

	AHellDiver(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	class UHellDiverStateComponent* GetStateComponent();

	void EquipGrenade();
	void EquipStratagem();
	void OnThrowReleased();

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
	FTransform GetHandSocketTransform() const;
	FTransform GetEquip1SocketTransform() const;
	FTransform GetEquip2SocketTransform() const;
	FTransform GetEquip3SocketTransform() const;



	UFUNCTION()
	void SetCollisionState(ECharacterState newState);
	void SetCollisionCamera(class UCollisionCameraDataAsset data);
	virtual void SetStandingCollisionCamera();
	virtual void SetCrouchingCollisionCamera();
	virtual void SetProningCollisionCamera() ;
protected: 

	FTimerHandle _rollingTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStateComponent* _stateComponent;
	
	UPROPERTY()
	class AThrowable* _heldThrowable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", meta = (AllowPrivateAccess = "true"))
	class UHellDiverStatComponent* _statComponent;
	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ATimedGrenadeBase> _grenadeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Throwables")
	ATimedGrenadeBase* _equippedGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Throwables", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AStratagem> _stratagemClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Throwables")
	AStratagem* _equippedStratagem;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta = (AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _standingStance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta=(AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _crouchingStance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stance", meta=(AllowPrivateAccess = "true"))
	class UCollisionCameraDataAsset* _proningStance;


};
