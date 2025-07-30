// Fill out your copyright notice in the Description page of Project Settings.


#include "BroadcastTower.h"

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

	DrawDebugBox(GetWorld(), _geometryCollection->GetComponentLocation(), FVector(50), FColor::Green, false, 3.0f);
}

//float ABroadcastTower::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
//{
//	/*if (_geometryCollection)
//	{
//		FVector hitLocation = GetActorLocation();
//
//		FVector impulseDirection = (hitLocation - DamageCauser->GetActorLocation()).GetSafeNormal();
//		float impulseStrength = DamageAmount;
//
//		_geometryCollection->AddImpulseAtLocation(impulseDirection * impulseStrength, _geometryCollection->GetComponentLocation());
//		UE_LOG(LogTemp, Warning, TEXT("DamageAmount: %f, ImpulseStrength: %f"), DamageAmount, impulseStrength);
//	}*/
//
//	_geometryCollection->ApplyExternalStrain(100000.f, _geometryCollection->GetComponentLocation(), 500.f);
//
//	return DamageAmount;
//}

