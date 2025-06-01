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
	FString text = FString::Printf(TEXT("x%d/%d"), curMag, maxMag);
	_magCount->SetText(FText::FromString(text));
	UE_LOG(LogTemp, Log, TEXT("SetMag"));
}
