// Fill out your copyright notice in the Description page of Project Settings.


#include "HelldiverReinforceManager.h"
#include "../PlayerCharacter.h"
#include "../../Controller/MainPlayerController.h"

#include "NavigationSystem.h"
// Sets default values
AHelldiverReinforceManager::AHelldiverReinforceManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AHelldiverReinforceManager::BeginPlay()
{
	Super::BeginPlay();
	InitSquad();

	
}

void AHelldiverReinforceManager::InitSquad()
{


	FActorSpawnParameters param;
	param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (_hellDiverClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ReinforcementManager Init Fail"));
		return;
	}
	for (int i = 0; i < _remainReinforceBudget; i++)
	{
		APlayerCharacter* helldiver = GetWorld()->SpawnActor<APlayerCharacter>(_hellDiverClass, FVector::ZeroVector, FRotator::ZeroRotator, param);
		if (helldiver)
		{
			UE_LOG(LogTemp, Display, TEXT("SpawnUnit"));
			helldiver->UnitDeactivate();
			helldiver->AutoPossessPlayer = EAutoReceiveInput::Disabled;
			helldiver->AutoPossessAI = EAutoPossessAI::Disabled;
			_hellDiverPool.Add(helldiver);
		}
	}

		UE_LOG(LogTemp, Error, TEXT("ReinforcementManager Init Success"));
}

void AHelldiverReinforceManager::ReinforceHelldiver(FVector callPoint)
{
	//헬포드에 빙의시키고 헬다이버는 내부의 소켓에 어태치하고 땅으로 떨어진후에 헬다이버에 빙의해야하지만 테스트 버전이니 바로 호출위치 주변 랜덤위치에 드롭하자.
	AController* temp=nullptr;
	APlayerCharacter* extra = nullptr;
	if (_controllerQ.Dequeue(temp) == false)
		return;
	if (temp == nullptr)
		return;
	for (auto helldiver : _hellDiverPool)
	{
		if (helldiver->IsReadyToSpawn())
		{
			extra = helldiver;
			break;
		}
	}

	//NavMesh 찾기 : 이 지점을 기준으로 특정범위내에 소환가능위치가 있는지 확인. 
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (naviSystem->IsValidLowLevel() == false)
		return ;



	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수 여기서 가능한 위치가 없으면 false를 반환.
	if (naviSystem->GetRandomPointInNavigableRadius(callPoint, 2000, randLocation))
	{
		extra->SetActorLocation(randLocation);

	}
	else
	{

		extra->SetActorLocation(callPoint);
	}

	temp->Possess(extra);
	extra->ResetUnit();

}

void AHelldiverReinforceManager::ReturnDeadController(AController* controller)
{

	UE_LOG(LogTemp, Display, TEXT("ReturnDeadController  : %s"), *controller->GetName());
	_controllerQ.Enqueue(controller);
}

