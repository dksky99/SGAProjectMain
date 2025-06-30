// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget.h"	

#include "SceneCapturer.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ASceneCapturer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASceneCapturer();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void FilterActorList();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ResetMap(AActor* target); // 플레이어 쪽에서 자신을 넘겨줌

	void ChangeOrthoWidth(bool zoomIn);

	void StartDraggingMap();
	void StopDraggingMap();

	bool PingOnMap(); // 핑 찍기가 불가능하면 false, 가능하면 핑을 찍은 후 true 반환 

	void SetFollowTarget(AActor* target) { _curFollowTarget = target; }

private:
	UPROPERTY(EditAnywhere, Category = "Game")
	USceneCaptureComponent2D* _sceneCaptureComponent;

	UPROPERTY(EditAnywhere, Category = "Game")
	UTextureRenderTarget2D* _renderTarget;

	UPROPERTY(EditAnywhere, Category = "Game")
	TEnumAsByte<ECameraProjectionMode::Type> _projectionType;

	// 액터 필터 리스트
	UPROPERTY(EditAnywhere, Category = "Game")
	TArray<FName> _hiddenActorTagList;			// Primitive Render 모드일 때 기피 액터

	UPROPERTY(EditAnywhere, Category = "Game")
	TArray<FName> _showActorTagList;				// Show Only 모드일 때 표시 액터

	// 추적 관련 변수
	UPROPERTY()
	AActor* _curFollowTarget; // 추적 액터 (플레이어 혹은 커서)

	UPROPERTY(EditAnywhere, Category = "Game")
	TSubclassOf<AActor> _cursorActorClass;
	UPROPERTY()
	AActor* _cursorActor;

	UPROPERTY()
	APlayerController* _playerController;

	bool _isDraggingCursor = false;
	FVector2D _lastMousePos;


	float _fixedHeight = 500.f; // z축 위치

	// 맵 확대
	UPROPERTY(EditAnywhere, Category = "Game") // 맵 최대 크기
	FVector2D _maxMapSize = { 3000.f, 3000.f };

	UPROPERTY(EditAnywhere, Category = "Game") // 맵 확대 단계
	TArray<float> _orthoWidthLevel = { 500.f, 1500.f, 3000.f };

	int32 _orthoWidthLevelIndex = 1;
	float _targetOrthoWidth = 1500.f;

	// 핑 찍기
	UPROPERTY(EditAnywhere, Category = "Game")
	TSubclassOf<AActor> _pingActorClass;
	UPROPERTY()
	AActor* _pingActor;
};
