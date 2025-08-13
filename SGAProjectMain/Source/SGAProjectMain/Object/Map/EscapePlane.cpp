// Fill out your copyright notice in the Description page of Project Settings.


#include "EscapePlane.h"

#include "Components/BoxComponent.h"
#include "../../Character/HellDiver/HellDiver.h"
#include "PlaneAnimInstance.h"

// Sets default values
AEscapePlane::AEscapePlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

    if (UPlaneAnimInstance* AnimInst = Cast<UPlaneAnimInstance>(_planeMesh->GetAnimInstance()))
    {
        AnimInst->PlayMontageFromSection(_planeMontage, "Landing");
    }
}

void AEscapePlane::OnTriggerBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!_isEscapeEnabled) return;

    AHellDiver* hellDiver = Cast<AHellDiver>(OtherActor);
    if (!hellDiver) return;

    if (UPlaneAnimInstance* AnimInst = Cast<UPlaneAnimInstance>(_planeMesh->GetAnimInstance()))
    {
        AnimInst->PlayMontageFromSection(_planeMontage, "TakeOff");
    }

	_isEscapeEnabled = false;
}

void AEscapePlane::EnableTriggerBox()
{
    _escapeTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    _escapeTriggerBox->SetGenerateOverlapEvents(true);
    _isEscapeEnabled = true;
}

// Called every frame
void AEscapePlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    //if (_isLanding)
    //{
    //    FVector curLocation = GetActorLocation();
    //    FVector newLocation = FMath::VInterpTo(curLocation, _targetLocation, DeltaTime, 1.f);
    //    SetActorLocation(newLocation);

    //    float Distance = FVector::Dist(newLocation, _targetLocation);
    //    if (Distance <= 5.0f)
    //    {
    //        _isLanding = false;
    //        //EnableTriggerBox();  // 트리거 켜기
    //    }
    //}
}

