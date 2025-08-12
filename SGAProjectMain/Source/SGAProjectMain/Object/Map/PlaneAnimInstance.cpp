// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaneAnimInstance.h"

#include "EscapePlane.h"
#include "../../MainGameMode.h"
#include "Kismet/GameplayStatics.h"

void UPlaneAnimInstance::PlayMontageFromSection(UAnimMontage* montage, FName sectionName)
{
	if (!montage) return;

	Montage_Play(montage);	// 몽타주 재생
	Montage_JumpToSection(sectionName, montage); // 특정 섹션으로 점프
}

void UPlaneAnimInstance::AnimNotify_OpenHatch()
{
	if (auto actor = GetOwningActor())
	{
		auto plane = Cast<AEscapePlane>(actor);
		plane->EnableTriggerBox(); // 착륙 후 트리거 박스 활성화
	}
}

void UPlaneAnimInstance::AnimNotify_Exit()
{
	AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
	GM->OnBattleEnd();
}
