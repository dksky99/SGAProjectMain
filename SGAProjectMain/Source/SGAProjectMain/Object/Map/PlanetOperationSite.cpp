// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetOperationSite.h"

#include "Components/DecalComponent.h"

// Sets default values
APlanetOperationSite::APlanetOperationSite()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_regionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RegionDecal"));
	RootComponent = _regionDecal;

    IconA = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconA"));
    IconB = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconB"));
    IconC = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconC"));
    
    static ConstructorHelpers::FClassFinder<APlanetObjectiveIcon> iconBP(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Object/Map/GalacticPlanet/BP_PlanetObjIcon.BP_PlanetObjIcon_C'"));
    if (iconBP.Succeeded())
    {
        IconA->SetChildActorClass(iconBP.Class);
		IconB->SetChildActorClass(iconBP.Class);
		IconC->SetChildActorClass(iconBP.Class);
    }
   
    IconA->SetupAttachment(RootComponent);
    IconB->SetupAttachment(RootComponent);
    IconC->SetupAttachment(RootComponent);
}

void APlanetOperationSite::OnConstruction(const FTransform& Xform)
{
	Super::OnConstruction(Xform);

    if (IconA) IconA->SetVisibility(bUseA, true);
    if (IconB) IconB->SetVisibility(bUseB, true);
    if (IconC) IconC->SetVisibility(bUseC, true);
}

// Called when the game starts or when spawned
void APlanetOperationSite::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanetOperationSite::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

