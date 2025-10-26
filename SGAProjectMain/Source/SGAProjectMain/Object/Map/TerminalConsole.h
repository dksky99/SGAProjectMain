// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Interactable.h"
#include "Components/SceneComponent.h"
#include "TerminalConsole.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FMissionCompleted);

UCLASS()
class SGAPROJECTMAIN_API ATerminalConsole : public AInteractable
{
	GENERATED_BODY()
	
public:
	ATerminalConsole();

	void BeginPlay();

	virtual void Interact(class AHellDiver* hellDiver);
	void ReceiveInput(FKey key);

	FMissionCompleted _missionCompletedEvent;

	void ShowDefaultMark() override;
	void ShowKeyButtonMark() override;
	void SetInteractable(bool isInteractable);

protected:
	void ActivateTerminalConsole();
	void ResetTerminalConsole();

	void SetPlayerLocation();
	void ChangeCameraView(bool isInteracting);
	
	void OnTaskCompleted();

protected:
	// 위젯
	UPROPERTY(EditAnywhere, Category = "Game/UI")
	class UWidgetComponent* _terminalWidgetComponent;

	UPROPERTY()
	class UUserWidget* _terminalWidget;

	// 카메라
	UPROPERTY()
	ACameraActor* _cutInCam;
	
	UPROPERTY(VisibleAnywhere, Category = "Game/Camera")
	USceneComponent* _camAnchor; // 카메라 고정 위치

	UPROPERTY(VisibleAnywhere, Category = "Game/Camera")
	USceneComponent* _lookAt;

	UPROPERTY()
	AActor* _playerViewTarget;

	// 캐릭터
	UPROPERTY(VisibleAnywhere, Category = "Game/Camera")
	USceneComponent* _playerAnchor; // 캐릭터 고정 위치

	UPROPERTY()
	class APlayerCharacter* _player;

	UPROPERTY(EditAnywhere, Category = "Game/Console")
	bool _isInteractable = true;

	// 현재 수행중인 작업
	UPROPERTY(EditAnywhere, Category = "Terminal Tasks") // 추후 배열로 변경
	class UTerminalTaskBase* _curTask = nullptr;

};
