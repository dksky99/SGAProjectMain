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
	/*for (int32 id : stratagemIDs)
	{
		if (id < 0) continue;
		FStratagemSlot stratagemSlot = GI->GetStratagemSlotFromTable(id);
		StratagemSlots.Add(stratagemSlot);
	}*/

	for (int32 id : stratagemIDs)
	{
		if (id < 0)
		{
			continue;
		}

		FStratagemSlot stratagemSlot = GI->GetStratagemSlotFromTable(id);

		// 게임 시작 시점에는 "아직 한 번도 사용한 적 없음" 상태로 초기화
		stratagemSlot.LastUsedTime = -9999.0f;

		// 탄수 시스템을 쓰는 슬롯이면, 로드 시 남은 탄수를 가득 채워줍니다.
		if (stratagemSlot.MaxCharges > 0)
		{
			stratagemSlot.CurrentCharges = stratagemSlot.MaxCharges;
		}

		StratagemSlots.Add(stratagemSlot);
	}
}

void UStratagemComponent::TryUseCurrentStratagem()
{
	/*if (!StratagemSlots.IsValidIndex(CurrentSlotIndex))
		return;

	FStratagemSlot& slot = StratagemSlots[CurrentSlotIndex];

	if (IsStratagemOnCooldown(CurrentSlotIndex))
		return;

	StratagemSlots[CurrentSlotIndex].LastUsedTime = GetWorld()->GetTimeSeconds();*/

	if (!StratagemSlots.IsValidIndex(CurrentSlotIndex))
		return;

	if (IsStratagemOnCooldown(CurrentSlotIndex))
		return;

	// 실제 사용 확정
	CommitStratagemUse();
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
	//if (!StratagemSlots.IsValidIndex(SlotIndex))
	//	return true;

	//float CurrentTime = GetWorld()->GetTimeSeconds();
	//const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	//return (CurrentTime - slot.LastUsedTime) < slot.Cooldown;

	if (!StratagemSlots.IsValidIndex(SlotIndex))
		return true;
	
	const float currentTime = GetWorld()->GetTimeSeconds();
	const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	const float rearmRemain = GetRemainingRearmCooldownForGroup(slot.CooldownGroup);
	if (rearmRemain > 0.0f)
		return true;

	// 탄수 먼저 체크 (MaxCharges > 0이면 탄수 시스템 사용 중)
	if (slot.MaxCharges > 0 && slot.CurrentCharges <= 0)
	{
		// 탄을 전부 썼으면, 재보급 전까지는 항상 사용 불가
		return true;
	}

	// 개별 쿨타임
	if (slot.Cooldown > 0.0f)
	{
		const float elapsed = currentTime - slot.LastUsedTime;
		if (elapsed < slot.Cooldown)
		{
			return true;
		}
	}

	// 그룹 쿨타임 (이글 등 공유 쿨)
	if (!slot.CooldownGroup.IsNone() && slot.GroupCooldown > 0.0f)
	{
		if (const float* lastGroupTime = _groupLastUsedTime.Find(slot.CooldownGroup))
		{
			const float groupElapsed = currentTime - *lastGroupTime;
			if (groupElapsed < slot.GroupCooldown)
			{
				return true;
			}
		}
	}

	return false;
}

float UStratagemComponent::GetRemainingCooldown(int32 SlotIndex) const
{
	//if (!StratagemSlots.IsValidIndex(SlotIndex))
	//	return 0.0f;

	//float CurrentTime = GetWorld()->GetTimeSeconds();
	//const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	//float elapsed = CurrentTime - slot.LastUsedTime;
	//return FMath::Clamp(slot.Cooldown - elapsed, 0.0f, slot.Cooldown);

	if (!StratagemSlots.IsValidIndex(SlotIndex))
		return 0.0f;

	const float currentTime = GetWorld()->GetTimeSeconds();
	const FStratagemSlot& slot = StratagemSlots[SlotIndex];

	// 재무장 쿨이 돌고 있다면 → 이 그룹은 모두 같은 재무장 남은 시간을 표시
	const float rearmRemain = GetRemainingRearmCooldownForGroup(slot.CooldownGroup);
	if (rearmRemain > 0.0f)
		return rearmRemain;

	float remainPersonal = 0.0f;
	if (slot.Cooldown > 0.0f)
	{
		const float elapsed = currentTime - slot.LastUsedTime;
		remainPersonal = FMath::Clamp(slot.Cooldown - elapsed, 0.0f, slot.Cooldown);
	}

	float remainGroup = 0.0f;
	if (!slot.CooldownGroup.IsNone() && slot.GroupCooldown > 0.0f)
	{
		if (const float* lastGroupTime = _groupLastUsedTime.Find(slot.CooldownGroup))
		{
			const float groupElapsed = currentTime - *lastGroupTime;
			remainGroup = FMath::Clamp(slot.GroupCooldown - groupElapsed, 0.0f, slot.GroupCooldown);
		}
	}

	// 실제로는 개인 쿨과 그룹 쿨 중 더 오래 남은 쪽에 막혀 있으므로 Max 사용
	return FMath::Max(remainPersonal, remainGroup);
}

void UStratagemComponent::CommitStratagemUse()
{
	/*if (StratagemSlots.IsValidIndex(CurrentSlotIndex))
	{
		StratagemSlots[CurrentSlotIndex].LastUsedTime = GetWorld()->GetTimeSeconds();
	}*/

	if (!StratagemSlots.IsValidIndex(CurrentSlotIndex))
		return;

	const float currentTime = GetWorld()->GetTimeSeconds();
	FStratagemSlot& slot = StratagemSlots[CurrentSlotIndex];

	// 개별 쿨 시작
	slot.LastUsedTime = currentTime;

	// 탄수 감소 (0이면 탄수 시스템 사용 안 함)
	if (slot.MaxCharges > 0 && slot.CurrentCharges > 0)
	{
		slot.CurrentCharges -= 1;
	}

	// 그룹 쿨 시작 (이글 등 공유 쿨 그룹)
	if (!slot.CooldownGroup.IsNone() && slot.GroupCooldown > 0.0f)
	{
		_groupLastUsedTime.FindOrAdd(slot.CooldownGroup) = currentTime;
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

bool UStratagemComponent::HasUsedAnyEagleAmmo() const
{
	// 이글 그룹 이름은 데이터 테이블에서 CooldownGroup = "Eagle" 로 맞춰둔다고 가정합니다.
	static const FName eagleGroupName(TEXT("Eagle"));

	for (const FStratagemSlot& slot : StratagemSlots)
	{
		// 이 슬롯이 이글 그룹에 속하고, 탄수 시스템을 사용하는 경우만 체크
		if (slot.CooldownGroup == eagleGroupName && slot.MaxCharges > 0)
		{
			// MaxCharges > CurrentCharges 이면 "한 번이라도 발사된 적 있음"
			if (slot.CurrentCharges < slot.MaxCharges)
			{
				return true;
			}
		}
	}

	return false;
}

float UStratagemComponent::GetRemainingRearmCooldownForGroup(FName cooldownGroup) const
{
	if (cooldownGroup.IsNone())
		return 0.0f;

	UWorld* world = GetWorld();
	if (!world)
		return 0.0f;

	const float currentTime = world->GetTimeSeconds();

	// 데이터 테이블에 등록한 이글 재무장 스트라타젬 ID
	static const FName eagleRearmID(TEXT("EagleRearm"));

	for (const FStratagemSlot& slot : StratagemSlots)
	{
		// 이 그룹을 재무장하는 슬롯인지 확인
		if (slot.StratagemID == eagleRearmID && slot.CooldownGroup == cooldownGroup)
		{
			if (slot.Cooldown <= 0.0f)
				return 0.0f;

			const float elapsed = currentTime - slot.LastUsedTime;
			const float remaining = FMath::Clamp(slot.Cooldown - elapsed, 0.0f, slot.Cooldown);
			return remaining;
		}
	}

	return 0.0f;
}

void UStratagemComponent::ApplyEagleRearm()
{
	if (!GetWorld())
		return;

	const float currentTime = GetWorld()->GetTimeSeconds();

	static const FName eagleRearmID(TEXT("EagleRearm"));

	FName targetGroup = NAME_None;
	float rearmCooldown = 0.0f;

	// 이글 재무장 슬롯 찾기 (그룹 이름 + 재무장 시간 가져오기)
	for (FStratagemSlot& slot : StratagemSlots)
	{
		if (slot.StratagemID == eagleRearmID)
		{
			targetGroup = slot.CooldownGroup;   // 예: "Eagle"
			rearmCooldown = slot.Cooldown;      // 예: 120.0f

			// 재무장 슬롯 자기 자신의 쿨 시작 시간 기록
			slot.LastUsedTime = currentTime;
			break;
		}
	}

	if (targetGroup.IsNone() || rearmCooldown <= 0.0f)
	{
		return;
	}

	// 해당 그룹의 이글 계열 탄수 전부 회복
	for (FStratagemSlot& slot : StratagemSlots)
	{
		if (slot.CooldownGroup == targetGroup && slot.MaxCharges > 0)
		{
			slot.CurrentCharges = slot.MaxCharges;
		}
	}
}
