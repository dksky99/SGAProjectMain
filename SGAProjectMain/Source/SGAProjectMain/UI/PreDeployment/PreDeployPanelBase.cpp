// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployPanelBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "../../Gun/GunDataTable.h"
#include "PreDeployCategorySection.h"
#include "PreDeployDetailBase.h"

void UPreDeployPanelBase::InitializePanel(UPreDeploymentState* state)
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    UDataTable* gunTable = GI->GetGunTable();

    // 무기 타입별로 총을 그룹화 (카테고리 & 총의 id)
    TMap<EGunCategory, TArray<int32>> groupedGuns;
    for (auto& row : gunTable->GetRowMap()) // 모든 총 데이터를 불러오기
    {
        FGunData* gunData = (FGunData*)row.Value;
		if (gunData->_slotType == _panelSlotType) // 특정 슬롯 타입에 해당하는 총만
        {
            int32 id = FCString::Atoi(*row.Key.ToString());
            groupedGuns.FindOrAdd(gunData->_category).Add(id); // 카테고리별로 배열에 추가
        }
    }

    // 그룹화된 총들로 카테고리 섹션 설정
    for (auto& group : groupedGuns)
    {
        UPreDeployCategorySection* section = CreateWidget<UPreDeployCategorySection>(this, _categoryClass);
		section->_onEntrySpawnedEvent.AddUObject(this, &UPreDeployPanelBase::OnEntrySpawned); // 섹션의 엔트리 생성 이벤트 바인딩
        FText title = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)group.Key);
		section->InitializeSection(title, group.Value);
		//section->_onSectionPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandlePicked); // 섹션의 선택 이벤트 바인딩
        //section->SetTitleText(title); // 섹션 이름 설정

        //for (const int32 id : group.Value) // 각 그룹 내의 총들에 대해
        //{
        //    USelectableEntryBase* entry = CreateWidget<USelectableEntryBase>(this, _entryClass); // 엔트리 위젯 생성
        //    entry->InitializeEntry(id); // 총 ID로 초기화
        //    entry->_onPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandlePicked); // 선택 이벤트 바인딩
        //    section->_panel->AddChild(entry); // 섹션의 패널에 추가
        //    _entries.Add(entry); // 내부 목록에도 추가
        //}

        _sectionPanel->AddChild(section); // 메인 패널에 섹션 추가
    }

    _state = state;

	_equipBtn->OnClicked.AddDynamic(this, &UPreDeployPanelBase::HandleEquipRequest);

	int32 gunID = -1;
    if (_panelSlotType == EGunSlotType::Primary)
		gunID = state->GetPrimaryGunID();
	else if (_panelSlotType == EGunSlotType::Secondary)
		gunID = state->GetSecondaryGunID();

    _detailPanel->SetDetail(gunID);
    FGunData gunData = GI->GetGunDataFromTable(gunID);
    FText sectionText = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)gunData._category);
    _curSectionText->SetText(sectionText);
}

 //   const int32 Count = _panel->GetChildrenCount();
 //   for (int32 i = 0; i < Count; ++i) {
 //       if (USelectableEntryBase* entry = Cast<USelectableEntryBase>(_panel->GetChildAt(i)))
 //       {
 //           _entries.Add(entry);
 //           entry->_onPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandlePicked);
 //           entry->InitializeEntry(entry->GetItemID()); // 임시
 //       }
 //   }


void UPreDeployPanelBase::HandleEntryPicked(UPreDeployEntryBase* entry)
{
    if (_curSelectedEntry)
        _curSelectedEntry->SetSelected(false); // 이전 선택 해제

	entry->SetSelected(true); // 새로 선택된 엔트리 강조
    _curSelectedEntry = entry;   

    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	FGunData gunData = GI->GetGunDataFromTable(entry->GetItemID());
    FText sectionText = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)gunData._category);
    if (_curSectionText)
		_curSectionText->SetText(sectionText); // 현재 섹션 텍스트 업데이트

    if (_detailPanel)
		_detailPanel->SetDetail(entry->GetItemID()); // 상세 패널 업데이트
}

void UPreDeployPanelBase::OnEntrySpawned(UPreDeployEntryBase* entry)
{
	entry->_onPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandleEntryPicked);
}

void UPreDeployPanelBase::HandleEquipRequest()
{
	if (!_curSelectedEntry) return;

    // state에 정보 넘겨주기
    int32 itemID = _curSelectedEntry->GetItemID();
    _state->SetGunID(itemID);

    if (_selectChangedEvent.IsBound())
        _selectChangedEvent.Broadcast(itemID); // 패널에서 처리
}
