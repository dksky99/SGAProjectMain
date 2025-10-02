// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_AttackEnd.h"
#include "../Character/CharacterBase.h"
#include "../Character/CharacterStateComponent.h"

FString UAN_AttackEnd::GetNotifyName_Implementation() const
{
	return "EndAttack";
}

void UAN_AttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp == nullptr) return;
    if (MeshComp->GetOwner() == nullptr) return;

    ACharacterBase* unit = Cast<ACharacterBase>(MeshComp->GetOwner());

    if (unit == nullptr) return;

    if (unit->_reservedFunction.IsBound())
        unit->_reservedFunction.Unbind();
    unit->ReleaseMeleeColision();
    unit->GetStateComponent()->ActionEnd();
}
