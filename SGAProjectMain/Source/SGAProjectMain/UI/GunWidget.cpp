// Fill out your copyright notice in the Description page of Project Settings.


#include "GunWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UGunWidget::SetAmmo(int32 curAmmo, int32 maxAmmo)
{
	if (curAmmo <= 0)
		curAmmo = 0;

	float ratio = static_cast<float>(curAmmo) / static_cast<float>(maxAmmo);
	_curAmmoPB->SetPercent(ratio);
}

void UGunWidget::SetSpare(int32 curSpare, int32 maxSpare)
{
	FString text = FString::Printf(TEXT("%d/%d"), curSpare, maxSpare);
	_xSpare->SetVisibility(ESlateVisibility::Visible);

	if (maxSpare == 0) // 탄창이 사실상 없는 총의 경우 미표시
	{
		text = FString::Printf(TEXT(""));
		_xSpare->SetVisibility(ESlateVisibility::Collapsed);
	}

	_spareCount->SetText(FText::FromString(text));
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

void UGunWidget::SetStimPack(int32 curStimPack, int32 maxStimPack)
{
	FString text = FString::Printf(TEXT("%d/%d"), curStimPack, maxStimPack);
	_stimPackCount->SetText(FText::FromString(text));
}

void UGunWidget::SetGun(UTexture2D* gunIcon)
{
	_gunIcon->SetBrushFromTexture(gunIcon);
}
