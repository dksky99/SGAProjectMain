// Fill out your copyright notice in the Description page of Project Settings.


#include "H_CharacterLoc.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

void H_CharacterLoc::SetCharacterToGround(ACharacter* player, USceneComponent* anchor, UWorld* world)
{
	if (!player || !anchor || !world) return;

	UCapsuleComponent* capsule = player->GetCapsuleComponent();
	if (!capsule) return;
	float halfHeight = capsule->GetScaledCapsuleHalfHeight();

	// 앵커 위→아래로 레이 쏴서 바닥 찾기
	FVector start = anchor->GetComponentLocation() + FVector(0, 0, 200.f);
	FVector end = anchor->GetComponentLocation() - FVector(0, 0, 5000.f);

	FHitResult hit;
	FCollisionQueryParams param;
	param.AddIgnoredActor(player);

	bool bHit = world->LineTraceSingleByChannel(hit, start, end, ECC_Visibility, param);

	// 맞으면 그 지점으로 이동
	FVector newLoc = anchor->GetComponentLocation();
	if (bHit) newLoc.Z = hit.ImpactPoint.Z + halfHeight;
	player->SetActorLocation(newLoc);
}