// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapIconActor.h"

#include "EngineUtils.h"
#include "SceneCapturer.h"

// Sets default values
AMinimapIconActor::AMinimapIconActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MapIconMesh"));
	RootComponent = _mesh;
}

// Called when the game starts or when spawned
void AMinimapIconActor::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AMinimapIconActor::FindSceneCapturer);
}

// Called every frame
void AMinimapIconActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_sceneCapturer)
	{
		float orthoWidth = _sceneCapturer->GetCurOrthoWidth();
		float baseWidth = 512.0f;
		float scale = orthoWidth / baseWidth;
		_mesh->SetWorldScale3D(FVector(scale));
	}
}

void AMinimapIconActor::FindSceneCapturer()
{
	for (TActorIterator<ASceneCapturer> IT(GetWorld()); IT; ++IT)
	{
		ASceneCapturer* sceneCapturer = *IT;
		if (sceneCapturer)
		{
			_sceneCapturer = sceneCapturer;
			break;
		}
	}
}

