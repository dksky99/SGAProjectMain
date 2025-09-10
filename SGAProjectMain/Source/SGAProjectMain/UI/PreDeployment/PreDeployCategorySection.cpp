// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployCategorySection.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"

void UPreDeployCategorySection::InitializeSection(FText title, const TArray<int32> ids)
{
	_entryPanel->ClearChildren();
	_titleText->SetText(title);

	int32 idx = 0;
	for (const int32 id : ids)
	{
		UPreDeployEntryBase* entry = CreateWidget<UPreDeployEntryBase>(this, _entryClass); // 엔트리 위젯 생성
		entry->InitializeEntry(id); // 총 ID로 초기화
		entry->_onPickedEvent.AddUObject(this, &UPreDeployCategorySection::HandlePicked); // 선택 이벤트 바인딩
		_entryPanel->AddChildToUniformGrid(entry, idx / _entriesPerRow, idx % _entriesPerRow); // 섹션의 패널에 추가

		_entries.Add(entry); // 내부 목록에도 추가
		idx++;
	}
}

void UPreDeployCategorySection::HandlePicked(int32 itemID)
{
	UPreDeployEntryBase* selected = nullptr;

	for (auto* entry : _entries)
	{
		if (entry->GetItemID() == itemID)
			selected = entry;
	}

	if (_onSectionPickedEvent.IsBound())
		_onSectionPickedEvent.Broadcast(itemID, selected); // ID와 선택된 엔트리 전달
}
