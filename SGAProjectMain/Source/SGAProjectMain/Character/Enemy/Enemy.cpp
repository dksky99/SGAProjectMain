// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "EnemyStatComponent.h"

#include "Components/WidgetComponent.h"
#include "../../UI/DummyHpBar.h"

AEnemy::AEnemy(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    _statComponent = CreateDefaultSubobject<UEnemyStatComponent>(TEXT("Stat"));

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
        _statComponent->_hpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
}

float AEnemy::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
{
    _statComponent->ChangeHp(-damageAmount);

    UE_LOG(LogTemp, Log, TEXT("Damage : %f"), damageAmount);

    if (_statComponent->IsDead())
        Dead();
    
    return 0.0f;
}
