// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

void UCrosshairWidget::ShowHitMarker(EHitOutcome hitOutcome)
{
	UTexture2D* icon = _fullDmgIcon;
	FVector2D offset = FVector2D::ZeroVector;

	if (hitOutcome == EHitOutcome::Penetrate)
		icon = _weakDmgIcon;

	if (hitOutcome == EHitOutcome::Ricochet)
	{
		icon = _ricochetIcon;
		offset = _ricochetOffset;
	}

	if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(_hitMarker->Slot))
		slot->SetPosition(offset);
	_hitMarker->SetBrushFromTexture(icon);
	_hitMarker->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().ClearTimer(_hitMarkerTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(_hitMarkerTimerHandle, [this]()
		{
			_hitMarker->SetVisibility(ESlateVisibility::Hidden);
		}, 0.25f, false);

}