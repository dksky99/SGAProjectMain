// Fill out your copyright notice in the Description page of Project Settings.


#include "FactorySquadSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "../../../Object/CDamageType.h"
AFactorySquadSpawner::AFactorySquadSpawner()
{

    PrimaryActorTick.bCanEverTick = false;

    PrimaryActorTick.TickInterval = 0.5f;
}
void AFactorySquadSpawner::BeginPlay()
{

    Super::BeginPlay();
}
void AFactorySquadSpawner::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);

    CheckDistanceToTarget();

}

void AFactorySquadSpawner::ActivateSpawner(AEnemySquad* squad, FVector loc)
{
    Super::ActivateSpawner(squad, loc);
    PrimaryActorTick.bCanEverTick = true;

}

void AFactorySquadSpawner::DeactivateSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    Super::DeactivateSpawner();
}

void AFactorySquadSpawner::CheckDistanceToTarget()
{
    // 인덱스 0은 항상 첫 번째 로컬 플레이어입니다.
    APlayerController* PlayerController0 = UGameplayStatics::GetPlayerController(
        GetWorld(),
        0 // 플레이어 인덱스 (0, 1, 2, ...)
    );

    if (PlayerController0)
    {
        auto pawn = PlayerController0->GetPawn();
        if (pawn == nullptr)
            return;
        float distance = GetDistanceTo(pawn);
        if (distance < _alertRange)
            SpawnUnits();
    }
}

float AFactorySquadSpawner::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//커스텀 데미지이벤트. 이곳에 피해를 입은 부위와 일반피해, 내구피해, 철거력, 관통력 등을 가져올 수 있다.그리고 상태이상을 유발한다면 얼마나가중할지도 포함된다.
	if (DamageEvent.GetTypeID() == FCDamageEvent::ClassID)
	{
		// 1. FCDamageEvent 타입으로 안전하게 캐스팅
		// &DamageEvent는 FDamageEvent const*이므로, FCDamageEvent const*로 static_cast합니다.
		const FCDamageEvent* CustomEvent = static_cast<const FCDamageEvent*>(&DamageEvent);
		//데미지타입을 가져온다 여기에는 피해의 속성과 이것이 추가적인상태이상수치를 유발하는지 여부를 가져온다.
		//데미지타입이 있다면 그것으로하고 없다면 기본클래스를 만들어 사용.
		const UCDamageType* CustomDamageType = Cast<UCDamageType>(CustomEvent->DamageTypeClass->GetDefaultObject()) != nullptr ?
			Cast<UCDamageType>(CustomEvent->DamageTypeClass->GetDefaultObject()) :
			Cast<UCDamageType>(UCDamageType::StaticClass()->GetDefaultObject());



		ProcessDamage( CustomEvent);




	}


	// 기본 로직을 반드시 호출
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AFactorySquadSpawner::ProcessDamage(const FCDamageEvent* damageEvent)
{
    if (damageEvent->ColComp->ComponentHasTag(TEXT("Core")))
    {
        if (damageEvent->DemolitionDamage >= 20.f)
        {
            //파괴
        }
    }
    else
    {
        if (damageEvent->DemolitionDamage >= 40.f)
        {
            //파괴

        }
    }
   
}

void AFactorySquadSpawner::CallFinishAction()
{
   
}
