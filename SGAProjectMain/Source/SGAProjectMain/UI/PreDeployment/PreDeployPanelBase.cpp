// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployPanelBase.h" 

void UPreDeployPanelBase::InitializePanel(UPreDeploymentState* state)
{
    _state = state;
}

void UPreDeployPanelBase::HandleEntryPicked(UPreDeployEntryBase* entry)
{
    if (_curSelectedEntry)
        _curSelectedEntry->SetSelected(false); // 이전 선택 해제

	entry->SetSelected(true); // 새로 선택된 엔트리 강조
    _curSelectedEntry = entry;
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
