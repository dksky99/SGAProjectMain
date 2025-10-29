// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployWeaponDetail.h"

#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "../../Data/GunDataTable.h"

void UPreDeployWeaponDetail::SetDetail(int32 id)
{
	Super::SetDetail(id);

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	FGunData gunData = GI->GetGunDataFromTable(id);

	_equipImage->SetBrushFromTexture(GI->GetGunPreviewFromTable(id));

	_nameText->SetText(gunData._name);

	FText text = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)gunData._category);
	_sectionText->SetText(text);

	_descText->SetText(gunData._desc);


	_damageText->SetText(FText::AsNumber((int32)gunData._baseDamage));
	_damageBar->SetPercent(GetStatPercent(gunData._baseDamage, 100, true));

	_maxAmmoText->SetText(FText::AsNumber(gunData._maxAmmo));
	_maxAmmoBar->SetPercent(GetStatPercent(gunData._maxAmmo, 10));

	_recoilText->SetText(FText::AsNumber((int32)gunData._recoil));
	_recoilBar->SetPercent(GetStatPercent(gunData._recoil, 10));

	int32 fireRate = (int32)(60.f / gunData._fireInterval);
	_fireRateText->SetText(FText::AsNumber(fireRate));
	_fireRateBar->SetPercent(GetStatPercent(fireRate, 150));


	FString traitString;
	switch (gunData._penetrateTrait)
	{
	case EPenetrateTrait::Light:
		traitString = TEXT("Light Armor Penetrating");
		break;
	case EPenetrateTrait::Medium:
		traitString = TEXT("Medium Armor Penetrating");
		break;
	case EPenetrateTrait::Heavy:
		traitString = TEXT("Heavy Armor Penetrating");
		break;
	case EPenetrateTrait::AntiTank:
		traitString = TEXT("Anti-Tank Armor Penetrating");
		break;
	default:
		traitString = TEXT("");
		break;
	}
	_traitText->SetText(FText::FromString(traitString));
}

float UPreDeployWeaponDetail::GetStatPercent(float stat, int32 tierSize, bool addOne)
{
	// 헬다이버즈 기준

	if (tierSize <= 0)
		return 0.f;

	int32 tier = (int32)stat / tierSize;

	if (addOne)
	{
		tier += 1;
	}

	float tierPercent = tier * 0.1f; // 티어 하나당 10%
	return FMath::Clamp(tierPercent, 0.f, 1.f);
}
