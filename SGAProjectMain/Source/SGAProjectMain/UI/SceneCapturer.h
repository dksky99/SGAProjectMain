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

	UPROPERTY()
	class APlayerCharacter* _player;

	float _fixedHeight = 500.0f; // 캡쳐하는 높이
};
