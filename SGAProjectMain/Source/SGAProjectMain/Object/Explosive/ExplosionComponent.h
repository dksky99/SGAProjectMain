// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../CDamageType.h"
#include "ExplosionComponent.generated.h"


UCLASS( ClassGroup=(Custom))
class SGAPROJECTMAIN_API UExplosionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UExplosionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 폭발 실행 트리거 (외부에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Game/Explosion")
	void Explode(FVector explosionCenter = FVector::ZeroVector, bool canDamageOwner = true);

protected:
	// 폭발 감지 및 이펙트 재생
	void HandleExplosion(const FVector& centerLocation, bool canDamageOwner);

	// 실제 폭발 데미지 감쇠
	void ApplyDamageToOverlaps(const TArray<FOverlapResult>& Overlaps, const FVector& centerLocation, bool canDamageOwner);

protected:
	// 최대 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Explosion")
	float _damage = 100.0f;

	// 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Explosion")
	float _radius = 300.0f;

	// 폭발 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Explosion")
	UParticleSystem* _effect = nullptr;

	// 폭발 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Projectile")
	USoundBase* _sound = nullptr;
};
