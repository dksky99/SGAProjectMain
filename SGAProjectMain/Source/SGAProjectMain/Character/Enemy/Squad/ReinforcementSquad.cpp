// Fill out your copyright notice in the Description page of Project Settings.


#include "ReinforcementSquad.h"

#include "NavigationSystem.h"

void AReinforcementSquad::Init()
{
    Super::Init();

    for (auto& pairs : _unitPool)
    {
        _unitMem.Add(pairs.Key);

        for (auto& pair : pairs.Value._units)
        {
            _unitMem[pairs.Key]._units.Add(pair.Key,true);
        }


    }

}

bool AReinforcementSquad::CheckAbleToCall(FVector origin)
{
    if (_isReadyToCall == false)
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
        CallReinforcement(origin);
        return true;
    }




    return false;
}

void AReinforcementSquad::CallReinforcement(FVector target)
{

    UE_LOG(LogTemp, Display, TEXT("TryAddUnit"));
    auto extra = CheckExtraUnit();
    if (extra)
    {
        UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
        SpawnUnit(extra);
    }

    GetWorld()->GetTimerManager().SetTimer(_callUnitTimer, this, &AReinforcementSquad::CallRemainUnit, _generateCoolDown, false);


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
}

