// Fill out your copyright notice in the Description page of Project Settings.


#include "CompassWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"

void UCompassWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (_compassImage)
    {
        UMaterialInterface* mat = Cast<UMaterialInterface>(_compassImage->Brush.GetResourceObject());
        if (mat)
        {
            _compassMat = UMaterialInstanceDynamic::Create(mat, this);
            _compassImage->SetBrushFromMaterial(_compassMat);
        }

        _directionMarks = {
            { 0.f,   _northText },
            { 90.f,  _eastText  },
            { 180.f, _southText },
            { 270.f, _westText  }
        };
    }
}

void UCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!_pc)
    {
        _pc = GetOwningPlayer(); // 해당 위젯의 소유자 컨트롤러 찾기
    }

    FVector viewLoc;
    FRotator viewRot;
    _pc->GetPlayerViewPoint(viewLoc, viewRot);

    // 보고있는 방향 각도 구하기
    float curYaw = FMath::Fmod(viewRot.Yaw + 360.f, 360.f); // 360을 더하여 음수값 방지 -> 0 ~ 360 사이 값

    // 보고있는 각도 표시
    FString text = FString::Printf(TEXT("%.0f"), curYaw);
    _angleText->SetText(FText::FromString(text));

    // 눈금에 방향 표시
    for (const auto& dirMark : _directionMarks)
    {
        float delta = FMath::FindDeltaAngleDegrees(curYaw, dirMark.angle); // 현재 yaw 값과 기준값 사이 최소 회전 차이
        if (FMath::Abs(delta) < 91.f) // 두 값의 차이가 91도 이하일 경우
        {
            dirMark.textBlock->SetVisibility(ESlateVisibility::Visible); // 위젯에 표시
            UpdateWidgetPos(dirMark.textBlock, delta);
        }
        else
        {
            dirMark.textBlock->SetVisibility(ESlateVisibility::Hidden); // 컴파스 범위에서 벗어났다면 안 보이게
        }
    }

    // 머티리얼 오프셋 조정 -> 눈금 사진 움직임
    if (_compassMat)
    {
        _compassMat->SetScalarParameterValue("CompassOffset", curYaw / 360.f); // 오프셋은 0~1
    }

    // 핑이 찍혔을 경우 핑 표시
    if (_isPingActive)
    {
        if (!_pc->GetPawn()) return;

        FVector playerLocation = _pc->GetPawn()->GetActorLocation();
        FVector dirToTarget = (_pingLocation - playerLocation).GetSafeNormal();
        FRotator dirRot = dirToTarget.Rotation();

        float targetYaw = dirRot.Yaw;

        // 두 각도의 최소 회전차 (-180 ~ +180)
        float delta = FMath::FindDeltaAngleDegrees(curYaw, targetYaw);
        
        if (FMath::Abs(delta) < 90.f) // 두 값의 차이가 91도 이하일 경우
        {
            _pingImage->SetRenderOpacity(1.f); // 완전히 불투명하게
            UpdateWidgetPos(_pingImage, delta);
        }
        else
        {
            _pingImage->SetRenderOpacity(0.3f); // 범위에서 벗어났을 경우 반투명하게
            if (delta < -90.f)
                UpdateWidgetPos(_pingImage, -90.f); // -90 ~ -180까지는 왼쪽 끝부분에 표시
            else                                    // 2.f 차이는 다른 글씨나 눈금을 가리지 않기 위한 여유 수치
                UpdateWidgetPos(_pingImage, 90.f); // 90~ 180까지는 오른쪽 끝부분에 표시
        }
    }
}

void UCompassWidget::UpdateWidgetPos(UWidget* widget, float delta)
{
    // 위치 세팅
    //FVector2D compassSize = _compassImage->GetCachedGeometry().GetLocalSize(); // 눈금 이미지 크기

    if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(widget->Slot))
    {
        FVector2D textPos = slot->GetPosition();
        textPos.X = 510.f / 180.f * delta;   // 컴파스는 180도 표시 -> 1도 당 size/180.f
        slot->SetPosition(textPos);
    }
}

void UCompassWidget::ShowPingImage(bool needToShow)
{
    if (needToShow)
    {
        _pingImage->SetVisibility(ESlateVisibility::Visible);
        _isPingActive = true;
    }
    else
    {
        _pingImage->SetVisibility(ESlateVisibility::Hidden);
        _isPingActive = false;
    }
}