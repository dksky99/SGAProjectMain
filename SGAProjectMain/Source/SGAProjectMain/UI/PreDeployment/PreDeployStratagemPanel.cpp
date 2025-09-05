// Fill out your copyright notice in the Description page of Project Settings.


#include "PreDeployStratagemPanel.h"

void UPreDeployStratagemPanel::HandlePicked(int32 stgID)
{
    _state->SetStratagemID(stgID);

    for (auto* entry : _entries)
    {
        if (entry->GetItemID() == stgID)
            entry->SetSelected(true);
    }
}
