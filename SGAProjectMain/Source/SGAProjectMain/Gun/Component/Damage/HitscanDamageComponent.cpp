// Fill out your copyright notice in the Description page of Project Settings.


#include "HitscanDamageComponent.h"

#include "Kismet/GameplayStatics.h"
#include "../../../SGAProjectMain.h"
#include "../../../Character/CharacterBase.h"
#include "../../GunBase.h"

void UHitscanDamageComponent::DoFireShot(FVector fireLocation, FVector fireDirection)
{
	FColor drawColor = FColor::Green; // 디버깅용

	FVector end = fireLocation + fireDirection * 10000.f;

	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(
		hitResult,
		fireLocation,
		end,
		ECC_GameDamage);

	if (hitResult.bBlockingHit)
	{
		drawColor = FColor::Red;
		float distance = FVector::Dist(hitResult.TraceStart, hitResult.ImpactPoint);
		float finalDamage = CalculateDamage(distance / 100.f);

		if (finalDamage < 0) return;

		if (ACharacterBase* character = Cast<ACharacterBase>(hitResult.GetActor()))
		{
			//UGameplayStatics::ApplyDamage(character, finalDamage, _owner->GetController(), this, nullptr);

			const FVector shotDirection = (hitResult.TraceStart - hitResult.ImpactPoint).GetSafeNormal();
			UGameplayStatics::ApplyPointDamage(
				character,										// 데미지를 받을 액터
				finalDamage,									// 적용할 데미지 값
				shotDirection,									// 데미지가 들어온 방향(단위 벡터)
				hitResult,										// 충돌에 대한 자세한 정보(FHitResult)
				_gun->GetOwnerCharacter()->GetController(),     // 데미지를 일으킨 컨트롤러(Instigator)
				_gun,											// 데미지를 발생시킨 액터(Damage Causer)
				UDamageType::StaticClass()						// 사용할 데미지 타입 클래스
			);
		}
	}

	FVector hitPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	DrawDebugLine(GetWorld(), hitResult.TraceStart, hitPoint, drawColor, false, 1.0f);
}
