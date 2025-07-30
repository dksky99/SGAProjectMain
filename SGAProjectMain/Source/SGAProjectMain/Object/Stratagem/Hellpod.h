// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hellpod.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellpod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHellpod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 상승 시작
	void StartRising();

	// 상승 완료 후 후속 처리
	void OnRiseFinished();

	// 주변 캐릭터 날리기
	void LaunchOverlappedActors(const FVector& hitPoint);

	// 내부 액터 스폰
	void SpawnInternalActor();

	void DestroySelf();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Hellpod")
	UStaticMeshComponent* _mesh;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	TSubclassOf<AActor> _hellpodToSpawn;

	UPROPERTY(VisibleAnywhere, Category = "Game/Hellpod")
	class UProjectileMovementComponent* _projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	float _damage = 100.f;

	// 중복 실행 방지
	bool isAlreadySpawned = false;

	// 바닥 충돌 여부
	bool _isGrounded = false;

	// 상승 애니메이션 플래그
	bool _isRising = false;

	FVector _targetLocation;
	float _riseSpeed = 300.f; // 초당 300cm 상승

	// 타이머 (올라오기 지연)
	FTimerHandle _riseTimerHandle;

	// 충돌한 캐릭터 저장 (중복 데미지 방지)
	TSet<AActor*> _damagedCharacters;
};
