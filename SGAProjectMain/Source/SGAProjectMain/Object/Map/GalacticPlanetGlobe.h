// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interactable.h"
#include "GalacticPlanetGlobe.generated.h"

UENUM(BlueprintType)
enum class EPlanetGlobeMode : uint8
{
	Browse,
	Focus
};

USTRUCT(BlueprintType)
struct FOperationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _latitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _longitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class APlanetOperationSite> OperationSiteClass;
};

UCLASS()
class SGAPROJECTMAIN_API AGalacticPlanetGlobe : public AInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGalacticPlanetGlobe();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitializeOperations();
	void SetPlayerInputComponent();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact(class AHellDiver* player) override;

	void StartInteracting();
	void StopInteracting();

protected:
	UFUNCTION()
	void OnIconInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnIconOutOfRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input")
	class UInputAction* _selectAction;

	UFUNCTION()
	void OnSelect(const FInputActionValue& value);

	void EnterFocus(class APlanetOperationSite* Icon);

	void TickBrowseMode(float DeltaTime);
	void TickFocusMode(float DeltaTime);

	void RotateGlobe(float deltaX, float deltaY);
	FVector CalculateGlobePosition(float latitude, float longitude, float globeRadius);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> _globeWidgetClass;
	UPROPERTY()
	class UPlanetGlobeWidget* _globeWidget;

	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _gameIMC;
	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _globeWidgetIMC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _globeRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOperationData> _operations;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> _operationSites;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APlanetSelectRing> _ringClass;
	UPROPERTY(EditAnywhere)
	class APlanetSelectRing* _ring;

	UPROPERTY()
	APlayerController* _playerController;

	EPlanetGlobeMode _mode = EPlanetGlobeMode::Browse;
	bool _isInteracting = false;

	float _curPitchDeg = 0.f;

	FVector _curSiteLoc;		// 선택 지점(표면 포인트)
	FVector2D _ringScreenPos;   // 링의 현재 스크린 좌표
	FVector2D _ringVel;         // 링 스크린 속도
	FQuat _targetGlobeRotation; // 글로브 목표 회전(선택 지점이 정면으로 오게)

	UPROPERTY()
	class APlanetOperationSite* _curSite;
	UPROPERTY()
	class APlanetObjectiveIcon* _curIcon;
};
