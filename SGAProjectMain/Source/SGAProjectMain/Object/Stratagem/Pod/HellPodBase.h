// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HellPodBase.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellPodBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHellPodBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float deltaSeconds) override;

	// 체력 처리
	virtual float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent, class AController* eventInstigator, AActor* damageCauser) override;

	// 아이템 스폰 + 소켓 부착(개수/소켓명은 가상함수로 해결)
	void SpawnAndAttachItems();

	// 개별 아이템 스폰 + 부착
	AItemBase* SpawnAndAttachOne(const FName& socketName, TSubclassOf<AItemBase> itemClass);

	// 남은 아이템 전부 바닥으로 (Detach + 물리/충돌 활성화)
	void DropAllItemsToGround();

	// 아이템의 파직스 끄기/켜기
	void ToggleItemPhysics(AActor* itemActor);

	// 열린 상태/열림/닫힘 몽타주(필요한 것만 세팅)
	void PlayMontage(UAnimMontage* montage);

	// 모두 소진(픽업) 시의 베이스 동작: 파생에서 원하는 연출로 오버라이드
	virtual void OnAllItemsConsumed();

	// 파괴 직전(HP 0.0f) 동작: 기본은 전부 드랍 후 즉시 소멸, 필요하면 오버라이드
	virtual void OnPodDestroyed();

	// 소켓 개수/소켓 이름 규칙(파생에서 오버라이드)
	virtual int32 GetSocketCount() const { return _maxItemCount; }
	virtual FName MakeSocketName(int32 indexBased);

	// 애님 노티파이
	UFUNCTION()
	void OnMontageNotifyBegin(FName notifyName, const FBranchingPointNotifyPayload& payload);

private:
	// 아이템이 사라지기 직전 알림(AItemBase::_onPreDespawn) 콜백
	UFUNCTION()
	void OnItemPreDespawned(class AItemBase* item);

	// 내부 소멸 처리
	void DestroySelf();

protected:
	
	// 본체 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	USkeletalMeshComponent* _mesh;

	// 메시 높이
	float _meshHeight;

	// 목표 높이
	float _targetHeight;

	// 높이 오차
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	float _riseStopToleranceZ;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	float _maxHp = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	float _currentHp = 0.0f;

	// 몽타주(필요한 것만 세팅, 없어도 동작)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	UAnimMontage* _openMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	UAnimMontage* _closeMontage;

	// 스폰할 아이템 클래스(AItemBase 기반)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	TSubclassOf<AItemBase> _itemClass;

	// 스폰할 아이템 클래스 2번 없으면 널
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	TSubclassOf<AItemBase> _ItemClass2 = nullptr;

	// 부착된 아이템 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	TArray<AItemBase*> _items;

	// 파직스가 켜져있나
	bool _isPhysicsOn = false;

	// 소켓 네임
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	FName _socketName;

	// 아이템 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stratagem/Pod")
	int32 _maxItemCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Pod")
	int32 _remainingItems = 0;
};
