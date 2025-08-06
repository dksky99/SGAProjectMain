// Fill out your copyright notice in the Description page of Project Settings.


#include "BroadcastTower.h"
#include "Kismet/GameplayStatics.h"
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
    const FPointDamageEvent* PointEvent = nullptr;

    if (DamageEvent.GetTypeID() == FPointDamageEvent::ClassID)
        PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
    

    FVector ImpactPoint = GetActorLocation(); // 기본값
    if (PointEvent)
    {
        ImpactPoint = PointEvent->HitInfo.ImpactPoint;
    }

    if (DamageAmount >= 5000.f)
    {
        // 파괴 필드 스폰
        FActorSpawnParameters Params;
        ADestructFieldActor* FieldActor = GetWorld()->SpawnActor<ADestructFieldActor>(
            ADestructFieldActor::StaticClass(),
            ImpactPoint,
            FRotator::ZeroRotator,
            Params
        );

        if (FieldActor)
        {
            FieldActor->ActivateField(ImpactPoint);
        }

        UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
        if (!World) return DamageAmount;

        AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
        if (GM)
        {
            GM->CallEscapePlane();
        }
    }

    return DamageAmount;
}

