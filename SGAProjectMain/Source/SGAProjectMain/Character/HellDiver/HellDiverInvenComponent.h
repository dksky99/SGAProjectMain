// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Object/Item/SampleResources.h"
#include "HellDiverInvenComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UHellDiverInvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHellDiverInvenComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	int32 SetGun(class AGunBase* gun);
	void EquipGun(int32 index);
	void DropGun(int32 index);
	bool CanSwitchGun(int32 index);

	void EquipBackpack(class ABackpack* backpack);
	void DropBackpack();

	void AddSample(struct FSampleBundle sample);
	void DropSample();

	TArray<AGunBase*> GetAllGun() { return _gunSlot; } // 들고있는 총 목록 전달
	AGunBase* GetEquippedGun() { return _equippedGun; } // 현재 장착 중인 총 전달
	ABackpack* GetBackpack() { return _backpack; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Game/Inven")
	AGunBase* _equippedGun;

	UPROPERTY()
	TArray<AGunBase*> _gunSlot;

	UPROPERTY()
	ABackpack* _backpack;

	UPROPERTY(EditAnywhere, Category = "Game/Inven")
	TSubclassOf<class ASampleResources> _sampleClass;

	UPROPERTY()
	struct FSampleBundle _sampleBundle;
};
