// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Corpse.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ACorpse : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACorpse();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void InitCorpseMesh(USkeletalMeshComponent* meshComp, UMaterialInterface* material = nullptr);
	void CopyPose(USkeletalMeshComponent* meshComp);
private:

		UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* _corpseMesh;

};
