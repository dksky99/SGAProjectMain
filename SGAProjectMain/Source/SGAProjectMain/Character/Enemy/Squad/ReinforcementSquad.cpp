// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementSquad.h"

#include "../Enemy.h"
#include "NavigationSystem.h"

void AReinforcementSquad::Init()
{
    Super::Init();

    

}

bool AReinforcementSquad::CheckAbleToCall(FVector origin, FVector target = FVector(0,0,0))
{
    if (_isReadyToCall == false)
        return false;

    if (_squadState != ESquadState::Deactivate)
        return false;


    FVector pos = origin;

    //NavMesh 찾기
    auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

    if (naviSystem->IsValidLowLevel() == false)
        return false;



    //반환받을 랜덤한 위치.
    FNavLocation randLocation;
    //일정 반경안의 랜덤한 지점을 가져오는 함수
    if (naviSystem->GetRandomPointInNavigableRadius(pos, _callRadius, randLocation))
    {
        _squadState = ESquadState::Search;
        SetTargetLoc(target);
        SetActorLocation(origin);

        CallReinforcement();
        return true;
    }




    return false;
}

void AReinforcementSquad::CallReinforcement()
{

    UE_LOG(LogTemp, Display, TEXT("CallReinforce"));
    auto extra = CheckExtraUnit();
    if (extra==nullptr)
    {
        GetWorld()->GetTimerManager().SetTimer(_callUnitTimer, this, &AReinforcementSquad::ReadyToCall, _coolDown, false);
        return;
    }
    _spawnPoint->SetWorldLocation( GetCallPoint(GetActorLocation()));
    UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
    SpawnUnit(extra);
    //Todo: extra-> 생성 이후 목표 위치를 세팅해줘야함
    float nextCall = FMath::FRandRange(_callingDelay_Min, _callingDelay_Max);

    GetWorld()->GetTimerManager().SetTimer(_callUnitTimer, this, &AReinforcementSquad::CallReinforcement, nextCall, false);


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

