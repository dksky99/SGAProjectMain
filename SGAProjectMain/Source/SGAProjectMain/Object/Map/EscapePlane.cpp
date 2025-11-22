// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapePlane.h"

#include "Components/BoxComponent.h"
#include "../../Character/HellDiver/HellDiver.h"
#include "../../MainGameMode.h"
#include "PlaneAnimInstance.h"

// Sets default values
AEscapePlane::AEscapePlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    _planeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PlaneMesh"));
    RootComponent = _planeMesh;

    _escapeTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTriggerBox"));
    _escapeTriggerBox->SetupAttachment(_planeMesh);
}

// Called when the game starts or when spawned
void AEscapePlane::BeginPlay()
{
	Super::BeginPlay();
	
    _escapeTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 초기엔 충돌을 꺼둠
    _escapeTriggerBox->SetGenerateOverlapEvents(false);
    _escapeTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEscapePlane::OnTriggerBoxOverlap);

    if (UPlaneAnimInstance* animInst = Cast<UPlaneAnimInstance>(_planeMesh->GetAnimInstance()))
    {
        animInst->PlayMontageFromSection(_planeMontage, "Landing");
    }
}

void AEscapePlane::EnableTriggerBox()   
{
    _escapeTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    _escapeTriggerBox->SetGenerateOverlapEvents(true);
    _isEscapeEnabled = true;

    // 제한 시간 경과로 호출되었다면 20초 후 이륙 시작
    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM && GM->IsTimeOver())
		StartTimerToTakeOff();
}

void AEscapePlane::StartTimerToTakeOff()
{
    if (GetWorldTimerManager().IsTimerActive(_takeOffTimerHandle)) return;
    GetWorldTimerManager().SetTimer(_takeOffTimerHandle, this, &AEscapePlane::TakeOff, 20.0f, false);
}

void AEscapePlane::OnTriggerBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!_isEscapeEnabled) return;

    AHellDiver* hellDiver = Cast<AHellDiver>(OtherActor);
    if (!hellDiver) return;
	if (_extractedHellDivers.Contains(hellDiver)) return; // 이미 탑승한 헬다이버

    _helldiverExtractEvent.Broadcast();
	_extractedHellDivers.Add(hellDiver);

	StartTimerToTakeOff(); // 1명이라도 탑승하면 이륙 타이머 시작
}

void AEscapePlane::TakeOff()
{
    if (UPlaneAnimInstance* animInst = Cast<UPlaneAnimInstance>(_planeMesh->GetAnimInstance()))
    {
        animInst->PlayMontageFromSection(_planeMontage, "TakeOff");
    }

    _isEscapeEnabled = false;
}

