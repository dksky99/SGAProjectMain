// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "../CharacterStateComponent.h"
#include "Components/WidgetComponent.h"
#include "../../UI/DummyHpBar.h"
#include "../StatComponent.h"

#include "../../Controller/EnemyController.h"
#include "EnemySquad.h"
AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Super::_stateComp = CreateDefaultSubobject<UCharacterStateComponent>(TEXT("State"));

    _hpBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBar"));
    _hpBarWidget->SetupAttachment(GetMesh());
    _hpBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    _hpBarWidget->SetRelativeLocation(FVector(0, 0, 230.0f));
}

void AEnemy::BeginPlay()
{
    Super::BeginPlay();



    auto hpBar = Cast<UDummyHpBar>(_hpBarWidget->GetWidget());
    if (hpBar)
    {
       // _statComponent->_enemyHpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
        _statComponent->_hpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
    }
}

void AEnemy::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //UE_LOG(LogTemp, Display, TEXT("%s PossesedBy :%s"), *(this->GetName()), *(NewController->GetName()));

}

void AEnemy::UnPossessed()
{
    Super::UnPossessed();

    UE_LOG(LogTemp, Display, TEXT("%s UnPossessed"), *(this->GetName()));

}

void AEnemy::FoundTarget(ACharacterBase* target)
{
}

void AEnemy::RaiseAlert()
{

}

bool AEnemy::AddToSquad(AEnemySquad* squad)
{
    if(squad ==nullptr)
        return false;
    _squad = squad;
    if(_squad==nullptr)
        return false;
    return true;
}

void AEnemy::UnitDeactivate()
{

    AEnemyController* controller = Cast<AEnemyController>(GetController());
    if (controller)
    {
        controller->UnPossess();
    }
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);

    
    SetActorLocation(FVector::ZeroVector);

}

void AEnemy::Dead()
{
    Super::Dead();


}

void AEnemy::SpawnGhost()
{
    Super::SpawnGhost();

    UnitDeactivate();

}

void AEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

    Super::EndPlay(EndPlayReason);
}

void AEnemy::Destroyed()
{

    Super::Destroyed();
}

//float AEnemy::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
//{
//	_statComponent->ChangeHp(-damageAmount);
//
//	UE_LOG(LogTemp, Log, TEXT("Damage : %f"), damageAmount);
//
//	if (_statComponent->IsDead())
//		Dead();
//
//	return 0.0f;
//}
