// Fill out your copyright notice in the Description page of Project Settings.


#include "GunWidget.h"

#include "Components/ProgressBar.h"

void UGunWidget::SetAmmo(int32 curAmmo, int32 maxAmmo)
{
	if (curAmmo <= 0)
		curAmmo = 0;

	float ratio = static_cast<float>(curAmmo) / static_cast<float>(maxAmmo);
	_curAmmoPB->SetPercent(ratio);
}
