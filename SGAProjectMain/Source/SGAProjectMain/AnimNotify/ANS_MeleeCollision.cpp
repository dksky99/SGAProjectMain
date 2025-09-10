// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_MeleeCollision.h"
#include "../Character/CharacterBase.h"

FString UANS_MeleeCollision::GetNotifyName_Implementation() const
{
    return "MeleeCollision";
}

void UANS_MeleeCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (MeshComp == nullptr) return;
    if (MeshComp->GetOwner() == nullptr) return;

    ACharacterBase* unit = Cast<ACharacterBase>(MeshComp->GetOwner());

    if (unit == nullptr) return;

    unit->ActivateMeleeColision();
}

void UANS_MeleeCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (MeshComp == nullptr) return;
    if (MeshComp->GetOwner() == nullptr) return;

    ACharacterBase* unit = Cast<ACharacterBase>(MeshComp->GetOwner());

    if (unit == nullptr) return;

    unit->DeactivateMeleeColision();
}
