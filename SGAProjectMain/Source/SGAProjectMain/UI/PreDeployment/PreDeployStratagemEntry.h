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
	
};
