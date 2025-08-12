// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapePlane.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEscapePlane : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEscapePlane();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void EnableTriggerBox();

protected:
	UPROPERTY(EditAnywhere, Category = "Game")
	USkeletalMeshComponent* _planeMesh;

	UPROPERTY(EditAnywhere, Category = "Game/Animation")
	UAnimMontage* _planeMontage;

	UPROPERTY(EditAnywhere, Category = "Game")
	class UBoxComponent* _escapeTriggerBox; // 문 부분에 위치

	UPROPERTY(EditAnywhere, Category = "Game")
	FVector _targetLocation;

	bool _isLanding = true;
	bool _isEscapeEnabled = false;
};
