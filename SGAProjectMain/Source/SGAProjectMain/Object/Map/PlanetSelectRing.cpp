// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetSelectRing.h"

// Sets default values
APlanetSelectRing::APlanetSelectRing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

}

// Called when the game starts or when spawned
void APlanetSelectRing::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlanetSelectRing::PlaceOnSurface(FVector point, FVector normalVec, FVector upTangent)
{
	FVector X = normalVec.GetSafeNormal(); // 표면의 법선 벡터
	const FVector Y = upTangent.GetSafeNormal(); // 위쪽 접선 벡터
	const FVector Z = FVector::CrossProduct(X, Y).GetSafeNormal(); // 오른쪽 접선 벡터

	SetActorLocation(point + X * 2.f); // 표면에서 살짝 띄우기
	SetActorRotation(FRotationMatrix::MakeFromXZ(X, Z).Rotator());
}

// Called every frame
void APlanetSelectRing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

