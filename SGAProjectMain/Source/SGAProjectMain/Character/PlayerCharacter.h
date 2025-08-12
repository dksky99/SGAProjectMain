// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HellDiver/HellDiver.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */


UENUM()
enum class ECharacterViewType : uint8
{
	TPS,
	TPSZoom,
	FPS,
	MAX
};

UCLASS()
class SGAPROJECTMAIN_API APlayerCharacter : public AHellDiver
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FTransform GetLeftHandPos();

protected:
	// Called when the game starts or when spawned

public:
	// Called every frame

	UFUNCTION()
	void Move(const  FInputActionValue& value);

	UFUNCTION()
	void MoveFinish(const  FInputActionValue& value);
	UFUNCTION()
	void Look(const  FInputActionValue& value);
	UFUNCTION()
	void TryPakour(const  FInputActionValue& value);
	UFUNCTION()
	void StartFiring(const  FInputActionValue& value);
	UFUNCTION()
	void WhileFiring(const  FInputActionValue& value);
	UFUNCTION()
	void StopFiring(const  FInputActionValue& value);
	UFUNCTION()
	void StartAiming(const  FInputActionValue& value);
	UFUNCTION()
	void WhileAiming(const  FInputActionValue& value);
	UFUNCTION()
	void StopAiming(const  FInputActionValue& value);
	UFUNCTION()
	void HoldReload(const  FInputActionValue& value);
	UFUNCTION()
	void ReleaseReload(const  FInputActionValue& value);

	void EnterGunSetting();

	UFUNCTION()
	void TryChangeFireMode(const  FInputActionValue& value);
	UFUNCTION()
	void TryChangeLightMode(const  FInputActionValue& value);
	UFUNCTION()
	void TryChangeScopeMode(const  FInputActionValue& value);

	UFUNCTION()
	void ChangeAimingView(const  FInputActionValue& value);

	UFUNCTION()
	void TrySprint(const  FInputActionValue& value);

	UFUNCTION()
	void StopSprint(const  FInputActionValue& value);
	


	UFUNCTION()
	void TryCrouch(const  FInputActionValue& value);

	UFUNCTION()
	void TryChangeControl(const  FInputActionValue& value);

	UFUNCTION()
	void TryProne(const  FInputActionValue& value);

	UFUNCTION()
	void TryRolling(const  FInputActionValue& value);

	UFUNCTION()
	void SwitchWeapon1(const  FInputActionValue& value) { SwitchGun(0, value); }
	UFUNCTION()
	void SwitchWeapon2(const  FInputActionValue& value) { SwitchGun(1, value); }
	UFUNCTION()
	void SwitchWeapon3(const  FInputActionValue& value) { SwitchGun(2, value); }

	void SwitchGun(int32 index, const FInputActionValue& value);
	void PickupGun(AGunBase* gun) override;
	void AddSample(struct FSampleBundle sample) override;
	void DropBackpack();
	

	UFUNCTION()
	void Interact(const  FInputActionValue& value);

	UFUNCTION()
	void BeginStratagemInputMode(const FInputActionValue& value);
	UFUNCTION()
	void EndStratagemInputMode(const FInputActionValue& value);
	UFUNCTION()
	void OnStrataKeyW(const FInputActionValue& value);
	UFUNCTION()
	void OnStrataKeyA(const FInputActionValue& value);
	UFUNCTION()
	void OnStrataKeyS(const FInputActionValue& value);
	UFUNCTION()
	void OnStrataKeyD(const FInputActionValue& value);
	
	UFUNCTION()
	void OnUseStimPack(const FInputActionValue& value);

	void CheckStratagemInputCombo();
	
	void BeginTerminalInputMode(class ATerminalConsole* console); // 상호작용 시작 시 콘솔에서 호출
	void EndTerminalInputMode(); // 상호작용 종료 시 콘솔에서 호출
	
	void SetViewData(const class UPlayerControlDataAsset* characterControlData);

	void FocusMove(FVector2D moveVector);
	void DefaultMove(FVector2D moveVector);

	void MovingLook();
	void DefaultLook();
	void CalcPitch();
	void CalcYaw();

	void ChangeViewCamera(ECharacterViewType type);
	void DeactiveAnotherCamera();
	UChildActorComponent* GetCurCamera();

	void SetDefaultVIew();
	void SetMovingView();
	void FinishMoving();
	void InitView();
	void SetFPSView();
	void SetTPSZoomView();
	void SetTPSView();
	virtual FRotator Focusing() override;
	virtual FRotator Focusing_Legacy() ;
	void UpdateCameraOcclusion();
	FVector GetCenterLoc();
	void ViewTurnBack();

	virtual void SetStandingCollisionCamera() override;
	virtual void SetCrouchingCollisionCamera()override;
	virtual void SetProningCollisionCamera()override;

	void SetSceneCapturer(class ASceneCapturer* capturer) { _sceneCapturer = capturer; }
	void OpenMap();

protected:



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _moveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _lookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _jumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _sprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _crouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _proneAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _rollingAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _testingViewAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _mouseLButtonAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _mouseRButtonAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _reloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _weapon1ChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _weapon2ChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _weapon3ChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _grenadeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _lightChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _scopeChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _aimChangeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _interactAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _mapAction;

	TArray<FKey> _stratagemInputBuffer;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _strataInputModeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _strataWAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _strataAAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _strataSAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _strataDAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _stimPackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _invenAction;




	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Control", meta = (AllowPrivateAccess = "true"))
	class UPlayerControlDataAsset* _defaultControl;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Control", meta = (AllowPrivateAccess = "true"))
	class UPlayerControlDataAsset* _fpsControl;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Control", meta = (AllowPrivateAccess = "true"))
	class UPlayerControlDataAsset* _tpsControl;

	ECharacterViewType _viewType = ECharacterViewType::TPS;




	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _gunWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class UGunWidget* _gunWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _stgWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class UStratagemWidget* _stratagemWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _minimapWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class UMiniMapWidget* _minimapWidget;
	UPROPERTY()
	class ASceneCapturer* _sceneCapturer;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _staminaBarWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class UStaminaBarWidget* _staminaBarWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _compassWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class UCompassWidget* _compassWidget;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _sampleWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Widget", meta = (AllowPrivateAccess = "true"))
	class USampleWidget* _sampleWidget;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* _camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* _cameraRoot;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* _tpsSpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* _tpsZoomSpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* _fpsSpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* _tpsCameraActor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* _tpsZoomCameraActor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* _fpsCameraActor;




	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	float _cameraBlendTime=0.2f;
	UPROPERTY()
	TArray<UPrimitiveComponent*> _fadedComponents;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Camera", meta = (AllowPrivateAccess = "true"))
	FVector _aimOffset_;
	// 위젯 -> 총 설정
	float _reloadPressedTime = 0.0f;
	bool _isGunSettingMode = false;

	FTimerHandle _gunSettingTimer;

	// 위젯 -> 지도보기
	bool _isDraggingMap = false;

	// 아이템 감지용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Interaction")
	class USphereComponent* _itemDetectionSphere;

	// 커맨드 콘솔
	UPROPERTY()
	ATerminalConsole* _curTerminal;
};
