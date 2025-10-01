// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Standard.h"
#include "Components/ShapeComponent.h"
#include "Components/CapsuleComponent.h"
#include "../../CharacterStateComponent.h"
#include "../../CharacterAnimInstance.h"
#include "../../../Data/UnitAttackDataAsset.h"
#include "NavigationSystem.h"

AEnemy_Standard::AEnemy_Standard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	

}


bool AEnemy_Standard::TryNear(AActor* target)
{
	if (AttackMelee())
		return true;


	return false;
}

bool AEnemy_Standard::TryMiddle(AActor* target)
{
	return false;
}

bool AEnemy_Standard::TryFar(AActor* target)
{
	return false;
}


bool AEnemy_Standard::TryBurrow(AActor* target)
{

	FVector pos = target->GetActorLocation();

	//NavMesh 찾기
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (naviSystem->IsValidLowLevel() == false)
		return false;



	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수
	if (naviSystem->GetRandomPointInNavigableRadius(pos, 100, randLocation))
	{
		BurrowIn(randLocation);
		return true;
	}
	return false;
}

void AEnemy_Standard::BurrowIn(FVector target)
{
	//애님 몽타주 이후 사라진 후 타겟 위치로 이동,특정 시간 후 애님몽타주를 실행하면서 모습을 드러냄.


}

void AEnemy_Standard::BurrowOut()
{
}

void AEnemy_Standard::ActivateClaw_L()
{
}

void AEnemy_Standard::ActivateClaw_R()
{
}

void AEnemy_Standard::DeactivateClaw_L()
{
}

void AEnemy_Standard::DeactivateClaw_R()
{
}

