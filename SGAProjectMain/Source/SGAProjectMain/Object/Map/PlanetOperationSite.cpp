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

	_mainIcon = CreateDefaultSubobject<UChildActorComponent>(TEXT("MainIcon"));
    _iconA = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconA"));
    _iconB = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconB"));
    _iconC = CreateDefaultSubobject<UChildActorComponent>(TEXT("IconC"));
    
    static ConstructorHelpers::FClassFinder<APlanetObjectiveIcon> iconBP(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Object/Map/GalacticPlanet/BP_PlanetObjIcon.BP_PlanetObjIcon_C'"));
    if (iconBP.Succeeded())
    {
		_mainIcon->SetChildActorClass(iconBP.Class);
        _iconA->SetChildActorClass(iconBP.Class);
		_iconB->SetChildActorClass(iconBP.Class);
		_iconC->SetChildActorClass(iconBP.Class);
    }
   
	_mainIcon->SetupAttachment(RootComponent);
    _iconA->SetupAttachment(RootComponent);
    _iconB->SetupAttachment(RootComponent);
    _iconC->SetupAttachment(RootComponent);
}

void APlanetOperationSite::OnConstruction(const FTransform& Xform)
{
	Super::OnConstruction(Xform);

    if (_iconA) _iconA->SetVisibility(bUseA, true);
    if (_iconB) _iconB->SetVisibility(bUseB, true);
    if (_iconC) _iconC->SetVisibility(bUseC, true);
}

// Called when the game starts or when spawned
void APlanetOperationSite::BeginPlay()
{
	Super::BeginPlay();

    _iconA->SetVisibility(false);
    _iconB->SetVisibility(false);
    _iconC->SetVisibility(false);
	
}

// Called every frame
void APlanetOperationSite::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

