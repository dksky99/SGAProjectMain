// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployEntryBase.h"
#include "PreDeployStratagemEntry.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployStratagemEntry : public UPreDeployEntryBase
{
	GENERATED_BODY()

public:
	void InitializeEntry(int32 id) override;

	// 스트라타젬 슬롯용 (몇 번째 슬롯인지)
	void SetSlotIndex(int32 index) { _slotIndex = index; }
	int32 GetSlotIndex() { return _slotIndex; }

protected:
	int32 _slotIndex = -1;
};
