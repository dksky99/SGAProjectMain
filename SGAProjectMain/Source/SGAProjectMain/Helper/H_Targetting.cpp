// Fill out your copyright notice in the Description page of Project Settings.


#include "H_Targetting.h"
#include "../Character/CharacterBase.h"
#include "Algo/MaxElement.h"
#include "Algo/MinElement.h"
#include "Algo/Sort.h"

AActor* H_Targetting::Targetting(ETargettingType targettype, ETargettingType defaultTargetType, TArray<AActor*>& arr, const AActor* existing, AActor* curPawn)
{
	AActor* target=nullptr;
	switch (targettype)
	{
	case ETargettingType::MostNear:
		target=MostNearByTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostFar:
		target = MostFarByTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostLowHP:
		//target = MostLowHpTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostHighHP:
		//target = MostHighHpTarget(arr, existing, curPawn);
		break;
	case ETargettingType::Revenge:
		//target = RevengeTarget(arr, existing, curPawn);
		break;
	default:
		target = MostNearByTarget(arr, existing, curPawn);
		break;
	}
	if (target)
		return target;
	switch (defaultTargetType)
	{
	case ETargettingType::MostNear:
		target = MostNearByTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostFar:
		target = MostFarByTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostLowHP:
		//target = MostLowHpTarget(arr, existing, curPawn);
		break;
	case ETargettingType::MostHighHP:
		//target = MostHighHpTarget(arr, existing, curPawn);
		break;
	case ETargettingType::Revenge:
		//target = RevengeTarget(arr, existing, curPawn);
		break;
	default:
		target = MostNearByTarget(arr, existing, curPawn);
		break;
	}


	return target;
}

AActor* H_Targetting::MostNearByTarget(TArray<AActor*>& arr, const AActor* existing,  AActor* curPawn)
{
	if (arr.IsEmpty())
		return nullptr;

	auto temp=Algo::MinElementBy(arr, [curPawn](const AActor* A) {
		return curPawn->GetDistanceTo(A);
		});

	if(temp)
		return *temp;
	return nullptr;
}

AActor* H_Targetting::MostFarByTarget(TArray<AActor*>& arr, const AActor* existing, AActor* curPawn)
{
	if (arr.IsEmpty())
		return nullptr;

	auto temp = Algo::MaxElementBy(arr, [curPawn](const AActor* A) {
		return curPawn->GetDistanceTo(A);
		});

	if (temp)
		return *temp;
	return nullptr;
}

/*
AActor* H_Targetting::MostLowHpTarget(TArray<AActor*>& arr, const AActor* existing,  AActor* curPawn)
{
	if (arr.IsEmpty())
		return nullptr;
	auto temp=Algo::MinElementBy(arr, [](AActor* a) {
		auto c1 = Cast<ACharacterBase>(a);
		

		return c1->GetCurHP();

		});

	if(temp)
		return *temp;
	
	return nullptr;
}

AActor* H_Targetting::MostHighHpTarget(TArray<AActor*>& arr, const AActor* existing, AActor* curPawn)
{
	if (arr.IsEmpty())
		return nullptr;
	auto temp = Algo::MaxElementBy(arr, [](AActor* a) {
		auto c1 = Cast<ACharacterBase>(a);


		return c1->GetCurHP();

		});

	if (temp)
		return *temp;

	return nullptr;
}
AActor* H_Targetting::RevengeTarget(TArray<AActor*>& arr, const AActor* existing, AActor* curPawn)
{
	auto owner = Cast<ACharacterBase>(curPawn);
	if (owner == nullptr)
		return nullptr;

	auto list= (owner->GetLogComponent()->GetValidList());


	if (list.IsEmpty())
		return nullptr;
	

	return list[0].Key;
}
*/