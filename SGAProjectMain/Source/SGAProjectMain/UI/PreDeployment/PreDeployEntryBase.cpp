// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployEntryBase.h"

#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"

void UPreDeployEntryBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	_button->OnClicked.AddDynamic(this, &UPreDeployEntryBase::HandlePick);
}

void UPreDeployEntryBase::HandlePick()
{
	if (_onPickedEvent.IsBound())
		_onPickedEvent.Broadcast(this); // 자신을 직접 전달
}

void UPreDeployEntryBase::InitializeEntry(int32 id)
{
	_itemID = id;

	UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	UTexture2D* image = GI->GetGunPreviewFromTable(id);
	if (image)
		_itemImage->SetBrushFromTexture(image);
}

void UPreDeployEntryBase::SetEquipped(bool isEquipped)
{
	if (isEquipped)
		_equipMark->SetVisibility(ESlateVisibility::Visible);
	else
		_equipMark->SetVisibility(ESlateVisibility::Hidden);
}

void UPreDeployEntryBase::SetSelected(bool isSelected)
{
	FLinearColor color = isSelected ? FLinearColor::Yellow : FLinearColor::White;
	_border->SetBrushColor(color);
}
