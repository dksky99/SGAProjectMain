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
	_curFollowTarget = playerCharacter;
	Cast<APlayerCharacter>(playerCharacter)->SetSceneCapturer(this);

	_playerController = GetWorld()->GetFirstPlayerController();

	_sceneCaptureComponent->TextureTarget = _renderTarget;
	_sceneCaptureComponent->ProjectionType = _projectionType;
	_targetOrthoWidth = _orthoWidthLevel[_orthoWidthLevelIndex];
	_sceneCaptureComponent->OrthoWidth = _targetOrthoWidth;

	// 필터 적용
	FilterActorList();
}

// Called every frame
void ASceneCapturer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_curFollowTarget) return;

	// 드래그 중일 경우
	if (_isDraggingCursor && _cursorActor)
	{
		FVector2D curMousePos;
		_playerController->GetMousePosition(curMousePos.X, curMousePos.Y);

		FVector2D delta = curMousePos - _lastMousePos;
		_lastMousePos = curMousePos;

		// 드래그 방향으로 커서 이동
		FVector cursorLocation = _cursorActor->GetActorLocation();
		cursorLocation.Y += delta.X;
		cursorLocation.X -= delta.Y;
		//FVector Delta3D = FVector(Delta.X, Delta.Y, 0.f) * DragSensitivity; // 좌표계에 따라 YZ 반전 필요할 수도 있음
		_cursorActor->SetActorLocation(cursorLocation);
	}
	
	// 타겟 액터 따라다니기
	FVector targetLocation = _curFollowTarget->GetActorLocation();
	FVector newLocation = FVector(targetLocation.X, targetLocation.Y, _fixedHeight); // 위에서 아래로
	SetActorLocation(newLocation);

	// 확대 혹은 축소 시
	float curOrthoWidth = _sceneCaptureComponent->OrthoWidth;
	if (curOrthoWidth != _targetOrthoWidth)
	{
		float newOrthoWidth = FMath::FInterpTo(curOrthoWidth, _targetOrthoWidth, DeltaTime, 8.f);
		_sceneCaptureComponent->OrthoWidth = newOrthoWidth;
	}
}

void ASceneCapturer::ChangeOrthoWidth(bool zoomIn)
{
	if (zoomIn) // 맵 확대의 경우
	{
		if (_orthoWidthLevelIndex > 0) // 최대 확대 단계가 아닌 경우
		{
			_orthoWidthLevelIndex--; // 인덱스 감소 -> orthoWidth 감소 -> 맵 확대
		}
	}
	else // 맵 축소의 경우
	{
		if (_orthoWidthLevelIndex < 2) // 최대 축소 단계가 아닌 경우
		{
			_orthoWidthLevelIndex++; // 인덱스 증가 -> orthoWidth 증가 -> 맵 축소
		}
	}

	_targetOrthoWidth = _orthoWidthLevel[_orthoWidthLevelIndex];
}

void ASceneCapturer::StartDraggingMap()
{
	if (!_cursorActor)
	{
		FVector startPos = _curFollowTarget->GetActorLocation(); // 커서가 없을 경우 추적 타겟은 플레이어
		_cursorActor = GetWorld()->SpawnActor<AActor>(_cursorActorClass, startPos, FRotator::ZeroRotator); // 플레이어 위치에 커서 스폰
		_curFollowTarget = _cursorActor;
	}
	
	_playerController->GetMousePosition(_lastMousePos.X, _lastMousePos.Y);
	_isDraggingCursor = true;
}

void ASceneCapturer::StopDraggingMap()
{
	_isDraggingCursor = false;
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

