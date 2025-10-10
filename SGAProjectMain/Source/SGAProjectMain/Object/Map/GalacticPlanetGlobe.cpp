// Fill out your copyright notice in the Description page of Project Settings.


#include "GalacticPlanetGlobe.h"

#include "PlanetOperationSite.h"

// Sets default values
AGalacticPlanetGlobe::AGalacticPlanetGlobe()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_mesh->SetSimulatePhysics(true);
	RootComponent = _mesh;
}

// Called when the game starts or when spawned
void AGalacticPlanetGlobe::BeginPlay()
{
	Super::BeginPlay();

    InitializeOperations();
	
	_playerController = GetWorld()->GetFirstPlayerController();
}

void AGalacticPlanetGlobe::InitializeOperations()
{
    for (FOperationData& opData : _operations)
    {
        if (!opData.OperationSiteClass)
            continue;

        UChildActorComponent* opSite = NewObject<UChildActorComponent>(this);
        if (opSite)
        {
            opSite->SetupAttachment(RootComponent);
            opSite->SetChildActorClass(opData.OperationSiteClass);
            opSite->RegisterComponent();
            _operationSites.Add(opSite);

            FVector localPos = CalculateGlobePosition(
                opData._latitude,
                opData._longitude,
                _globeRadius
            );
            opSite->SetRelativeLocation(localPos);

            // 표면을 향하도록 회전 (중심에서 바깥쪽)
            FRotator lookRotation = (-localPos).Rotation();
            opSite->SetRelativeRotation(lookRotation);
        }
    }
}

// Called every frame
void AGalacticPlanetGlobe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (!_isInteracting) return;

	FVector2D delta;
	_playerController->GetInputMouseDelta(delta.X, delta.Y);
	delta *= 15.f;
	
	FQuat quat = _mesh->GetComponentQuat();

	FVector worldRight = FVector::RightVector; // 월드 기준 오른쪽 벡터

	float newPitch = FMath::Clamp(_curPitchDeg + -delta.Y, -70.f, 70.f);
	float deltaPitch = newPitch - _curPitchDeg;
	_curPitchDeg = newPitch;

	FQuat pitchQuat = FQuat(worldRight, FMath::DegreesToRadians(deltaPitch));
	quat = pitchQuat * quat; // Pitch 먼저 적용

	FVector localUp = quat.GetUpVector(); // Pitch가 적용된 후의 로컬 Up 벡터(자전축)를 구함
	FQuat yawQuat = FQuat(localUp, FMath::DegreesToRadians(delta.X));
	quat = yawQuat * quat; // Yaw 적용

	quat.Normalize();
	_mesh->SetWorldRotation(quat);
}

void AGalacticPlanetGlobe::StartInteracting()
{
	_isInteracting = true;
}

void AGalacticPlanetGlobe::StopInteracting()
{
	_isInteracting = false;
}

FVector AGalacticPlanetGlobe::CalculateGlobePosition(float latitude, float longitude, float globeRadius)
{
    float lat = FMath::DegreesToRadians(latitude);
    float lon = FMath::DegreesToRadians(longitude);
    return { globeRadius * FMath::Cos(lat) * FMath::Cos(lon),
             globeRadius * FMath::Cos(lat) * FMath::Sin(lon),
             globeRadius * FMath::Sin(lat) };
}

