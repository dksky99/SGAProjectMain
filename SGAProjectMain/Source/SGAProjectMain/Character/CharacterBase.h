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

	FText GetCharacterName() { return _name; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	bool _isTurnLeft = false;
	bool _isTurnRight = false;
protected:

	float _vertical = 0;
	float _horizontal = 0;

	float _deltaAngle = 0.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FText _name;

};
