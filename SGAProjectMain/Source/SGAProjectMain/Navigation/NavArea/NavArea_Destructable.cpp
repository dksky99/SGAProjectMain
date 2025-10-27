// Fill out your copyright notice in the Description page of Project Settings.


#include "NavArea_Destructable.h"

UNavArea_Destructable::UNavArea_Destructable(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
    // 디버깅 시 네비메시에서 이 영역을 빨간색으로 표시합니다.
    DrawColor = FColor::Red;

    // 기본 비용을 설정할 수 있지만, 필터에서 제외할 것이므로 큰 의미는 없습니다.
    DefaultCost = 100.0f;
}
