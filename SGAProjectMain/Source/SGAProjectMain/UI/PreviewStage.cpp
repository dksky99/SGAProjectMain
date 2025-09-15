// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewStage.h"

#include "Components/SceneCaptureComponent2D.h"
#include "../Character/HellDiver/HellDiver.h"

// Sets default values
APreviewStage::APreviewStage()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    _root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(_root);

    _previewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    _previewMesh->SetupAttachment(_root);
    _previewMesh->SetCastShadow(false);

    _sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Capture"));
    _sceneCapture->SetupAttachment(_root);
    _sceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    _sceneCapture->bCaptureEveryFrame = true;
    _sceneCapture->ShowOnlyActorComponents(this);

}

// Called when the game starts or when spawned
void APreviewStage::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APreviewStage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APreviewStage::SetPreviewStageFromCharacter(AHellDiver* character)
{
    if (USkeletalMeshComponent* mesh = character->FindComponentByClass<USkeletalMeshComponent>())
    {
        _previewMesh->SetSkeletalMesh(mesh->GetSkeletalMeshAsset());
        _previewMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        _previewMesh->PlayAnimation(_animSequence, true);
    }

    if (_renderTarget)
        _sceneCapture->TextureTarget = _renderTarget;

    _sceneCapture->CaptureScene();
}

