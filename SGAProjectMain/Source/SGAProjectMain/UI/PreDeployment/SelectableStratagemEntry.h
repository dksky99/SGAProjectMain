// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SelectableEntryBase.h"
#include "SelectableStratagemEntry.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API USelectableStratagemEntry : public USelectableEntryBase
{
	GENERATED_BODY()

public:
	void InitializeEntry(int32 id) override;
	
};
