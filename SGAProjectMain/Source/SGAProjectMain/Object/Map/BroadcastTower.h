// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TerminalOperable.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "BroadcastTower.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ABroadcastTower : public ATerminalOperable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABroadcastTower();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//UFUNCTION()
	//void OnBreakEvent(const FChaosBreakEvent& BreakEvent); // 부서지는 트리거

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void OnCommandCompleted() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> _geometryCollection;
};
