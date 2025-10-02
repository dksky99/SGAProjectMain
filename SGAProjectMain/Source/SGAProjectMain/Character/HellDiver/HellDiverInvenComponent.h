// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../../Gun/GunBase.h"
#include "../../Object/Item/Backpack.h"
#include "../../Object/Item/SampleResources.h"
#include "HellDiverInvenComponent.generated.h"

UCLASS()
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
	void ApplyLoadOut(class UPreDeploymentState* preDeployState);
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SpawnGun(TSubclassOf<AGunBase> gunClass); // ÃÑ ¼ÒÈ¯
	void SetGun(AGunBase* gun); // ½½·Ô¿¡ ÃÑ ÀåÂø
	void EquipGun(int32 index); // ÇöÀç µé°íÀÖ´Â ÃÑ ¼³Á¤
	void DropGun(int32 index);
	bool CanSwitchGun(int32 index);

	void EquipBackpack(class ABackpack* backpack);
	void DropBackpack();

	void AddSample(struct FSampleBundle sample);
	void DropSample();

	void PutBackWeapon(AGunBase* gun);
	void PutBackMainWeapon();
	void PutBackSubWeapon();
	void PutBackSupportWeapon();
	void BringWeapon(AGunBase* gun);
	TArray<AGunBase*> GetAllGun() { return _gunSlot; } // µé°íÀÖ´Â ÃÑ ¸ñ·Ï Àü´Þ
	AGunBase* GetGunInSlot(int32 index) { return _gunSlot[index]; } // ½½·Ô¿¡ ÀÖ´Â ÃÑ Àü´Þ
	AGunBase* GetEquippedGun() { return _equippedGun; } // ÇöÀç ÀåÂø ÁßÀÎ ÃÑ Àü´Þ
	ABackpack* GetBackpack() { return _backpack; }
	FSampleBundle GetSampleBundle() { return _sampleBundle; }

private:
	UPROPERTY(Transient)
	class AHellDiver* _hellDiver;

	// ÃÑ±â Å¬·¡½º
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Gun", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AGunBase> _gunClass3;

	UPROPERTY(VisibleAnywhere, Category = "Game/Inven")
	AGunBase* _equippedGun;

	UPROPERTY()
	TArray<AGunBase*> _gunSlot;

	UPROPERTY()
	ABackpack* _backpack;

	UPROPERTY(EditAnywhere, Category = "Game/Inven")
	TSubclassOf<class ASampleResources> _sampleClass;

	UPROPERTY()
	FSampleBundle _sampleBundle;
};
