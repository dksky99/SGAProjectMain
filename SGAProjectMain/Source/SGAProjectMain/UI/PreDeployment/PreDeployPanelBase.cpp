// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployPanelBase.h" 

#include "PreDeployDetailBase.h"

void UPreDeployPanelBase::InitializePanel(UPreDeploymentState* state)
{
    _state = state;
}

void UPreDeployPanelBase::HandleEntrySelected(UPreDeployEntryBase* entry)
{
    if (!entry) return;
    FocusEntry(entry);
}

void UPreDeployPanelBase::FocusEntry(UPreDeployEntryBase* entry)
{
    if (_curSelectedEntry)
        _curSelectedEntry->SetSelected(false); // 이전 선택 해제

    entry->SetSelected(true); // 새로 선택된 엔트리 강조
    _curSelectedEntry = entry;

    FText sectionText = GetSectionText(entry);
    if (_curSectionText)
    {
        _curSectionText->SetVisibility(ESlateVisibility::Visible);
        _curSectionText->SetText(sectionText); // 현재 섹션 텍스트 업데이트
    }

    if (_detailPanel)
    {
        _detailPanel->SetVisibility(ESlateVisibility::Visible);
        _detailPanel->SetDetail(entry->GetItemID()); // 상세 패널 업데이트
    }

    if (auto scrollBox = Cast<UScrollBox>(_sectionPanel))
    {
        scrollBox->ScrollWidgetIntoView(
            entry,
            true,
            EDescendantScrollDestination::IntoView,
            0.15f
        );
    }

    _curSectionIndex = entry->GetSection();
    _curRow = entry->GetRow();
    _curCol = entry->GetCol();
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

void UPreDeployPanelBase::JumpToPrevSection()
{
    int32 newSecIndex = _curSectionIndex - 1;
    if (newSecIndex < 0)
        newSecIndex = _sections.Num() - 1;

    SelectSectionJumpTargetEntry(newSecIndex);
}

void UPreDeployPanelBase::JumpToNextSection()
{
    int32 newSecIndex = _curSectionIndex + 1;
    if (newSecIndex >= _sections.Num())
        newSecIndex = 0;

    SelectSectionJumpTargetEntry(newSecIndex);
}

void UPreDeployPanelBase::OnEntrySpawned(UPreDeployEntryBase* entry)
{
	entry->_onPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandleEntrySelected);
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

void UPreDeployPanelBase::SelectSectionJumpTargetEntry(int32 sec)
{
    auto curSection = _sections[sec];

    int32 lastRow = GetRowNumInSection(sec) - 1;
    if (lastRow < 0)
        return;

    if (lastRow == 0) // 만약 행이 하나 뿐이라면
    {
        // 첫 행에서 기존 열 유지. 없을 경우 가장 큰 열 선택
        int32 newCol = FMath::Min(_curCol, GetColumnNumInRow(sec, 0) - 1);
        SelectEntry(sec, 0, newCol);
        return;
    }

    if (_curRow < lastRow) // 만약 현재 가리키는 row가 lastRow보다 작다면
    {
        // lastRow 이전 행은 모두 꽉 차있기 때문에 목표 세션에서 동일한 위치에 존재하는 엔트리 바로 선택
        SelectEntry(sec, _curRow, _curCol);
        return;
    }

    else // 만약 현재 가리키는 row가 lastRow와 같거나 그보다 크다면
    {
        int32 colNum = GetColumnNumInRow(sec, lastRow); // 마지막 행에서 우선 시도
        if (_curCol < colNum)
        {
            // 개수가 충분할 경우 마지막 행에서 동일 열 유지
            SelectEntry(sec, lastRow, _curCol);
            return;
        }
        else
        {
            // 개수가 부족할 경우 lastRow 직전 행에서 동일 열 유지
            SelectEntry(sec, lastRow - 1, _curCol);
            return;
        }
    }
}

void UPreDeployPanelBase::SelectEntry(int32 sectionIndex, int32 row, int32 col)
{
    int32 entryNum = _sections[sectionIndex]->GetEntryNumPerRow(); // 한 행에 몇 열 들어갈 수 있는지
    if (entryNum <= 0) return;

    int32 entryIndex = row * entryNum + col;

    auto entry = _sections[sectionIndex]->GetEntries()[entryIndex];
    FocusEntry(entry);
}
