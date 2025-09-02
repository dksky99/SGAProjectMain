// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PreDeploymentState.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UPreDeploymentState : public UObject
{
	GENERATED_BODY()

public:
	void SetGunID(int32 id);

	void SetPrimaryGunID(int32 id) { _primaryGunID = id; }
	int32 GetPrimaryGunID() { return _primaryGunID; }

	void SetSecondaryGunID(int32 id) { _secondaryGunID = id; }
	int32 GetSecondaryGunID() { return _secondaryGunID; }

	void SetSupportGunID(int32 id) { _supportGunID = id; }
	int32 GetSupportGunID() { return _supportGunID; }
	
private:
	// юс╫ц
	int32 _primaryGunID = 1;
	int32 _secondaryGunID = 101;
	int32 _supportGunID = 201;
};