// Fill out your copyright notice in the Description page of Project Settings.


#include "SceneCapturer.h"
#include "Kismet/GameplayStatics.h"

#include "../Character/PlayerCharacter.h"

// Sets default values
ASceneCapturer::ASceneCapturer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_sceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture Component 2D"));
	_sceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	//sceneCaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	_sceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
}

// Called when the game starts or when spawned
void ASceneCapturer::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	_player = Cast<APlayerCharacter>(playerCharacter);

	_sceneCaptureComponent->TextureTarget = _renderTarget;
	_sceneCaptureComponent->ProjectionType = _projectionType;

	// 필터 적용
	FilterActorList();
}

// Called every frame
void ASceneCapturer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_player) return;

	FVector playerLocation = _player->GetActorLocation();
	FVector newLocation = FVector(playerLocation.X, playerLocation.Y, _fixedHeight); // 위에서 아래로
	SetActorLocation(newLocation);
}

void ASceneCapturer::FilterActorList()
{
	TArray<FName> actorTagsToFilter;

	switch (_sceneCaptureComponent->PrimitiveRenderMode)
	{
	case ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives:	// 있는 그대로 그리는 유형이면
		actorTagsToFilter = _hiddenActorTagList;								// 기피 액터 설정
		break;

	case ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList:			// 특정 액터만 그리는 옵션이면
		actorTagsToFilter = _showActorTagList;									// 표시 액터 설정
		break;
	}

	if (actorTagsToFilter.Num() == 0)
		return;

	for (FName tagName : actorTagsToFilter)
	{
		TArray<AActor*> actorsFound;	// 액터 배열 준비
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), tagName, actorsFound); // 태그로 목록 추출

		for (AActor* actorFound : actorsFound) // 태그가 일치하는 액터들을
		{
			switch (_sceneCaptureComponent->PrimitiveRenderMode)
			{
			case ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives:
				_sceneCaptureComponent->HiddenActors.Add(actorFound);
				break;

			case ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList:
				_sceneCaptureComponent->ShowOnlyActors.Add(actorFound);
				break;
			}
		}
	}
}

