// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionDataAsset.h"
#include "ObjectiveDataAsset.h"

bool UMissionDataAsset::IsOptionalObjectiveIDValid(FName objectiveID)
{
	for (UObjectiveDataAsset* optionalObjective : _optionalObjectives)
	{
		if (optionalObjective && optionalObjective->GetObjectiveID() == objectiveID)
		{
			return true;
		}
	}

	return false;
}

bool UMissionDataAsset::HasOptionalObjectives()
{
	if (_optionalObjectives.Num() > 0)
		return true;

	return false;
}

FName UMissionDataAsset::GetMainObjectiveID()
{
	return _mainObjective ? _mainObjective->GetObjectiveID() : FName();
}
