// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreDeployDetailBase.h"
#include "PreDeployStratagemDetail.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeployStratagemDetail : public UPreDeployDetailBase
{
	GENERATED_BODY()
	
public:
	virtual void SetDetail(int32 id) override;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* _cooldownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* _callInTimeText;
};
