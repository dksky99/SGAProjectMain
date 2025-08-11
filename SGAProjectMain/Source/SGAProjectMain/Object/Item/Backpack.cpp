// Fill out your copyright notice in the Description page of Project Settings.


#include "Backpack.h"

#include "../../Character/HellDiver/HellDiver.h"

ABackpack::ABackpack()
{
    _skeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    _skeletalMesh->SetSimulatePhysics(true);
    RootComponent = _skeletalMesh;

    _skeletalMesh->SetCollisionProfileName("PhysicsActor");
    _skeletalMesh->SetGenerateOverlapEvents(true);
    _skeletalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void ABackpack::PickupItem(AHellDiver* player)
{
	player->EquipBackpack(this);
	SetOwner(player);

    if (USkeletalMeshComponent* characterMesh = player->GetMesh())
    {
        _skeletalMesh->SetEnableGravity(false);
        _skeletalMesh->SetSimulatePhysics(false);
        _skeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("EquipSocket_BackPack"));
        _skeletalMesh->SetRelativeRotation(FRotator(-90.f, 0.f, 180.f));
    }
}
