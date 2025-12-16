// Fill out your copyright notice in the Description page of Project Settings.


#include "BroadcastTower.h"

#include "Engine/DamageEvents.h"
#include "DestructFieldActor.h"
#include "../../MainGameMode.h"

// Sets default values
ABroadcastTower::ABroadcastTower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_geometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	RootComponent = _geometryCollection;

    _terminalChild->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABroadcastTower::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABroadcastTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float ABroadcastTower::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    FHitResult hitInfo;
    FVector impulseDir;
    DamageEvent.GetBestHitInfo(this, DamageCauser, hitInfo, impulseDir);

    FVector impactPoint = hitInfo.ImpactPoint;

    if (DamageAmount >= 3000.f)
    {
        // 파괴 필드 스폰
        FActorSpawnParameters Params;
        ADestructFieldActor* FieldActor = GetWorld()->SpawnActor<ADestructFieldActor>(
            ADestructFieldActor::StaticClass(),
            impactPoint,
            FRotator::ZeroRotator,
            Params
        );

        if (FieldActor)
        {
            FieldActor->ActivateField(impactPoint);
        }

        UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
        if (!World) return DamageAmount;

        OnCommandCompleted();
        _terminalChild->SetChildActorClass(nullptr); // 터미널 제거
    }

    return DamageAmount;
}

void ABroadcastTower::OnCommandCompleted()
{
    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        GM->OnObjectiveCleared(_objectiveID);
    }
}

