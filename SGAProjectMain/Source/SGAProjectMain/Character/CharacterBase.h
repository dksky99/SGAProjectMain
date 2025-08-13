// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"



UCLASS()
class SGAPROJECTMAIN_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	void PostInitializeComponents() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void UpDown(float value);
	void RightLeft(float value);


	float MyVertical() { return _vertical; }
	float MyHorizontal() { return _horizontal; }
	float MyDeltaAngle() { return _deltaAngle; }
	float MyYaw() { return _yaw; }
	float MyPitch() { return _pitch; }

	FText GetCharacterName() { return _name; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual void Landed(const FHitResult& Hit) override;

	bool _isTurnLeft = false;
	bool _isTurnRight = false;

	void KnockDown(float time);

	void KnockDownRecovery();

	//virtual float TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;
	void CharacterToRagdoll();
	virtual void KnockDown();
	virtual void RecoverFromKnockDown();
	virtual void Dead();


	virtual FVector GetTargetLoc() {};

	virtual void SpawnGhost();

	virtual void ResetUnit();

	class UStatComponent* GetStatComponent() ;
	class UCharacterStateComponent* GetStateComponent();
	UFUNCTION()
	virtual void OnPartDestroyed_Handler(EBodyPart part); // 상속받아서 각 부위별 부위파괴 구현

	UFUNCTION()
	virtual void RestoreAllParts(); // 모든 파손부위 복구 (파손체크 x)

	UFUNCTION()
	virtual void OnDeath_Handler(); // 상속받아서 죽었을 시 동작 구현

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stat")
	class UStatComponent* _statComponent;

	static const FName StatComponentName;

	float _vertical = 0;
	float _horizontal = 0;

	float _deltaAngle = 0.0f;

	float _yaw=0.0f;
	float _pitch = 0.0f;
	bool _isViewTurnCenter=false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/State", meta = (AllowPrivateAccess = "true"))
	class UCharacterStateComponent* _stateComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FText _name;

	FTimerHandle _knockDownTimerHandle;

	

};
