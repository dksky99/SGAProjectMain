// Fill out your copyright notice in the Description page of Project Settings.


#include "Backpack.h"

#include "../../Character/HellDiver/HellDiver.h"

ABackpack::ABackpack()
{
	_mesh->SetCollisionProfileName("PhysicsActor");
	_mesh->SetGenerateOverlapEvents(true);
	_mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void ABackpack::PickupItem(AHellDiver* player)
{
	player->EquipBackpack(this);
	SetOwner(player);

    if (USkeletalMeshComponent* characterMesh = player->GetMesh())
    {
        _mesh->SetEnableGravity(false);
        _mesh->SetSimulatePhysics(false);
        _mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("cc_backpack_socket"));
    }
}
