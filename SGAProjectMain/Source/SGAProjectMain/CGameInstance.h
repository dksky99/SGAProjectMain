// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Object/Item/SampleResources.h"
#include "CGameInstance.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FSelectedStratagemSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<class AStratagem>> SelectedStratagems;
};

UCLASS()
class SGAPROJECTMAIN_API UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:


	UCGameInstance();
	UPROPERTY(BlueprintReadWrite)
	TMap<FName, FSelectedStratagemSet> AllPlayerStratagemSets;

	struct FGunData GetGunDataFromTable(int32 id);

	void AddEarnedSample(const FSampleBundle& earnedSample);
	FSampleBundle GetSavedSample() { return _savedSample; }

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMusic(int32 index = 0);

	virtual void Init() override;

	void C_OnWorldCreated(UWorld* world) ;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio")
	TArray<class USoundCue*> _bgmCues;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio")
	class UAudioComponent* _bgmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio")
	int32 _cueIndex;



private:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UDataTable* _gunTable;

	UPROPERTY(EditAnywhere, Category = "Sample")
	FSampleBundle _savedSample;
};
