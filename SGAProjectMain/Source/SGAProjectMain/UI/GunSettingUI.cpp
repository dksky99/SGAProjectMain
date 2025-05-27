// Fill out your copyright notice in the Description page of Project Settings.

#include "GunSettingUI.h"

#include "../Gun/GunBase.h"
#include "Components/TextBlock.h"

void UGunSettingUI::UpdateGunInfo(const FGunData& gunData, int32 curAmmo)
{
	FString gunName = gunData._name.ToString();
	FString text = FString::Printf(TEXT("%s"), *gunName);
	_gunNameText->SetText(FText::FromString(text));

	text = FString::Printf(TEXT("%d / %d"), curAmmo, gunData._maxAmmo);
	_curAmmoText->SetText(FText::FromString(text));
}
