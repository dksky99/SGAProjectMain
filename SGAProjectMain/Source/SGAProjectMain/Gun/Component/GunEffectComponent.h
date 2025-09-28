// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunEffectComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UGunEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunEffectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void InitializeEffect(USkeletalMeshComponent* gunMesh);

	void PlayFireEffect();
		
protected:
	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	class UNiagaraSystem* _fireNS;
	UPROPERTY()
	class UNiagaraComponent* _fireEffect;

	UPROPERTY(EditAnywhere, Category = "Game/Gun")
	class UNiagaraSystem* _shellEjectNS;
	UPROPERTY()
	class UNiagaraComponent* _shellEjectEffect;
};
