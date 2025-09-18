// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployStratagemEntry.h"

#include "Components/Image.h"
#include "../../Object/Stratagem/Stratagem.h"

void UPreDeployStratagemEntry::InitializeEntry(int32 id)
{
	_itemID = id;

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	TSubclassOf<AStratagem> stgClass = GI->GetStratagemClassFromTable(id);
	if (stgClass)
	{
		// 클래스를 이용하여 아이콘 가져오기
		const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();
		UTexture2D* image = stg->GetStgIcon();
		if (image)
			_itemImage->SetBrushFromTexture(image);
	}
}
