// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployWeaponPanel.h"

#include "PreDeployDetailBase.h"
#include "Components/Border.h"

void UPreDeployWeaponPanel::InitializePanel(UPreDeploymentState* state)
{
    Super::InitializePanel(state);

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

    _sectionPanel->ClearChildren();

    // 그룹화된 총들로 카테고리 섹션 설정
    for (auto& group : groupedGuns)
    {
        UPreDeployCategorySection* section = CreateWidget<UPreDeployCategorySection>(this, _categoryClass);
        section->_onEntrySpawnedEvent.AddUObject(this, &UPreDeployWeaponPanel::OnEntrySpawned); // 섹션의 엔트리 생성 이벤트 바인딩
        FText title = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)group.Key);
        section->InitializeSection(title, group.Value);

        _sectionPanel->AddChild(section); // 메인 패널에 섹션 추가
        _sections.Add(section);
    }

    _equipBtn->OnClicked.AddDynamic(this, &UPreDeployWeaponPanel::HandleEquipRequest);

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

void UPreDeployWeaponPanel::HandleEntryPicked(UPreDeployEntryBase* entry)
{
    Super::HandleEntryPicked(entry);

    UCGameInstance* GI = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
    FGunData gunData = GI->GetGunDataFromTable(entry->GetItemID());
    FText sectionText = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)gunData._category);
    if (_curSectionText)
        _curSectionText->SetText(sectionText); // 현재 섹션 텍스트 업데이트

    if (_detailPanel)
        _detailPanel->SetDetail(entry->GetItemID()); // 상세 패널 업데이트

    FLinearColor color = (entry == _lastEquippedEntry) ? FLinearColor::Green : FLinearColor::Yellow;
    
    if (_equipBorder)
        _equipBorder->SetBrushColor(color);
    if (_equipText)
        _equipText->SetColorAndOpacity(color);
}

void UPreDeployWeaponPanel::HandleEquipRequest()
{
    if (!_curSelectedEntry) return;

    if (_lastEquippedEntry)
        _lastEquippedEntry->SetEquipped(false);
    _curSelectedEntry->SetEquipped(true);

    _lastEquippedEntry = _curSelectedEntry;

    // state에 정보 넘겨주기
    int32 itemID = _curSelectedEntry->GetItemID();
    _state->SetGunID(itemID);

    if (_selectChangedEvent.IsBound())
        _selectChangedEvent.Broadcast(itemID); // 허브에서 처리

    if (_equipBorder)
        _equipBorder->SetBrushColor(FLinearColor::Green);
    if (_equipText)
        _equipText->SetColorAndOpacity(FLinearColor::Green);
}
