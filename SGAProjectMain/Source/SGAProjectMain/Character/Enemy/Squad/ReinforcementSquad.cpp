// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementSquad.h"

#include "../Enemy.h"
#include "NavigationSystem.h"

void AReinforcementSquad::Init()
{
    Super::Init();

    

}

bool AReinforcementSquad::CheckAbleToCall(FVector origin, FVector& target)
{
    // 호출가능상태인지 확인
    if (_isReadyToCall == false)
        return false;
    //이미 어떤식으로든 활성화된 상태인지
    if (_squadState != ESquadState::Deactivate)
        return false;


    FVector pos = origin;

    //NavMesh 찾기 : 이 지점을 기준으로 특정범위내에 소환가능위치가 있는지 확인. 
    auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

    if (naviSystem->IsValidLowLevel() == false)
        return false;



    //반환받을 랜덤한 위치.
    FNavLocation randLocation;
    //일정 반경안의 랜덤한 지점을 가져오는 함수 여기서 가능한 위치가 없으면 false를 반환.
    if (naviSystem->GetRandomPointInNavigableRadius(pos, _callRadius, randLocation))
    {
        target = pos;

        return true;
    }




    return false;
}

bool AReinforcementSquad::CallReinforcement(FVector target)
{

    UE_LOG(LogTemp, Display, TEXT("CallReinforce"));
    //
    //남아있는 유닛이 있는지 확인.

    auto extra = CheckExtraUnit();
    if (extra == nullptr)
    {
        return false;
    }
    _isReadyToCall = false;
    _squadState = ESquadState::Search;

    SetTargetLoc(target);
    SetActorLocation(target);

    CallUnit();

    return true;

}

void AReinforcementSquad::CallUnit()
{

    auto extra = CheckExtraUnit();
    if (extra == nullptr)
    {
        //여러차례의 딜레이 재귀 호출 후 이 스쿼드의 모든 병력이 소환되었을 경우. 증원 스쿼드는 
        GetWorld()->GetTimerManager().SetTimer(_callUnitTimer, this, &AReinforcementSquad::ReadyToCall, _coolDown, false);
        return;
    }
    _spawnPoint->SetWorldLocation(GetCallPoint(GetActorLocation()));
    SpawnUnit(extra);


    float nextCall = FMath::FRandRange(_callingDelay_Min, _callingDelay_Max);

    GetWorld()->GetTimerManager().SetTimer(_callUnitTimer, this, &AReinforcementSquad::CallUnit, nextCall, false);
}

bool AReinforcementSquad::SetTargetLoc(FVector target)
{

    FVector pos = target;

    //NavMesh 찾기
    auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());




    //반환받을 랜덤한 위치.
    FNavLocation randLocation;
    //일정 반경안의 랜덤한 지점을 가져오는 함수
    if (naviSystem->GetRandomPointInNavigableRadius(pos, _callRadius, randLocation))
    {
        _targetLoc=randLocation;
    }

    return false;
}

//중심을 기준으로 원형으로 나선형으로 배치하면 적당하지 않을까
//중심으로부터 sin으로 z축을 기준으로 나선
// 4분에1 파이마다 하나씩
//적당히 높은위치에서 
FVector AReinforcementSquad::GetCallPoint( FVector origin)
{

    FVector pos = origin;

    //NavMesh 찾기
    auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());




    //반환받을 랜덤한 위치.
    FNavLocation randLocation;
    //일정 반경안의 랜덤한 지점을 가져오는 함수
    if (naviSystem->GetRandomPointInNavigableRadius(pos, _callRadius, randLocation))
    {
        return randLocation;
    }


    return origin;
}

void AReinforcementSquad::ReadyToCall()
{
    _isReadyToCall = true;
    for (auto& pairs : _unitPool)
    {

        for (auto& pair : pairs.Value._units)
        {
            pair.Key->ReadyToSpawn();
        }


    }
}

