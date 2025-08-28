// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../Character/HellDiver/HellDiver.h"
#include "ItemBase.generated.h"

// 아이템이 제거되기 직전에 알림을 보내는 델리게이트 (블루프린트에서도 바인딩 가능)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPreDespawn, class AItemBase*, item);

UCLASS()
class SGAPROJECTMAIN_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void PickupItem(class AHellDiver* player);


	UPROPERTY(BlueprintAssignable, Category = "Game/Item")
	FOnItemPreDespawn _onPreDespawn;

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Item")
	UStaticMeshComponent* _mesh;
};
