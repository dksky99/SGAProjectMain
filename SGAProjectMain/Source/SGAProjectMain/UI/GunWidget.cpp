// Fill out your copyright notice in the Description page of Project Settings.


#include "GunWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UGunWidget::SetAmmo(int32 curAmmo, int32 maxAmmo)
{
	if (curAmmo <= 0)
		curAmmo = 0;

	float ratio = static_cast<float>(curAmmo) / static_cast<float>(maxAmmo);
	_curAmmoPB->SetPercent(ratio);
}

void UGunWidget::SetMag(int32 curMag, int32 maxMag)
{
	FString text = FString::Printf(TEXT("%d/%d"), curMag, maxMag);
	_magCount->SetText(FText::FromString(text));
	UE_LOG(LogTemp, Log, TEXT("SetMag"));
}

void UGunWidget::SetHp(float ratio)
{
	_curHpPB->SetPercent(ratio);
}

void UGunWidget::SetGrenade(int32 curGrenade, int32 maxGrenade)
{
	FString text = FString::Printf(TEXT("%d/%d"), curGrenade, maxGrenade);
	_grenadeCount->SetText(FText::FromString(text));
}

void UGunWidget::SetInjector(int32 curInjector, int32 maxInjector)
{
	FString text = FString::Printf(TEXT("%d/%d"), curInjector, maxInjector);
	_injectorCount->SetText(FText::FromString(text));
}
