// Fill out your copyright notice in the Description page of Project Settings.


#include "EaglePlane.h"

// Sets default values
AEaglePlane::AEaglePlane()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_meshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = _meshComponent;

	_bombSocketName = TEXT("Bomb_Socket");

	// 기본 상태: 안 보이고, 충돌 없음
	SetActorHiddenInGame(true);

	_meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_meshComponent->SetGenerateOverlapEvents(false);
	SetActorEnableCollision(false);

	_isActive = false;
}

// Called when the game starts or when spawned
void AEaglePlane::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorEnableCollision(false);
}

// Called every frame
void AEaglePlane::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!_isActive)
	{
		return;
	}

	_elapsedTime += DeltaTime;

	// 0.0f ~ 1.0f 사이 정규화 진행도
	const float t = FMath::Clamp(_elapsedTime / _totalRunDuration, 0.0f, 1.0f);

	// 경로 상 위치 계산
	const FVector newLocation = GetPositionAtNormalizedT(t);
	SetActorLocation(newLocation);

	// 이동 방향으로 회전 정렬
	const float nextT = FMath::Clamp(t + 0.01f, 0.0f, 1.0f);
	const FVector nextLocation = GetPositionAtNormalizedT(nextT);
	const FVector forward = (nextLocation - newLocation).GetSafeNormal();
	if (!forward.IsNearlyZero())
	{
		FRotator newRotation = forward.Rotation();
		newRotation.Yaw += -90.0f;
		SetActorRotation(newRotation);
	}

	// 비행 종료
	if (t >= 1.0f)
	{
		_isActive = false;
		Destroy();
	}
}

void AEaglePlane::InitializeFlight(const FVector& targetLocation, const FVector& approachDirection, float startDelay)
{
	_targetLocation = targetLocation;

	_approachDirection = approachDirection;
	_approachDirection.Z = 0.0f;
	if (!_approachDirection.Normalize())
	{
		_approachDirection = FVector::ForwardVector;
	}

	_totalRunDuration = FMath::Max(_totalRunDuration, 0.01f);
	_elapsedTime = 0.0f;

	// 경로 기준점 계산
	const float targetZ = _targetLocation.Z;
	const float centerZ = targetZ + _dropAltitude;

	FVector flatDir = _approachDirection;
	flatDir.Z = 0.0f;
	if (!flatDir.Normalize())
	{
		flatDir = FVector::ForwardVector;
	}

	const FVector startXY = _targetLocation + flatDir * _pathHalfDistance;
	const FVector exitXY = _targetLocation - flatDir * _pathHalfDistance;

	_startLocation = FVector(startXY.X, startXY.Y, centerZ + _extraHeight);
	_centerLocation = FVector(_targetLocation.X, _targetLocation.Y, centerZ);
	_exitLocation = FVector(exitXY.X, exitXY.Y, centerZ + _extraHeight);

	// 시작 위치로 세팅
	SetActorLocation(_startLocation);

	FTimerHandle startTimer;
	// _startDelay 후에 비행 시작
	GetWorldTimerManager().SetTimer(
		startTimer,
		this,
		&AEaglePlane::StartFlight,
		startDelay,
		false
	);
}

FVector AEaglePlane::GetPositionAtNormalizedT(float t) const
{
	// 0.0f ~ 1.0f 범위로 클램프
	const float clampedT = FMath::Clamp(t, 0.0f, 1.0f);
	const float u = 1.0f - clampedT;

	// 경로 기준점들: 시작 / 중앙(최저 고도) / 이탈
	const FVector p0 = _startLocation;
	const FVector p1 = _centerLocation;
	const FVector p2 = _exitLocation;

	// 2차 Bezier 곡선: P(t) = (1 - t)^2 * P0 + 2(1 - t) * t * P1 + t^2 * P2
	const FVector result =
		u * u * p0 +
		2.0f * u * clampedT * p1 +
		clampedT * clampedT * p2;

	return result;
}

FVector AEaglePlane::GetBombSpawnLocationAtNormalizedT(float t) const
{
	// t 시점에서의 기체 중심 위치
	const float clampedT = FMath::Clamp(t, 0.0f, 1.0f);
	const FVector planePos = GetPositionAtNormalizedT(clampedT);

	// 메쉬나 소켓이 없으면 기체 위치를 그대로 사용
	if (!_meshComponent || _bombSocketName.IsNone() || !_meshComponent->DoesSocketExist(_bombSocketName))
	{
		return planePos;
	}

	// 소켓의 "컴포넌트 공간" 위치 (메쉬 로컬 기준)
	const FTransform socketLocalTM = _meshComponent->GetSocketTransform(_bombSocketName, RTS_Component);
	const FVector localOffset = socketLocalTM.GetLocation();

	// t 시점에서의 회전 방향(앞 방향) 예측: Bezier 경로를 따라 앞을 보게 함
	const float nextT = FMath::Clamp(clampedT + 0.01f, 0.0f, 1.0f);
	const FVector posNow = GetPositionAtNormalizedT(clampedT);
	const FVector posNext = GetPositionAtNormalizedT(nextT);
	const FVector forward = (posNext - posNow).GetSafeNormal();

	FRotator planeRot = FRotator::ZeroRotator;
	if (!forward.IsNearlyZero())
	{
		planeRot = forward.Rotation();
	}

	const FRotationMatrix rotMat(planeRot);
	const FVector worldOffset = rotMat.TransformPosition(localOffset);

	return planePos + worldOffset;
}

void AEaglePlane::StartFlight()
{
	_isActive = true;

	SetActorHiddenInGame(false);
}

FVector AEaglePlane::GetBombSpawnLocation() const
{
	if (_meshComponent && !_bombSocketName.IsNone() && _meshComponent->DoesSocketExist(_bombSocketName))
	{
		const FTransform socketTM = _meshComponent->GetSocketTransform(_bombSocketName, RTS_World);
		return socketTM.GetLocation();
	}

	// 소켓이 없으면 일단 액터 위치 사용
	return GetActorLocation();
}

float AEaglePlane::GetFlightProgress() const
{
	if (_totalRunDuration <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}
	return FMath::Clamp(_elapsedTime / _totalRunDuration, 0.0f, 1.0f);
}

