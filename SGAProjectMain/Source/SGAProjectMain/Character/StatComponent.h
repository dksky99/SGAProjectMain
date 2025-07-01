// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

// 부위 식별용 열거형
UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Core,
	Head,
	Torso,
	LeftArm,
	RightArm,
	LeftLeg,
	RightLeg
};

// 사망 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
// 부위 파괴 이벤트 델리게이트 (어느 부위인지 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartDestroyed, EBodyPart, Part);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	bool IsDead();

	float GetDefaultSpeed() { return _defaultSpeed; }

	// 포인트 데미지(부위별) 핸들러
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
		UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);

	// 사망 알림
	UPROPERTY(BlueprintAssignable, Category = "Game/Stat")
	FOnDeath OnDeath;

	// 부위 파괴 알림
	UPROPERTY(BlueprintAssignable, Category = "Game/Stat")
	FOnPartDestroyed OnPartDestroyed;

private:
	// 실제 HP 차감 및 이벤트 브로드캐스트
	void ProcessDamage(EBodyPart Part, float Damage);

protected:
	float _defaultSpeed = 300.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stat")
	class ACharacterBase* _owner;

	// 코어 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _coreMaxHP;
	// 코어 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _coreHP;

	// 머리 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _headMaxHP;
	// 머리 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _headHP;

	// 몸통 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _torsoMaxHP;
	// 몸통 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _torsoHP;

	// 왼팔 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _leftArmMaxHP;
	// 왼팔 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _leftArmHP;

	// 오른팔 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _rightArmMaxHP;
	// 오른팔 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _rightArmHP;

	// 왼다리 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _leftLegMaxHP;
	// 왼다리 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _leftLegHP;

	// 오른다리 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _rightLegMaxHP;
	// 오른다리 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _rightLegHP;
};
