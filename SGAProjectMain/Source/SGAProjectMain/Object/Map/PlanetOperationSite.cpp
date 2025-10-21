// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanetOperationSite.h"

#include "Components/DecalComponent.h"
#include "../../Data/OperationDataAsset.h"
#include "../../Data/MissionDataAsset.h"

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
    
    static ConstructorHelpers::FClassFinder<APlanetMissionIcon> iconBP(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Object/Map/GalacticPlanet/BP_PlanetMissionIcon.BP_PlanetMissionIcon_C'"));
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

    ShowMissionIcons(true);
}

// Called when the game starts or when spawned
void APlanetOperationSite::BeginPlay()
{
	Super::BeginPlay();

    if (_operationData)
    {
		const auto missions = _operationData->GetMissions();
        const int32 missionCount = missions.Num();

        // 최대 3개 제한
        _bUseA = missionCount >= 1;
        _bUseB = missionCount >= 2;
        _bUseC = missionCount >= 3;

        auto InitIcon = [](UChildActorComponent* iconComp, UMissionDataAsset* missionData)
            {
                if (!iconComp) return;
                if (APlanetMissionIcon* icon = Cast<APlanetMissionIcon>(iconComp->GetChildActor()))
                {
                    icon->SetMissionData(missionData);
                }
			};

        // 미션 데이터 푸시
        if (_bUseA && missions.IsValidIndex(0))
            InitIcon(_iconA, missions[0]);
        if (_bUseB && missions.IsValidIndex(1))
            InitIcon(_iconB, missions[1]);
        if (_bUseC && missions.IsValidIndex(2))
            InitIcon(_iconC, missions[2]);
    }

    ShowMissionIcons(false);
}

// Called every frame
void APlanetOperationSite::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlanetOperationSite::ChangeToFocusMode()
{
    // 메인아이콘 충돌 비활성화
    if (auto icon = _mainIcon->GetChildActor())
    {
        icon->SetActorHiddenInGame(true);
        icon->SetActorEnableCollision(false);
	}
	_mainIcon->SetVisibility(false);
	
	ShowMissionIcons(true);
}

void APlanetOperationSite::ShowMissionIcons(bool bShow)
{
    auto SetVisible = [bShow](UChildActorComponent* iconComp, bool bUse)
        {
            if (!iconComp) return;
            if (AActor* icon = iconComp->GetChildActor())
            {
                bool visible = bShow && bUse;
                icon->SetActorHiddenInGame(!visible);
                icon->SetActorEnableCollision(visible);
                iconComp->SetVisibility(visible, true);
            }
        };

    SetVisible(_iconA, _bUseA);
    SetVisible(_iconB, _bUseB);
    SetVisible(_iconC, _bUseC);
}

