// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Item/ItemBase.h"
#include "TerminalConsole.generated.h"

/**
 * 
 */
DECLARE_MULTICAST_DELEGATE(FMissionCompleted);

UCLASS()
class SGAPROJECTMAIN_API ATerminalConsole : public AItemBase
{
	GENERATED_BODY()
	
public:
	ATerminalConsole();

	void BeginPlay();

	virtual void PickupItem(class AHellDiver* hellDiver);
	void ReceiveInput(FKey key);

	FMissionCompleted _missionCompletedEvent;

	void SetInteractable(bool isInteractable);

protected:
	void ResetTerminalConsole();
	
	void OnTaskCompleted();

protected:
	// 위젯
	UPROPERTY(EditAnywhere, Category = "Game/Command")
	class UWidgetComponent* _terminalWidgetComponent;
	UPROPERTY()
	class UUserWidget* _terminalWidget;

	// 현재 수행중인 작업
	UPROPERTY(EditAnywhere, Category = "Terminal Tasks") // 추후 변경
	class UTerminalTaskBase* _curTask = nullptr;

	UPROPERTY()
	class APlayerCharacter* _player;

	UPROPERTY(EditAnywhere, Category = "Game/Console")
	bool _isInteractable = true;

	UPROPERTY(EditAnywhere, Category = "Game/Command")
	class UWidgetComponent* _interactionMark;
};
