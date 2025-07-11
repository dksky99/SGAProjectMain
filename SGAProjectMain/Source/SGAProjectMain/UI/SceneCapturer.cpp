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

	_player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	_curFollowTarget = _player;

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
		FVector2D delta;
		_playerController->GetInputMouseDelta(delta.X, delta.Y);

		FVector cursorLocation = _cursorActor->GetActorLocation();
		cursorLocation.X += delta.Y * 15.f;
		cursorLocation.Y += delta.X * 15.f;
	
		// 맵 안에서만 이동
		cursorLocation.X = FMath::Clamp(cursorLocation.X, 0.f, _maxMapSize.X);
		cursorLocation.Y = FMath::Clamp(cursorLocation.Y, 0.f, _maxMapSize.Y);
		_cursorActor->SetActorLocation(cursorLocation);

		// 위젯에서 텍스트 연동
		BroadcastCursorInfo();
	}

	float curOrthoWidth = _sceneCaptureComponent->OrthoWidth;

	// 타겟 액터 따라다니기
	FVector targetLocation = _curFollowTarget->GetActorLocation();

	float halfWidth = curOrthoWidth * 0.5f;

	// 일정 범위 내에서는 타겟 액터를 따라다니고, 맵 가장자리에서는 타겟 액터만 이동
	float clampedX = FMath::Clamp(targetLocation.X, halfWidth, _maxMapSize.X - halfWidth);
	float clampedY = FMath::Clamp(targetLocation.Y, halfWidth, _maxMapSize.Y - halfWidth);

	FVector newLocation = FVector(clampedX, clampedY, _fixedHeight); // 위에서 아래로
	SetActorLocation(newLocation);

	// 확대 혹은 축소 시
	if (curOrthoWidth != _targetOrthoWidth)
	{
		float newOrthoWidth = FMath::FInterpTo(curOrthoWidth, _targetOrthoWidth, DeltaTime, 8.f);
		_sceneCaptureComponent->OrthoWidth = newOrthoWidth;
		
		if (_cursorActor)
			BroadcastCursorInfo();
	}

	if (_pingActor)
		BroadcastPingInfo();
}

void ASceneCapturer::ResetMap()
{
	_curFollowTarget = _player; // 다시 플레이어를 따라다니게 하기

	if (_cursorActor) // 커서가 있을 경우 삭제
	{
		_cursorActor->Destroy();
		_cursorActor = nullptr;
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
		startPos.Z = _fixedHeight - 10.f; // 다른 액터에 가려지지 않게
		_cursorActor = GetWorld()->SpawnActor<AActor>(_cursorActorClass, startPos, FRotator::ZeroRotator); // 플레이어 위치에 커서 스폰
		_curFollowTarget = _cursorActor;
	}
	
	//_playerController->GetMousePosition(_lastMousePos.X, _lastMousePos.Y);
	_isDraggingCursor = true;
}

void ASceneCapturer::StopDraggingMap()
{
	_isDraggingCursor = false;
}

void ASceneCapturer::BroadcastCursorInfo()
{
	if (!_cursorActor) return;
	
	float halfWidth = _sceneCaptureComponent->OrthoWidth / 2.f;
	FVector sceneCapturerToCursor = _cursorActor->GetActorLocation() - this->GetActorLocation();

	FVector playerToCursor = _cursorActor->GetActorLocation() - _player->GetActorLocation();
	
	if (_cursorUpdateEvent.IsBound())
		_cursorUpdateEvent.Broadcast(sceneCapturerToCursor, playerToCursor, halfWidth);
}

bool ASceneCapturer::PingOnMap()
{
	if (!_cursorActor)
		return false;

	// 커서가 있을 경우 -> 핑 찍기 가능
	FVector cursorLocation = _cursorActor->GetActorLocation();
	const float removeThreshold = 100.f;

	if (_pingActor)
	{
		float distance = FVector::Dist(_pingActor->GetActorLocation(), cursorLocation);

		if (distance < removeThreshold) // 비슷한 위치에서 다시 누를 경우 핑 제거
		{
			_pingActor->Destroy();
			_pingActor = nullptr;
			
			if (_pingOnOffEvent.IsBound())
				_pingOnOffEvent.Broadcast(false);

			return true;
		}
		else // 거리가 있을 경우 커서 위치로 핑 이동
		{
			_pingActor->SetActorLocation(cursorLocation);

			if (_pingOnOffEvent.IsBound())
				_pingOnOffEvent.Broadcast(true);

			return true;
		}
	}

	// 없을 경우 생성
	_pingActor = GetWorld()->SpawnActor<AActor>(_pingActorClass, cursorLocation, FRotator::ZeroRotator);
	
	if (_pingOnOffEvent.IsBound())
		_pingOnOffEvent.Broadcast(true);

	return true;
}

void ASceneCapturer::BroadcastPingInfo()
{
	if (!_pingActor) return;

	float halfWidth = _sceneCaptureComponent->OrthoWidth / 2.f;
	FVector sceneCapturerToPing = _pingActor->GetActorLocation() - this->GetActorLocation();

	if (_pingUpdateEvent.IsBound())
		_pingUpdateEvent.Broadcast(sceneCapturerToPing, halfWidth);
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

