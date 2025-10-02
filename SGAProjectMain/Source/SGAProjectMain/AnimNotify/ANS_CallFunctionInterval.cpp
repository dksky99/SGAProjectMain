// Fill out your copyright notice in the Description page of Project Settings.


#include "ANS_CallFunctionInterval.h"

FString UANS_CallFunctionInterval::GetNotifyName_Implementation() const
{
	return FString();
}

void UANS_CallFunctionInterval::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
}

void UANS_CallFunctionInterval::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
}

void UANS_CallFunctionInterval::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
}
