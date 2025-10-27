// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "DestructableObject.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ADestructableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// 데미지 처리 함수 (예시)
	UFUNCTION()
	void HandleDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	// 장애물을 파괴하고 내비게이션을 업데이트하는 함수
	void DestroyObstacle();
protected:
	// Sets default values for this actor's properties
	ADestructableObject();
	// 시작 시 호출
	virtual void BeginPlay() override;
public:
	// 파괴 전 내비게이션 영역 클래스 (AI 이동 방지용)
	UPROPERTY(EditAnywhere, Category = "Navigation")
	TSubclassOf<UNavArea> PreDestructionNavArea;

	// 파괴 후 내비게이션 영역 클래스 (AI 이동 허용용, nullptr 또는 NavArea_Default 권장)
	UPROPERTY(EditAnywhere, Category = "Navigation")
	TSubclassOf<UNavArea> PostDestructionNavArea;
protected:

	// 파괴되었는지 여부
	bool bIsDestroyed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;
	// 파괴되기 전 AI 이동을 막는 충돌 영역
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* CollisionComponent;

	// 이 장애물을 부수기 위해 필요한 파괴력입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction")
	float RequiredDestructionPower = 50.0f;
	// 장애물이 견딜 수 있는 총 데미지
	UPROPERTY(EditAnywhere, Category = "Barrier Stats")
	float Health;

};
