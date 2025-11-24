// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforceSquadSpawner.h"

void AReinforceSquadSpawner::CallFinishAction()
{
	// 증원 스쿼드는 소환이 끝나면 스포너를 돌려보낸다.
	DeactivateSpawner();
}
