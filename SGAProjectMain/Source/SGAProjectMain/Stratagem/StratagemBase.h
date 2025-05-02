// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StratagemBase.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AStratagemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStratagemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



protected:
	class APlayerCharacter* _owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stratagem")
	AActor* _targetActor; // ��Ʈ��Ÿ���� ������ ��ġ

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stratagem")
	float _deployDelay = 5.0f; // ���ʵڿ� ��������

	FTimerHandle _deployTimerHandle;

};
