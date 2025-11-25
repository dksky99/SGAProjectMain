// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionWidget.h"

#include "MissionSlotWidget.h"
#include "../CGameInstance.h"
#include "../MainGameMode.h"
#include "../Game/PreDeployment/PreDeploymentState.h"
#include "../Data/MissionDataAsset.h"
#include "../Data/ObjectiveDataAsset.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

void UMissionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	_completedText->SetVisibility(ESlateVisibility::Collapsed);
	_missionSlots->ClearChildren();

	auto GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (!GI) return;

	const auto& mission = GI->GetPreDeployState()->GetCurMission();
	if (!mission) return;

	auto GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GM) return;

	// 모든 mission(미션, 혹은 메인 목표)과 objective(목표)를 해당 클래스에서는 편의상 미션으로 통칭
	// 미션 완료 이벤트 바인딩
	GM->_objectiveCompletedEvent.AddUObject(this, &UMissionWidget::SetMissionCompleted); // 선택 목표(optional objective) 완료 이벤트
	GM->_missionCompletedEvent.AddUObject(this, &UMissionWidget::OnMissionCompleted);       // 메인 목표(mission) 완료 이벤트

	// 메인 목표(미션) 슬롯 추가
	AddMissionSlot(mission->GetMissionIcon(), mission->GetMissionName(), mission->GetMissionID());

	// 선택 목표 슬롯 추가
	for (const auto& obj : mission->GetOptionalObjectives())
	{
		AddMissionSlot(obj->GetObjectiveIcon(), obj->GetObjectiveName(), obj->GetObjectiveID());
	}
}

void UMissionWidget::AddMissionSlot(UTexture2D* texture, FText text, FName ID)
{
	auto slot = CreateWidget<UMissionSlotWidget>(this, _slotWidgetClass);
	if (slot)
	{
		slot->InitializeSlot(texture, text, ID);
		_missionSlots->AddChild(slot);
	}
}

void UMissionWidget::SetMissionCompleted(FName ID)
{
	for (auto& slot : _missionSlots->GetAllChildren())
	{
		if (auto missionSlot = Cast<UMissionSlotWidget>(slot))
		{
			if (missionSlot->GetMissionID() == ID)
			{
				missionSlot->DeactivateSlot();
				ShowMissionCompletedText();
				break;
			}
		}
	}
}

void UMissionWidget::ShowMissionCompletedText()
{
	_completedText->SetVisibility(ESlateVisibility::Visible);

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			_completedText->SetVisibility(ESlateVisibility::Collapsed);
		}), 2.0f, false);
}

void UMissionWidget::OnMissionCompleted()
{
	AddMissionSlot(_planeMissionIcon, FText::FromString("Extraction Available"), FName("EscapePlaneMission"));
}
