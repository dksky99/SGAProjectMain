// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Interactable.generated.h"

UENUM(BlueprintType)
enum class EInteractableIconType : uint8
{
	None,
	Gun,
	Sample,
	SupplyBox
};

USTRUCT(BlueprintType)
struct FInteractableInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInteractableIconType _type = EInteractableIconType::None; // 표시되는 아이템 종류

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText _text; // 상호작용 마크 옆에 뜨는 글씨
};

UCLASS()
class SGAPROJECTMAIN_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();

	virtual void Interact(class AHellDiver* player) {};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void ShowDefaultMark();
	virtual void ShowKeyButtonMark();
	void HideMark();

protected:
	UPROPERTY(EditAnywhere, Category = "Game/Mesh")
	UStaticMeshComponent* _mesh;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	class UWidgetComponent* _interactionMark;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	FInteractableInfo _interactableInfo; // UI용
};
