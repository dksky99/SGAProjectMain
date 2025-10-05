// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployStratagemPanel.h"

#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "../../Object/Stratagem/Stratagem.h"
#include "../../StratagemComponent.h"
#include "PreDeployStratagemDetail.h"

void UPreDeployStratagemPanel::InitializePanel(UPreDeploymentState* state)
{
    Super::InitializePanel(state);

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
		groupedStgs.FindOrAdd(stg->GetStgType()).Add(id); // 카테고리별로 배열에 추가
    }

    int32 sectionIndex = 0;

    // 그룹화된 총들로 카테고리 섹션 설정
    for (auto& group : groupedStgs)
    {
        UPreDeployCategorySection* section = CreateWidget<UPreDeployCategorySection>(this, _categoryClass);
        section->_onEntrySpawnedEvent.AddUObject(this, &UPreDeployStratagemPanel::OnEntrySpawned); // 섹션의 엔트리 생성 이벤트 바인딩
        FString type = StaticEnum<EStratagemType>()->GetNameStringByValue((int64)group.Key);
		FText title = FText::FromString(type);
        section->InitializeSection(title, group.Value, sectionIndex);
        //section->_onSectionPickedEvent.AddUObject(this, &UPreDeployStratagemPanel::HandlePicked); // 섹션의 선택 이벤트 바인딩

        _sectionPanel->AddChild(section); // 메인 패널에 섹션 추가
        _sections.Add(section);

        sectionIndex++;
    }

    // 스트라타젬 슬롯 설정
	TArray<UWidget*> children = _slotPanel->GetAllChildren();
    int32 i = 0;
    for (auto* child : children)
    {
        if (UPreDeployEntryBase* slot = Cast<UPreDeployEntryBase>(child)) // entry를 슬롯으로 재사용
        {
            slot->_onPickedEvent.AddLambda([this, i](UPreDeployEntryBase* slot) {
                HandleSlotPicked(slot, i);     // 슬롯 선택 이벤트 바인딩
                });

            i++;
        }
    }

    HideWidget(true);
}

void UPreDeployStratagemPanel::HandleEntrySelected(UPreDeployEntryBase* entry)
{
    Super::HandleEntrySelected(entry);

	TArray<int32> stgIDs = _state->GetStratagemIDs();
    int32 stgID = entry->GetItemID();

    if (stgIDs.Contains(stgID))
		return; // 이미 선택된 스트라타젬이면 무시

    entry->SetEquipped(true);

    if (_curSlotIndex == -1)
		_curSlotIndex = FindEmptySlotIndex();

    int32 prevID = stgIDs[_curSlotIndex]; // 이전에 있었던 스트라타젬의 아이디
    for (auto selectedEntry : _equippedStgEntries)
    {
        if (prevID != -1 && selectedEntry->GetItemID() == prevID) // 현재 슬롯에 이미 스트라타젬이 선택되어 있으면
        {
            selectedEntry->SetEquipped(false);
            _equippedStgEntries.Remove(selectedEntry); // 선택된 엔트리 목록에서 제거
        }
        else
            selectedEntry->SetEquipped(true);
    }

    _state->SetStratagemID(_curSlotIndex, stgID);

    entry->SetSelected(true);
	_equippedStgEntries.Add(entry); // 선택된 엔트리 목록에 추가

    auto slot = Cast<UPreDeployEntryBase>(_slotPanel->GetChildAt(_curSlotIndex));
    slot->InitializeEntry(stgID); // 선택된 슬롯에 스트라타젬 정보 설정

    HideWidget(false);

	_detailPanel->SetDetail(stgID); // 상세 패널에 정보 설정

	int32 nextIndex = FindEmptySlotIndex();
    if (nextIndex != INDEX_NONE)
		SelectSlot(nextIndex); // 다음 빈 슬롯 선택
    else
	    ClosePanel(); // 모든 슬롯이 채워져 있으면 패널 닫기
}

void UPreDeployStratagemPanel::HandleSlotPicked(UPreDeployEntryBase* slot, int32 slotIndex)
{
	SelectSlot(slotIndex); // 슬롯 선택 처리
}

FText UPreDeployStratagemPanel::GetSectionText(UPreDeployEntryBase* entry)
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    auto stgClass = GI->GetStratagemClassFromTable(entry->GetItemID());
    const AStratagem* stg = stgClass->GetDefaultObject<AStratagem>();

    FText sectionText = StaticEnum<EStratagemType>()->GetDisplayNameTextByValue((int64)stg->GetStgType());
    return sectionText;
}

void UPreDeployStratagemPanel::SelectSlot(int32 slotIndex)
{
	if (_curSlotIndex >= 0 && _curSlotIndex < 4) // 이전에 선택된 슬롯이 있으면
    {
        auto prevSlot = Cast<UPreDeployEntryBase>(_slotPanel->GetChildAt(_curSlotIndex));
        prevSlot->SetSelected(false); // 이전에 선택된 슬롯 선택 해제
    }

    _curSlotIndex = slotIndex;     // 현재 선택된 슬롯 인덱스 갱신

    auto curSlot = Cast<UPreDeployEntryBase>(_slotPanel->GetChildAt(_curSlotIndex));
	curSlot->SetSelected(true); // 현재 슬롯 선택 표시

	OpenPanel(); // 슬롯 선택 시 패널 열기
}

void UPreDeployStratagemPanel::HideWidget(bool isHidden)
{
    if (isHidden)
    {
        _sectionPanel->SetVisibility(ESlateVisibility::Hidden); // 섹션 패널 숨김
        _detailPanel->SetVisibility(ESlateVisibility::Hidden); // 상세 패널 숨김
        _backgroundBorder->SetVisibility(ESlateVisibility::Hidden);
        _curSectionText->SetVisibility(ESlateVisibility::Hidden);
        _ZKey->SetVisibility(ESlateVisibility::Hidden);
        _CKey->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        _sectionPanel->SetVisibility(ESlateVisibility::Visible); // 섹션 패널 표시
        _detailPanel->SetVisibility(ESlateVisibility::Visible); // 상세 패널 표시
        _backgroundBorder->SetVisibility(ESlateVisibility::Visible);
        _curSectionText->SetVisibility(ESlateVisibility::Visible);
        _ZKey->SetVisibility(ESlateVisibility::Visible);
        _CKey->SetVisibility(ESlateVisibility::Visible);
    }
    
}

int32 UPreDeployStratagemPanel::FindEmptySlotIndex()
{
    TArray<int32> stgIDs = _state->GetStratagemIDs();
    for (int32 i = 0; i < stgIDs.Num(); ++i)
    {
        if (stgIDs[i] == -1) // 빈 슬롯 찾기
            return i;
	}

    return INDEX_NONE;
}

void UPreDeployStratagemPanel::OpenPanel()
{
    if (_isPanelOpen) 
        return;

    if (_curSlotIndex == -1) 
        return;

    _isPanelOpen = true;

    if (_slotPanelUp)
		PlayAnimation(_slotPanelUp, 0.f, 1, EUMGSequencePlayMode::Forward, 2.f); // 슬롯 패널 애니메이션 재생

    HideWidget(false);

    UPreDeployEntryBase* entrySelected = nullptr;
    int32 stgID = _state->GetStratagemIDs()[_curSlotIndex];
    if (stgID == -1) // 슬롯에 스트라타젬이 장착되어 있지 않으면
    {
        entrySelected = _sections[0]->GetEntries()[0];
        stgID = entrySelected->GetItemID(); // 
    }
    else
    {
        for (auto entry : _equippedStgEntries)
        {
            if (entry->GetItemID() == stgID)
            {
                entrySelected = entry;
                break;
            }
        }
    }

    if (entrySelected)
        Super::HandleEntrySelected(entrySelected);

	if (_panelOpenedEvent.IsBound())
	    _panelOpenedEvent.Broadcast(true); // 허브 위젯에서 처리
}

void UPreDeployStratagemPanel::ClosePanel()
{
    if (!_isPanelOpen)
        return;

    _isPanelOpen = false;

    if (_slotPanelUp)
        PlayAnimation(_slotPanelUp, 0.f, 1, EUMGSequencePlayMode::Reverse, 2.f); // 슬롯 패널 애니메이션 역재생
	
    HideWidget(true);

	if (_panelOpenedEvent.IsBound())
		_panelOpenedEvent.Broadcast(false); // 허브 위젯에서 처리
}
