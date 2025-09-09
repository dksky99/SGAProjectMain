// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployStratagemPanel.h"

#include "../../Object/Stratagem/Stratagem.h"
#include "../../StratagemComponent.h"

void UPreDeployStratagemPanel::InitializePanel(UPreDeploymentState* state)
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    UDataTable* stgTable = GI->GetStratagemTable();

    // 무기 타입별로 스트라타젬을 그룹화 (카테고리 & id)
    TMap<EStratagemType, TArray<int32>> groupedStgs;
    for (auto& row : stgTable->GetRowMap()) // 모든 스트라타젬 데이터를 불러오기
    {
        FStratagemSlot* stgSlot = (FStratagemSlot*)row.Value;
        TSubclassOf<AStratagem> stgClass = stgSlot->StratagemClass;
        const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();

        int32 id = FCString::Atoi(*row.Key.ToString());
		groupedStgs.FindOrAdd(stg->GetStratagemType()).Add(id); // 카테고리별로 배열에 추가
    }

    // 그룹화된 총들로 카테고리 섹션 설정
    for (auto& group : groupedStgs)
    {
        UPreDeployCategorySection* section = CreateWidget<UPreDeployCategorySection>(this, _categoryClass);
        FString type = StaticEnum<EStratagemType>()->GetNameStringByValue((int64)group.Key);
		FText title = FText::FromString(type);
        section->InitializeSection(title, group.Value);
        section->_onSectionPickedEvent.AddUObject(this, &UPreDeployStratagemPanel::HandlePicked); // 섹션의 선택 이벤트 바인딩

        _sectionPanel->AddChild(section); // 메인 패널에 섹션 추가
    }

    _state = state;
}

void UPreDeployStratagemPanel::HandlePicked(int32 stgID, UPreDeployEntryBase* entry)
{
    _state->SetStratagemID(stgID);

    entry->SetSelected(true);

    /*for (auto* entry : _entries)
    {
        if (entry->GetItemID() == stgID)
            entry->SetSelected(true);
    }*/
}
