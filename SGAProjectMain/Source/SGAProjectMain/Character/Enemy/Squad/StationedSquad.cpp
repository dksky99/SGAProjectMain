// Fill out your copyright notice in the Description page of Project Settings.


#include "StationedSquad.h"
#include "../Enemy.h"
#include "../../../Controller/EnemyController.h"
#include "NavigationSystem.h"
#include "SquadSpawner.h"
#include "FactorySquadSpawner.h"
AStationedSquad::AStationedSquad()
{
}

void AStationedSquad::BeginPlay()
{
	Super::BeginPlay();
	Command_Stationed();
	_targetLoc = GetActorLocation();
	ActivateFactory();

}

void AStationedSquad::RecieveDamage(float damage, float armorPen)
{
	//방어력이 더 높으면 무시
	if (_defense > armorPen)
		return;
	_curDurability = FMath::Clamp(_curDurability - damage, 0, _curDurability);
	//내구가 다하면 붕괴.
	if (_curDurability <= 0)
		DestroyFactory();
}

void AStationedSquad::CallRemainUnit()
{
	UE_LOG(LogTemp, Display, TEXT("TryAddUnit"));
	if (_squadState == ESquadState::Deactivate)
		return;
	auto extra = CheckExtraUnit();
	if (extra)
	{
		UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
		//SpawnUnit(extra);
		extra->SetActorRotation(this->GetActorQuat());
		extra->GetCachedController()->RecieveTargetLoc(MakeRandomLocation());
		
	}

	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer,this, &AStationedSquad::CallRemainUnit, _generateCoolDown,false);


	return;
}

void AStationedSquad::ActivateFactory()
{

	_curDurability = _maxDurability;

	UE_LOG(LogTemp, Display, TEXT("StartSpawn"));
	GetWorld()->GetTimerManager().SetTimer(_GenerateTimer, this, &AStationedSquad::CallRemainUnit, _generateCoolDown, false);
}

void AStationedSquad::DestroyFactory()
{
	Command_Deactivate();

}

float AStationedSquad::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	//커스텀 데미지이벤트. 이곳에 피해를 입은 부위와 일반피해, 내구피해, 철거력, 관통력 등을 가져올 수 있다.그리고 상태이상을 유발한다면 얼마나가중할지도 포함된다.
	if (DamageEvent.GetTypeID() == FCDamageEvent::ClassID)
	{
		// 1. FCDamageEvent 타입으로 안전하게 캐스팅
		// &DamageEvent는 FDamageEvent const*이므로, FCDamageEvent const*로 static_cast합니다.
		const FCDamageEvent* CustomEvent = static_cast<const FCDamageEvent*>(&DamageEvent);
		//데미지타입을 가져온다 여기에는 피해의 속성과 이것이 추가적인상태이상수치를 유발하는지 여부를 가져온다.
		//데미지타입이 있다면 그것으로하고 없다면 기본클래스를 만들어 사용.
		


	}


	// 기본 로직을 반드시 호출
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

}
