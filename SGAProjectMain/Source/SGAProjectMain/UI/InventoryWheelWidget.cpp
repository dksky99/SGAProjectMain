// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWheelWidget.h"

#include "../Character/HellDiver/HellDiverInvenComponent.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UInventoryWheelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    _sections = { _sampleSection, _backpackSection, _itemSection, _gunSection };
	_icons = { _sampleIcon, _backpackIcon, _itemIcon, _gunIcon };

    _center = _canvasPanel->GetCachedGeometry().GetLocalSize() * 0.5f;
	_playerController = GetWorld()->GetFirstPlayerController();
    _cursorPos = _center;
}

void UInventoryWheelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

    FVector2D delta;
    _playerController->GetInputMouseDelta(delta.X, delta.Y);

    _cursorPos.X += delta.X * 15.f;
    _cursorPos.Y -= delta.Y * 15.f;

	FVector2D dir = _cursorPos - _center;
	float dis = dir.Size();
    if (dis < _centerRadius) // 커서가 취소 영역 안일 경우
    {
        _curIndex = -1;
    }
	else // 커서가 취소 영역 밖일 경우
    {
        if (dis > _maxRadius) // 최대 반지름 밖일 경우
        {
            dir = dir.GetSafeNormal() * _maxRadius;
            _cursorPos = _center + dir;
        }

        float angleRad = FMath::Atan2(dir.Y, dir.X); // -PI ~ +PI
        float angleDeg = FMath::RadiansToDegrees(angleRad); // -180 ~ +180
        angleDeg = FMath::Fmod(angleDeg + 360.f, 360.f); // 0 ~ 360

        _curIndex = FMath::FloorToInt(angleDeg / 90.0f);
	}

    if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(_cursorImage->Slot))
    {
        slot->SetPosition(_cursorPos); // 사진 크기 절반만큼 이동
    }

    UpdateWheel();
}

void UInventoryWheelWidget::InitializeWheel(UHellDiverInvenComponent* invenComp)
{
    _wheelEntries.SetNum(4);

    if (auto gun = invenComp->GetGunInSlot(2))
    {
        _wheelEntries[3]._isAvailable = true;
        FString gunName = gun->GetGunData()._name.ToString();
        _wheelEntries[3]._label = FString::Printf(TEXT("DROP %s"), *gunName.ToUpper()); // 대문자로 변환
    }

    if (auto backpack = invenComp->GetBackpack())
    {
		_wheelEntries[1]._isAvailable = true;
        FString backpackName = backpack->GetBackpackName().ToString();
		_wheelEntries[1]._label = FString::Printf(TEXT("DROP %s"), *backpackName.ToUpper());
    }

    if (!invenComp->GetSampleBundle().IsEmpty())
    {
        _wheelEntries[0]._isAvailable = true;
		_wheelEntries[0]._label = FString::Printf(TEXT("DROP SAMPLE"));
    }

}

void UInventoryWheelWidget::UpdateWheel()
{
    ResetWheel();

    if (_curIndex == -1 || !_wheelEntries[_curIndex]._isAvailable) // 취소 영역 안이거나 비활성화 섹션일 경우
    {
        _cancelCircle->SetVisibility(ESlateVisibility::Visible);
        _indexText->SetText(FText::FromString(TEXT("CANCEL")));
        return;
	}

    _sections[_curIndex]->SetColorAndOpacity(_highlightSectionColor);
    _icons[_curIndex]->SetColorAndOpacity(_highlightIconColor);

    FString text = _wheelEntries[_curIndex]._label;
    _indexText->SetText(FText::FromString(text));

    //FString text = FString::Printf(TEXT(""));
    //switch (_curIndex)
    //{
    //case 3:
    //    text = FString::Printf(TEXT("DROP GUN"));
    //    break;
    //case 2:
    //    text = FString::Printf(TEXT("DROP ITEM"));
    //    break;
    //case 1:
    //    text = FString::Printf(TEXT("DROP BACKPACK"));
    //    break;
    //case 0:
    //    text = FString::Printf(TEXT("DROP SAMPLE"));
    //    break;
    //default:
    //    text = FString::Printf(TEXT("CANCEL"));
    //    _cancelCircle->SetVisibility(ESlateVisibility::Visible);
    //}


  //  if (_curIndex != -1)
		//HighlightSection(_curIndex);
}

void UInventoryWheelWidget::ResetWheel()
{
    for (int32 i = 0; i < 4; i++)
    {
        if (_wheelEntries[i]._isAvailable)
        {
            _sections[i]->SetColorAndOpacity(_defaultSectionColor);
            _icons[i]->SetColorAndOpacity(_defaultIconColor);
        }
        else
        {
            _sections[i]->SetColorAndOpacity(_defaultSectionColor);
            _icons[i]->SetColorAndOpacity(_disabledIconColor);
        }
    }
    
	_cancelCircle->SetVisibility(ESlateVisibility::Hidden);
}

void UInventoryWheelWidget::HighlightSection(int32 index)
{
	_sections[index]->SetColorAndOpacity(_highlightSectionColor);
	_icons[index]->SetColorAndOpacity(_highlightIconColor);
}
