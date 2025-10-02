// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SupplyHellpod.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ASupplyHellpod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASupplyHellpod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 외부에서 데미지 적용(프로젝트에서 Damage 시스템을 쓰면 OnTakeAnyDamage 바인딩 권장)
	UFUNCTION(BlueprintCallable, Category = "Game/Stratagem/Health")
	void ApplyDamage(float damageAmount);

	// 헬 포드가 소켓에 보급 상자 4개를 스폰 및 부착
	void SpawnAndAttachSupplies();

	// 헬 포드 파괴 시 바닥으로 떨어뜨리기(부착 해제 + 물리 활성화)
	void DropAllSuppliesToGround();

	// 소환 시 몽타주 재생
	void PlaySpawnMontage();

	// 전부 가져가면 몽타주 재생 후 소멸
	void PlayDespawnAndDestroy();

	// 개별 상자 스폰 + 부착
	class ASupplyBox* SpawnAndAttachOne(const FName& socketName);

	// -------------------------------------------------------
private:
	// 아이템이 사라지기 직전 신호를 받을 콜백(AItemBase::_onPreDespawn 바인딩 대상)
	UFUNCTION()
	void OnItemPreDespawned(class AItemBase* item);

	// 내부 소멸 처리(몽타주가 없거나 종료 시점에 호출)
	void DestroySelf();

	// 몽타주 길이만큼 딜레이 후 DestroySelf
	void DestroyAfterMontage(UAnimMontage* montage);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	USkeletalMeshComponent* _mesh;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Health")
	float _maxHp = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Health")
	float _currentHp = 0.0f;

	// 소환 시 재생할 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Anim")
	UAnimMontage* _spawnMontage;

	// 전부 가져가면 재생할 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Anim")
	UAnimMontage* _despawnMontage;

	// 스폰할 보급 상자 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	TSubclassOf<ASupplyBox> _supplyBoxClass;

	// 부착 대상 소켓 이름들
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	FName _supplySocketNames;

	// 부착된 보급 상자 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	TArray<ASupplyBox*> _items;

	// 남은 상자 개수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Supply")
	int32 _remainingItems = 0;
};
