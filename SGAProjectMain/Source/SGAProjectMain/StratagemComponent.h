// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StratagemComponent.generated.h"

// 변경 전 스트라타젬 슬롯 필요없으면 지워주세요

//USTRUCT(BlueprintType)
//struct FStratagemSlot : public FTableRowBase
//{
//	GENERATED_BODY();
//
//	UPROPERTY(EditAnywhere)
//	FName StratagemID;
//
//	UPROPERTY(EditAnywhere)
//	TSubclassOf<class AStratagem> StratagemClass;
//
//	UPROPERTY(EditAnywhere)
//	float Cooldown = 10.f;
//
//	float LastUsedTime = -9999.f;
//
//	UPROPERTY(EditAnywhere)
//	FText Description;
//};

USTRUCT(BlueprintType)
struct FStratagemSlot : public FTableRowBase
{
	GENERATED_BODY();

	// 스트라타젬을 구분하는 ID (DT 키 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	FName StratagemID;

	// 실제 스트라타젬 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	TSubclassOf<class AStratagem> StratagemClass;

	// 개별 쿨타임 (0.0f 이면 개인 쿨 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float Cooldown = 10.0f;

	// 마지막 사용 시간 (런타임 전용)
	float LastUsedTime = -9999.0f;

	// 이글, 궤도폭격 같은 공유 쿨 그룹 이름 (없으면 NAME_None)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	FName CooldownGroup = NAME_None;

	// 그룹 쿨다운 길이 (0.0f 이면 그룹 쿨 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float GroupCooldown = 0.0f;

	// 최대 탄수 (0 이면 무제한)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	int32 MaxCharges = 0;

	// 현재 남은 탄수 (런타임 전용, ApplyLoadOut 등에서 초기화)
	int32 CurrentCharges = 0;

	// 설명 텍스트 (UI 용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	FText Description;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStratagemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStratagemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void ApplyLoadOut(class UPreDeploymentState* preDeployState);

	void TryUseCurrentStratagem();
	
	TSubclassOf<class AStratagem> GetSelectedStratagemClass() const;
	void SelectStratagem(int32 Index);

	void CommitStratagemUse();
	int32 FindStratagemByInputSequence(const TArray<FKey>& InputSequence) const;

	bool HasUsedAnyEagleAmmo() const;

	// 이 그룹이 재무장 슬롯 때문에 잠겨 있다면, 남은 재무장 쿨타임(초)을 반환합니다. 없으면 0.0f
	float GetRemainingRearmCooldownForGroup(FName cooldownGroup) const;

	// 이글 재무장을 실제로 적용합니다 (탄수 회복 + 재무장 쿨 세팅)
	void ApplyEagleRearm();

	bool IsStratagemOnCooldown(int32 SlotIndex) const;
	float GetRemainingCooldown(int32 SlotIndex) const;

	const TArray<FStratagemSlot>& GetStratagemSlots() const { return StratagemSlots; }
		
protected:
	UPROPERTY(EditAnywhere, Category = "Game/Stratagem")
	TArray<FStratagemSlot> StratagemSlots;

	UPROPERTY(VisibleAnywhere, Category = "Game/Stratagem")
	int32 CurrentSlotIndex = 0;

	// 이글, 궤도폭격 등 공유 쿨을 사용하는 그룹의 마지막 사용 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	TMap<FName, float> _groupLastUsedTime;
};
