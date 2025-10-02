// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Hellpod.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellpod : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHellpod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	UFUNCTION()
	void Move(const struct FInputActionValue& value);


	//투하 전에 헬포드에 헬다이버를 집어넣는다.
	void AttachHelldiverToHellpod(class APlayerCharacter* helldiver);
	//지표면에 도착 후 헬다이버에게 컨트롤러를 넘긴다.
	void DettachHelldiverToHellpod();


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	USkeletalMeshComponent* _mesh;
	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	class UCameraComponent* _camera;

	UPROPERTY(VisibleAnywhere, Category = "Game/Hellpod")
	class UProjectileMovementComponent* _projectile;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Hellpod")
	float _damage = 100.f;

	UPROPERTY()
	class APlayerCharacter* _helldiver;

	// 중복 실행 방지
	bool _isAlreadySpawned = false;

	// 바닥 충돌 여부
	bool _isGrounded = false;

	FVector _targetLocation;
	float _riseSpeed = 300.f; // 초당 300cm 상승

	// 타이머 (올라오기 지연)
	FTimerHandle _riseTimerHandle;

	// 충돌한 캐릭터 저장 (중복 데미지 방지)
	TSet<AActor*> _damagedCharacters;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _moveAction;
};
