// Fill out your copyright notice in the Description page of Project Settings.


#include "ImpactMarker.h"

#include "Components/WidgetComponent.h"

// Sets default values
AImpactMarker::AImpactMarker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_markerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("MarkerWidget"));
	RootComponent = _markerWidget;
	//_markerWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AImpactMarker::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AImpactMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AImpactMarker::SetMarkerLocation(const FVector& location)
{
	SetActorLocation(location);
}
