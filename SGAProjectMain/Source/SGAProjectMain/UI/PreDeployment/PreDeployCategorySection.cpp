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
		_onEntrySpawnedEvent.Broadcast(entry); // 엔트리 생성 이벤트 브로드캐스트 -> 패널에서 처리
		//entry->_onPickedEvent.AddUObject(this, &UPreDeployCategorySection::HandlePicked); // 선택 이벤트 바인딩
		_entryPanel->AddChildToUniformGrid(entry, idx / _entryRowWidth, idx % _entryRowWidth); // 섹션의 패널에 추가

		_entries.Add(entry); // 내부 목록에도 추가
		idx++;
	}
}
