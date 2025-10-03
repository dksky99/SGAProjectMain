// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployPanelBase.h"
#include "Blueprint/WidgetBlueprintLibrary.h"  
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
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

    _sectionPanel->ClearChildren();

    // 그룹화된 총들로 카테고리 섹션 설정
    for (auto& group : groupedGuns)
    {
        UPreDeployCategorySection* section = CreateWidget<UPreDeployCategorySection>(this, _categoryClass);
		section->_onEntrySpawnedEvent.AddUObject(this, &UPreDeployPanelBase::OnEntrySpawned); // 섹션의 엔트리 생성 이벤트 바인딩
        FText title = StaticEnum<EGunCategory>()->GetDisplayNameTextByValue((int64)group.Key);
		section->InitializeSection(title, group.Value);

        _sectionPanel->AddChild(section); // 메인 패널에 섹션 추가
        _sections.Add(section);
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

void UPreDeployPanelBase::MoveLeft()
{
    int32 colNum = GetColumnNumInRow(_curSectionIndex, _curRow);
    if (colNum <= 1) return;

    int32 newCol = (_curCol - 1 + colNum) % colNum;
    SelectEntry(_curSectionIndex, _curRow, newCol);
}

void UPreDeployPanelBase::MoveRight()
{
    int32 colNum = GetColumnNumInRow(_curSectionIndex, _curRow); // 해당 행에 몇 열까지 존재하는지
    if (colNum <= 1) return;

    int32 newCol = (_curCol + 1) % colNum;
    SelectEntry(_curSectionIndex, _curRow, newCol);
}

void UPreDeployPanelBase::MoveUp()
{
    if (_curRow > 0) // 같은 섹션 내에서 위쪽 행으로
    {
        int32 newRow = _curRow - 1;
        int32 colNum = GetColumnNumInRow(_curSectionIndex, newRow);
        if (colNum <= 0) return;

        int32 newCol = FMath::Min(_curCol, colNum - 1);
        SelectEntry(_curSectionIndex, newRow, newCol);
        return;
    }

    // 첫 행이었을 경우 이전 섹션의 마지막 행으로
    int32 newSecIndex = _curSectionIndex - 1;
    if (newSecIndex < 0)
        newSecIndex = _sections.Num() - 1;

    int32 lastRow = GetRowNumInSection(newSecIndex) - 1;
    int32 colNum = GetColumnNumInRow(newSecIndex, lastRow);
    if (colNum <= 0) return;

    int32 newCol = FMath::Min(_curCol, colNum - 1);
    SelectEntry(newSecIndex, lastRow, newCol);
}

void UPreDeployPanelBase::MoveDown()
{
    int32 lastRow = GetRowNumInSection(_curSectionIndex) - 1;
    if (_curRow + 1 <= lastRow) // 같은 섹션 내에서 아래쪽 행으로
    {
        int32 newRow = _curRow + 1;
        int32 colNum = GetColumnNumInRow(_curSectionIndex, newRow);
        if (colNum <= 0) return;

        int32 newCol = FMath::Min(_curCol, colNum - 1);
        SelectEntry(_curSectionIndex, newRow, newCol);
        return;
    }

    // 마지막 행이었을 경우 다음 섹션의 첫 행으로
    int32 newSecIndex = _curSectionIndex + 1;
    if (newSecIndex >= _sections.Num())
        newSecIndex = 0;

    int32 newRow = 0;
    int32 colNum = GetColumnNumInRow(newSecIndex, newRow);
    if (colNum <= 0) return;

    int32 newCol = FMath::Min(_curCol, colNum - 1);
    SelectEntry(newSecIndex, newRow, newCol);
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

int32 UPreDeployPanelBase::GetColumnNumInRow(int32 sec, int32 row)
{
    auto curSection = _sections[sec];
    int32 entryNum = curSection->GetEntryNumPerRow();
    int32 lastEntryIndex = curSection->GetEntries().Num() - 1;

    if (!curSection || entryNum <= 0 || lastEntryIndex < 0)
        return 0;

    // 쉽게 생각하기 위한 예시
    // 0 1 2 -> row 0
    // 3 4 5
    // 6 7

    if (row < lastEntryIndex / entryNum && row >= 0) // 마지막 행 직전 행까지는
        return entryNum; // 해당 row는 꽉 차있음
    else if (row == lastEntryIndex / entryNum) // 마지막 행의 경우
        return lastEntryIndex % entryNum + 1;
    else // 존재하지 않는 행일 경우
        return 0;
}

int32 UPreDeployPanelBase::GetRowNumInSection(int32 sec)
{
    auto curSection = _sections[sec];
    int32 entryNum = curSection->GetEntryNumPerRow();

    return (curSection->GetEntries().Num() + entryNum - 1) / entryNum;
}

void UPreDeployPanelBase::SelectEntry(int32 sectionIndex, int32 row, int32 col)
{
    int32 entryNum = _sections[sectionIndex]->GetEntryNumPerRow(); // 한 행에 몇 열 들어갈 수 있는지
    if (entryNum <= 0) return;

    int32 entryIndex = row * entryNum + col;

    auto entry = _sections[sectionIndex]->GetEntries()[entryIndex];
    entry->HandlePick();

    if (auto scrollBox = Cast<UScrollBox>(_sectionPanel))
    {
        scrollBox->ScrollWidgetIntoView(
            entry,
            true,
            EDescendantScrollDestination::IntoView,
            0.15f
        );
    }

    _curSectionIndex = sectionIndex;
    _curRow = row;
    _curCol = col;
}
