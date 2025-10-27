// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interactable.h"
#include "GalacticPlanetGlobe.generated.h"

UENUM(BlueprintType)
enum class EPlanetGlobeMode : uint8
{
	None,
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

protected:
	void StartInteracting();
	void StopInteracting();
	void EnterFocus();
	void ExitFocus();
	void SelectMission();
	void SetCameraView(EPlanetGlobeMode mode);

	void TickBrowseMode(float DeltaTime);
	void TickFocusMode(float DeltaTime);

	FQuat CalculateNewGlobeQuat(float deltaYaw, float deltaPitch, FQuat baseQuat);
	FVector CalculateGlobePosition(float latitude, float longitude, float globeRadius);
	
	UFUNCTION()
	void OnFocusTimelineUpdate(float value);
	UFUNCTION()
	void OnExitTimelineUpdate(float value);
	UFUNCTION()
	void OnFocusTimelineFinished();

	UFUNCTION()
	void OnIconInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnIconOutOfRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	UFUNCTION()
	void OnSelect(const FInputActionValue& value);
	UFUNCTION()
	void OnBack(const FInputActionValue& value);

protected:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* _root = nullptr;
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* _globeRoot = nullptr;

	UPROPERTY(EditAnywhere, Category = "Game/Camera")
	class UChildActorComponent* _browseCamera;
	UPROPERTY(EditAnywhere, Category = "Game/Camera")
	class UChildActorComponent* _focusCamera;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* _rotatingBand;

	UPROPERTY(EditAnywhere)
	USceneComponent* _playerAnchor;

	UPROPERTY(EditAnywhere, Category = "Game/UI")
	TSubclassOf<UUserWidget> _globeWidgetClass;
	UPROPERTY()
	class UPlanetGlobeWidget* _globeWidget;

	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _gameIMC;
	UPROPERTY(EditAnywhere, Category = "Game/IMC")
	class UInputMappingContext* _globeWidgetIMC;

	UPROPERTY(EditAnywhere, Category = "Game/Input")
	class UInputAction* _selectAction;
	UPROPERTY(EditAnywhere, Category = "Game/Input")
	class UInputAction* _backAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _globeRadius = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _siteZoneRadius = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _ringSensitivity = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _globeSensitivity = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FOperationData> _operations;

	UPROPERTY(VisibleAnywhere)
	TArray<UChildActorComponent*> _operationSites;

	UPROPERTY(EditAnywhere, Category = "Game/Timeline")
	UCurveFloat* _timelineCurve;
	UPROPERTY()
	class UTimelineComponent* _focusTimeline;
	UPROPERTY()
	class UTimelineComponent* _exitTimeline;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class APlanetSelectRing> _ringClass;
	UPROPERTY()
	class APlanetSelectRing* _ring;

	UPROPERTY()
	APlayerController* _playerController;
	UPROPERTY()
	AActor* _playerViewTarget;

	UPROPERTY()
	class APlanetOperationSite* _curSite = nullptr;
	UPROPERTY()
	class APlanetMissionIcon* _curIcon = nullptr;

	EPlanetGlobeMode _mode = EPlanetGlobeMode::Browse;
	bool _isInteracting = false;

	FQuat _startGlobeQuat, _targetGlobeQuat;
	FVector _startGlobeLoc, _targetGlobeLoc;
	FVector _startBandLoc, _targetBandLoc;

	float _curPitchDeg = 0.f;	// 현재 글로브 피치 각도
	FVector2D _focusedSiteAnchor;	// 선택 지점을 앵커로 설정한 스크린 좌표
	FVector2D _ringScreenPos;   // 링의 현재 스크린 좌표
	FQuat _focusedGlobeRotation;   // 글로브 기본 회전 상태
};
