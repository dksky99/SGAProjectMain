// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployStratagemDetail.h"

#include "../../Object/Stratagem/Stratagem.h"
#include "../../StratagemComponent.h"

void UPreDeployStratagemDetail::SetDetail(int32 id)
{
	Super::SetDetail(id);

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	FStratagemSlot slot = GI->GetStratagemSlotFromTable(id);
	TSubclassOf<class AStratagem> stgClass = slot.StratagemClass;
	if (!stgClass) return;

	const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();

	_nameText->SetText(FText::FromName(stg->GetStgName()));

	FString sectionText = StaticEnum<EStratagemType>()->GetNameStringByValue((int64)stg->GetStratagemType());
	sectionText += TEXT(" Stratagem Permit");
	_sectionText->SetText(FText::FromString(sectionText));

	_descText->SetText(slot.Description);

	_cooldownText->SetText(FText::AsNumber(slot.Cooldown));
	_callInTimeText->SetText(FText::AsNumber(stg->GetDeployDelay()));
}
