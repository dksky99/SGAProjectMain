// Fill out your copyright notice in the Description page of Project Settings.


#include "FactorySquadSpawner.h"
#include "Kismet/GameplayStatics.h"
#include "../../../Object/CDamageType.h"
AFactorySquadSpawner::AFactorySquadSpawner():Super()
{

    PrimaryActorTick.bCanEverTick = true; 

    SetActorTickEnabled(false);

    PrimaryActorTick.TickInterval = 0.5f;
    _callRadius = 0.0f;

    _minSpawnInterval = 4.0f;
    _maxSpawnInterval = 6.0f;

    _maxSpawnCount = 4;
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

void AFactorySquadSpawner::SpawnUnits()
{

    if (_spawnerReady == false)
        return;
    _spawnerReady = false;
    Super::SpawnUnits();

    //한번 소환을 했다면 3분이후에 소환이 가능하다.
    GetWorld()->GetTimerManager().SetTimer(_spawnerTimer, this, &ASquadSpawner::SpawnerCoolDownFinish, _spawnerCoolTime, false);
}

void AFactorySquadSpawner::ActivateSpawner(AEnemySquad* squad, FVector loc)
{
    //팩토리 스포너는 파괴되기전까지 계속유지되며 헬다이버의 위치가 스포너에 가까워지면 그에따라 소환을 시작해야한다.
    //그러니 틱을켜서 헬다이버와의 거리를 직접 확인한다.
    //팩토리는 2번째 매개변수가 의미가 없다.
    Super::ActivateSpawner(squad, loc);

    SetActorTickEnabled(true);
    UE_LOG(LogTemp, Display, TEXT("Factory Activate"));

}

void AFactorySquadSpawner::DeactivateSpawner()
{

    SetActorTickEnabled(false);
    _onFactoryDestroyed.Broadcast(_squad);
    Super::DeactivateSpawner();
}

void AFactorySquadSpawner::CheckDistanceToTarget()
{
    // 인덱스 0은 항상 첫 번째 로컬 플레이어입니다.
    APlayerController* PlayerController0 = UGameplayStatics::GetPlayerController(
        GetWorld(),
        0 
    );

    if (PlayerController0)
    {
        auto pawn = PlayerController0->GetPawn();
        if (pawn == nullptr)
            return;
        float distance = GetDistanceTo(pawn);
        if (distance < _alertRange)
        {

            UE_LOG(LogTemp, Display, TEXT("Helldiver InRange"));
            SpawnUnits();
        }
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
    //코어에 20이상의 철거를 가하거나
    if (damageEvent->ColComp->ComponentHasTag(TEXT("Core")))
    {
        if (damageEvent->DemolitionDamage >= 20.f)
        {
            //파괴
            DestroyFactory();
        }
    }
    //표면에 40이상의 철거를 가하거나.
    else
    {
        if (damageEvent->DemolitionDamage >= 40.f)
        {
            //파괴
            DestroyFactory();

        }
    }
   
}

void AFactorySquadSpawner::DestroyFactory()
{
    DeactivateSpawner();
}
void AFactorySquadSpawner::CallFinishAction()
{
   
}
