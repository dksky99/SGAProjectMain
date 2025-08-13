// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "../CharacterAnimInstance.h"
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
        _statComponent->_coreHpChanged.AddUObject(hpBar, &UDummyHpBar::SetHp);
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

void AEnemy::Spawn()
{
    GetWorld()->GetTimerManager().SetTimer(_respawnTimer,this, &AEnemy::ReadyToSpawn,_respawnCoolDown,false );
    _isReadyToSpawn = false;
    //스폰몽타주가 있을시. 몽타주가 끝나고 컨트롤러 결합을할지 고민중
    if (_spawnMontage)
    {
        if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
        {

            animInstance->PlayAnimMontage(_spawnMontage);

            // 재생 후 인스턴스 가져오기
            if (FAnimMontageInstance* montageInstance = animInstance->GetActiveInstanceForMontage(_spawnMontage))
            {

            }
        }
    }
}

bool AEnemy::IsReadyToSpawn()
{
    if (GetController())
        return false;
   
    return _isReadyToSpawn;
}

void AEnemy::ReadyToSpawn()
{
    _isReadyToSpawn = true;
    if (GetWorld()->GetTimerManager().IsTimerActive(_respawnTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(_respawnTimer);
    }
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
