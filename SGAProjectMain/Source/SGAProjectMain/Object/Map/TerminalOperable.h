// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerminalOperable.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ATerminalOperable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATerminalOperable();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void OnCommandCompleted();

	UPROPERTY(VisibleAnywhere)
	UChildActorComponent* _terminalChild;
};
