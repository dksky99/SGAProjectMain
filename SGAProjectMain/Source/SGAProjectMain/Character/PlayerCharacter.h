// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Idle,
	Firing,
	CookingGrenade,
	StratagemInputting,
	Rolling,
	Reloading,
	// 필요하면 추가
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

UCLASS()
class SGAPROJECTMAIN_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
protected:
	// Called when the game starts or when spawned

public:
	// Called every frame

	UFUNCTION()
	void Move(const  FInputActionValue& value);
	UFUNCTION()
	void Look(const  FInputActionValue& value);
	UFUNCTION()
	void TryJump(const  FInputActionValue& value);
	UFUNCTION()
	void StartFiring(const  FInputActionValue& value);
	UFUNCTION()
	void WhileFiring(const  FInputActionValue& value);
	UFUNCTION()
	void StopFiring(const  FInputActionValue& value);
	UFUNCTION()
	void StartAiming(const  FInputActionValue& value);
	UFUNCTION()
	void WhileAiming(const  FInputActionValue& value);
	UFUNCTION()
	void StopAiming(const  FInputActionValue& value);


protected:



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _moveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _lookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _jumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _mouseLButtonAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _mouseRButtonAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* _camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* _springArm;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AGunBase* _equippedGun;

	EPlayerState _playerState;
	EWeaponType _weaponType;

	bool _isAiming = false;
};
