// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployPanelBase.h"

void UPreDeployPanelBase::InitializePanel(UPreDeploymentState* state)
{
	_state = state;

    const int32 Count = _panel->GetChildrenCount();
    for (int32 i = 0; i < Count; ++i) {
        if (USelectableEntryBase* entry = Cast<USelectableEntryBase>(_panel->GetChildAt(i)))
        {
            entry->_onPickedEvent.AddUObject(this, &UPreDeployPanelBase::HandlePicked);
        }
    }
}

void UPreDeployPanelBase::HandlePicked(int32 itemID)
{
    // state에 정보 넘겨주기
    // 임시
    _state->SetPrimaryGunID(itemID);
}
