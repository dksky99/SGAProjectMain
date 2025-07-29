// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapePlane.h"

#include "Components/BoxComponent.h"
#include "../MainGameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEscapePlane::AEscapePlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    _planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
    RootComponent = _planeMesh;

    _escapeTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTriggerBox"));
    _escapeTriggerBox->SetupAttachment(_planeMesh);
    //_escapeTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // ÃÊ±â¿£ Ãæµ¹À» ²¨µÒ
    //_escapeTriggerBox->SetGenerateOverlapEvents(false);

    //Test
    _escapeTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    _escapeTriggerBox->SetGenerateOverlapEvents(true);
    _isEscapeEnabled = true;

}

// Called when the game starts or when spawned
void AEscapePlane::BeginPlay()
{
	Super::BeginPlay();
	
    _escapeTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEscapePlane::OnDoorOverlap);
}

void AEscapePlane::OnDoorOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!_isEscapeEnabled) return;

    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    GM->OnBattleEnd();
}

// Called every frame
void AEscapePlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

