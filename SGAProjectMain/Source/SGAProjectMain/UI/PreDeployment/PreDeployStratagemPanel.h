// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployPanelBase.h"
#include "PreDeployStratagemPanel.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployStratagemPanel : public UPreDeployPanelBase
{
	GENERATED_BODY()
	
public:
	virtual void HandlePicked(int32 stgID) override;
};
