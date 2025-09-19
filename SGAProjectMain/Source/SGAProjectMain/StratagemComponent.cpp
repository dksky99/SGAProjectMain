// Fill out your copyright notice in the Description page of Project Settings.


#include "StratagemComponent.h"
#include "Object/Stratagem/Stratagem.h"
#include "Game/PreDeployment/PreDeploymentState.h"
#include "CGameInstance.h"

// Sets default values for this component's properties
UStratagemComponent::UStratagemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStratagemComponent::BeginPlay()
{
	Super::BeginPlay();

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	ApplyLoadOut(GI->GetPreDeployState());
}

void UStratagemComponent::ApplyLoadOut(UPreDeploymentState* preDeployState)
{
	if (!preDeployState) return;

	auto GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	TArray<int32> stratagemIDs = preDeployState->GetStratagemIDs();
	TArray<int32> defaultIDs = { -1, -1, -1, -1 }; // 임시
	if (stratagemIDs == defaultIDs) // state에 세팅이 안 되어있을 경우 기본값 사용
		return;

	StratagemSlots.Empty(); // 되어있을 경우 기본값 삭제
	for (int32 id : stratagemIDs)
	{
		if (id < 0) continue;
		FStratagemSlot stratagemSlot = GI->GetStratagemSlotFromTable(id);
		StratagemSlots.Add(stratagemSlot);
	}
}

void UStratagemComponent::TryUseCurrentStratagem()
{
	if (!StratagemSlots.IsValidIndex(CurrentSlotIndex))
		return;

	FStratagemSlot& slot = StratagemSlots[CurrentSlotIndex];

	if (IsStratagemOnCooldown(CurrentSlotIndex))
		return;

	StratagemSlots[CurrentSlotIndex].LastUsedTime = GetWorld()->GetTimeSeconds();
}

void UStratagemComponent::SelectStratagem(int32 Index)
{
	if (StratagemSlots.IsValidIndex(Index))
	{
		CurrentSlotIndex = Index;
	}
}

TSubclassOf<AStratagem> UStratagemComponent::GetSelectedStratagemClass() const
{
	if (StratagemSlots.IsValidIndex(CurrentSlotIndex))
	{
		return StratagemSlots[CurrentSlotIndex].StratagemClass;
	}
	return nullptr;
}

bool UStratagemComponent::IsStratagemOnCooldown(int32 SlotIndex) const
{
	if (!StratagemSlots.IsValidIndex(SlotIndex))
		return true;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	return (CurrentTime - slot.LastUsedTime) < slot.Cooldown;
}

float UStratagemComponent::GetRemainingCooldown(int32 SlotIndex) const
{
	if (!StratagemSlots.IsValidIndex(SlotIndex))
		return 0.0f;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	float elapsed = CurrentTime - slot.LastUsedTime;
	return FMath::Clamp(slot.Cooldown - elapsed, 0.0f, slot.Cooldown);
}

void UStratagemComponent::CommitStratagemUse()
{
	if (StratagemSlots.IsValidIndex(CurrentSlotIndex))
	{
		StratagemSlots[CurrentSlotIndex].LastUsedTime = GetWorld()->GetTimeSeconds();
	}
}

int32 UStratagemComponent::FindStratagemByInputSequence(const TArray<FKey>& InputSequence) const
{
	for (int32 i = 0; i < StratagemSlots.Num(); ++i)
	{
		TSubclassOf<AStratagem> stratagemClass = StratagemSlots[i].StratagemClass;
		if (!stratagemClass) continue;

		const AStratagem* CDO = stratagemClass->GetDefaultObject<AStratagem>();
		if (!CDO) continue;

		const TArray<FKey>& combo = CDO->GetInputSequence();

		if (combo == InputSequence)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

