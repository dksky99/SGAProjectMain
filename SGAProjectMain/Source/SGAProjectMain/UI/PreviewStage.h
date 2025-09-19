// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreviewStage.generated.h"

UCLASS()
class SGAPROJECTMAIN_API APreviewStage : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APreviewStage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetPreviewStageFromCharacter(class AHellDiver* character);

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* _root;
	UPROPERTY(VisibleAnywhere) 
	class USceneCaptureComponent2D* _sceneCapture;
	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* _previewMesh;

	UPROPERTY(EditAnywhere, Category = "Game/Capture")
	class UTextureRenderTarget2D* _renderTarget;

	UPROPERTY(EditAnywhere, Category = "Game/Capture")
	class UAnimSequence* _animSequence;
};
