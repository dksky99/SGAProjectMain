// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StimPackComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FStimChanged, int, int);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStimPackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStimPackComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UseStimPack();
	void RefillStimPack();

	// 회복 콜백
	void HandleRegen();

	// 내부 헬퍼: 타이머 시작/연장
	void StartOrExtendRegen();

	FStimChanged _stimChanged;

private:
	UPROPERTY(VisibleAnywhere, Category = "Game/StimPack")
	class AHellDiver* _owner;

	UPROPERTY(EditDefaultsOnly, Category = "Game/StimPack")
	int32 _maxStimPack = 4;

	UPROPERTY(VisibleAnywhere, Category = "Game/StimPack")
	int32 _curStimPack = 2;

	FTimerHandle _regenTimerHandle;

	// 회복 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Game/StimPack")
	float TotalRegenDuration = 4.0f;

	// 회복 틱 간격
	UPROPERTY(EditDefaultsOnly, Category = "Game/StimPack")
	float RegenInterval = 0.05f;

	// 회복 량
	UPROPERTY(EditDefaultsOnly, Category = "Game/StimPack")
	float AmountPerTick = 10.0f;

	// 남은 틱 카운터
	int32 _ticksElapsed = 0;
	int32 _maxTicks = 0;

	
};
