// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddHP(float amount); // HP�� ���ϰų� ���� ���� �����Ѵٸ� ������ ���ؾ��Ѵ�
	bool IsDead();
	void Dead(); // �׾��� �� �� �۾� �߰�

	float GetDefaultSpeed() { return _defaultSpeed; }

protected:

	float _defaultSpeed = 300.0f;

	float _hp = 100.0f;
	float _maxHp = 100.0f;

		
};
