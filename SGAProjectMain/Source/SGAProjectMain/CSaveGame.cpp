// Fill out your copyright notice in the Description page of Project Settings.


#include "CSaveGame.h"

void UCSaveGame::ResetOperationData()
{
	_curOperationID = FName();
	_completedMissionIDs.Empty();
}

void UCSaveGame::SetCurOperationID(FName operationID)
{
	if (operationID == _curOperationID)
		return;

	// 현재 진행 중인 임무 ID가 변경되었을 경우에만 갱신
	ResetOperationData();
	_curOperationID = operationID;
}
