// Fill out your copyright notice in the Description page of Project Settings.


#include "EagleStrikeController.h"
#include "EaglePlane.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AEagleStrikeController::AEagleStrikeController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 기본 진입 방향은 +X
	_approachDirection = FVector::ForwardVector;

}

// Called when the game starts or when spawned
void AEagleStrikeController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEagleStrikeController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(_eagleInstance))
		return;

	// 이글 비행 진행도 (0.0f ~ 1.0f)
	const float flightProgress = _eagleInstance->GetFlightProgress();

	UpdateBombing(flightProgress);
}

void AEagleStrikeController::InitializeStrike(float startDelay)
{
	_startDelay = startDelay;

	// 진입 방향 계산
	_approachDirection = SelectApproachDirection();
	_approachDirection.Z = 0.0f;
	if (!_approachDirection.Normalize())
	{
		_approachDirection = FVector::ForwardVector;
	}

	// 폭탄 패턴 / 드롭 타이밍 계산
	BuildBombImpactPattern();
	BuildDropTimingPattern();

	// 이글 스폰 + 비행 정보 넘기기
	UWorld* world = GetWorld();
	if (!world || !_eaglePlaneClass)
	{
		return;
	}

	const FVector targetLocation = GetActorLocation(); // 컨트롤러(=스트라타젬) 위치가 타겟

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();

	AEaglePlane* eagle = world->SpawnActor<AEaglePlane>(
		_eaglePlaneClass,
		targetLocation,          // 초기 위치 (어차피 InitializeFlight에서 startLocation으로 덮어씀)
		FRotator::ZeroRotator,
		spawnParams
	);

	if (!eagle)
	{
		return;
	}

	// 이글에게 타겟 위치 + 진입 방향을 넘겨줌
	eagle->InitializeFlight(targetLocation, _approachDirection, _startDelay);

	// 폭격 진행 상태 초기화
	_nextSalvoIndex = 0;

	_eagleInstance = eagle;
}

float AEagleStrikeController::ComputeEtaToFirstImpact() const
{
	// 이글이 없으면 계산 불가
	if (!_eagleInstance)
	{
		return 0.0f;
	}

	// 드롭 타이밍이 없으면 계산 불가
	if (_salvoDropTimes.Num() <= 0)
	{
		return 0.0f;
	}

	// 폭탄 낙하지점이 없으면 계산 불가
	if (_bombImpactPoints.Num() <= 0)
	{
		return 0.0f;
	}

	// 폭탄 속도가 0이면 계산 불가
	if (_bombSpeed <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	// 첫 폭탄 드롭 시점 (정규화 t: 0.0f ~ 1.0f)
	const float tDrop = _salvoDropTimes[0];

	// 이글 전체 비행 시간
	const float totalRunDuration = _eagleInstance->GetTotalRunDuration();

	// 이글이 드롭 위치까지 가는 데 걸리는 시간
	const float timeToDrop = totalRunDuration * tDrop;

	// 그 시점에서의 이글의 머즐 위치
	const FVector muzzlePos = _eagleInstance->GetBombSpawnLocationAtNormalizedT(tDrop);

	// 첫 번째 폭탄 낙하지점
	const FVector impactPos = _bombImpactPoints[0];

	// 폭탄 비행 시간 (직선, 중력 0 가정)
	const float distance = FVector::Dist(muzzlePos, impactPos);
	const float timeFlight = distance / _bombSpeed;

	// 최종 ETA = 드롭 지점까지 이동 시간 + 폭탄 비행 시간 + 대기 시간
	return timeToDrop + timeFlight + _startDelay;
}

FVector AEagleStrikeController::SelectApproachDirection() const
{
	TArray<FVector> directions;
	BuildApproachDirectionArray(directions);

	TArray<FVector> freeDirections;

	// 컨트롤러 위치를 타겟 위치로 사용
	for (const FVector& dir : directions)
	{
		if (!IsDirectionBlockedByTallObstacle(dir))
		{
			freeDirections.Add(dir);
		}
	}

	// 장애물이 없는 방향이 하나라도 있으면 그 중 하나를 사용
	if (freeDirections.Num() > 0)
	{
		FVector chosen = freeDirections[0];
		chosen.Z = 0.0f;
		chosen.Normalize();
		return chosen;
	}

	// 모든 방향이 막혀 있으면 기본 방향(+X)을 사용
	FVector fallbackDir = FVector(1.0f, 0.0f, 0.0f);
	fallbackDir.Normalize();
	return fallbackDir;
}

void AEagleStrikeController::BuildApproachDirectionArray(TArray<FVector>& outDirections) const
{
	outDirections.Reset();

	// 동, 동북, 북, 서북, 서, 서남, 남, 동남 (XY 평면)
	outDirections.Add(FVector(1.0f, 0.0f, 0.0f));                      // 동
	outDirections.Add(FVector(1.0f, 1.0f, 0.0f).GetSafeNormal());      // 동북
	outDirections.Add(FVector(0.0f, 1.0f, 0.0f));                      // 북
	outDirections.Add(FVector(-1.0f, 1.0f, 0.0f).GetSafeNormal());     // 서북
	outDirections.Add(FVector(-1.0f, 0.0f, 0.0f));                     // 서
	outDirections.Add(FVector(-1.0f, -1.0f, 0.0f).GetSafeNormal());    // 서남
	outDirections.Add(FVector(0.0f, -1.0f, 0.0f));                     // 남
	outDirections.Add(FVector(1.0f, -1.0f, 0.0f).GetSafeNormal());     // 동남
}

bool AEagleStrikeController::IsDirectionBlockedByTallObstacle(const FVector& directionXY) const
{
	UWorld* world = GetWorld();
	if (!world)
	{
		return false;
	}

	// XY 평면 방향만 사용
	FVector dir = directionXY;
	dir.Z = 0.0f;
	if (!dir.Normalize())
	{
		return false;
	}

	// 컨트롤러 위치 = 타겟 위치
	const FVector targetLocation = GetActorLocation();

	// "높은 장애물"을 검사할 Z 높이
	const float checkZ = targetLocation.Z + _tallObstacleHeight;

	// 타겟 위치에서 해당 방향으로 _checkRadius만큼 나간 지점까지
	FVector start = targetLocation;
	start.Z = checkZ;

	FVector end = start + dir * _checkRadius;
	end.Z = checkZ;

	FHitResult hitResult;
	FCollisionQueryParams params(SCENE_QUERY_STAT(EagleObstacleTrace), false);
	params.bReturnPhysicalMaterial = false;

	// WorldStatic(지형/건물 등)만 장애물로 취급
	const bool bHit = world->LineTraceSingleByChannel(
		hitResult,
		start,
		end,
		ECC_WorldStatic,
		params
	);

	return bHit;
}

void AEagleStrikeController::BuildBombImpactPattern()
{
	_bombImpactPoints.Reset();

	// 폭탄이 아예 없으면 종료
	if (_totalBombCount <= 0)
	{
		return;
	}

	const FVector target = GetActorLocation();

	// 이글 진행 방향: 바깥에서 타겟으로 향하는 방향
	FVector forward = -_approachDirection;
	forward.Z = 0.0f;
	if (!forward.Normalize())
	{
		forward = FVector(1.0f, 0.0f, 0.0f);
	}

	// 진행 방향에 직교하는 우측 벡터
	FVector right(-forward.Y, forward.X, 0.0f);
	if (!right.Normalize())
	{
		right = FVector(0.0f, 1.0f, 0.0f);
	}

	// 1발이면 그냥 타겟 정중앙에 하나만 떨어뜨립니다.
	if (_totalBombCount == 1)
	{
		_bombImpactPoints.Add(target);
		return;
	}

	// 이 아래는 짝수 개수(2, 4, 6, ...)라고 가정
	const int32 bombsPerSalvo = 2;
	const int32 salvoCount = FMath::Max(1, _totalBombCount / bombsPerSalvo);

	// 살보 중심 인덱스 (홀수면 가운데가 0, 짝수면 중심이 두 살보 사이)
	const float centerIndexF = static_cast<float>(salvoCount - 1) * 0.5f;

	for (int32 i = 0; i < salvoCount; ++i)
	{
		const float k = static_cast<float>(i) - centerIndexF; // -2,-1,0,1,2...

		// 타겟 기준 앞/뒤 위치
		const FVector salvoCenter = target + forward * (k * _salvoSpacing);

		// 좌/우 두 발
		const FVector bombL = salvoCenter - right * _lateralOffset;
		const FVector bombR = salvoCenter + right * _lateralOffset;

		_bombImpactPoints.Add(bombL);
		_bombImpactPoints.Add(bombR);
	}
}

void AEagleStrikeController::BuildDropTimingPattern()
{
	_salvoDropTimes.Reset();
	_ascentStartT = 0.5f;

	// 폭탄이 없으면 타이밍도 없음
	if (_totalBombCount <= 0)
	{
		return;
	}

	// 1발이면 그냥 중앙(t = 0.5f)에 떨어뜨린다고 가정
	if (_totalBombCount == 1)
	{
		_salvoDropTimes.Add(0.5f);
		return;
	}

	// 1발이면 이미 위에서 처리했다고 가정, 나머지는 짝수(2,4,6,...)로 사용
	const int32 salvoCount = _totalBombCount / 2;

	// 폭탄 투하는 전체 런(t:0~1) 중 "중앙 구간"에서만 일어나게끔
	// 예: 0.3f ~ 0.7f 구간을 사용
	const float dropStartT = 0.3f;
	const float dropEndT = 0.7f;

	_salvoDropTimes.Reserve(salvoCount);

	for (int32 i = 0; i < salvoCount; ++i)
	{
		// 0.0f ~ 1.0f 사이 비율
		const float alpha = (salvoCount > 1)
			? static_cast<float>(i) / static_cast<float>(salvoCount - 1)
			: 0.5f;

		// 투하 구간 내에서 선형 분포
		const float t = FMath::Lerp(dropStartT, dropEndT, alpha);
		_salvoDropTimes.Add(t);
	}
}

void AEagleStrikeController::UpdateBombing(float flightProgress)
{
	// 폭탄/타이밍 데이터가 없으면 처리 불가
	if (_salvoDropTimes.Num() <= 0 || _bombImpactPoints.Num() <= 0)
	{
		return;
	}

	// 살보 개수
	const int32 salvoCount = _salvoDropTimes.Num();

	// 한 살보당 폭탄 개수 (현재 구조에서는 2발 기준)
	const int32 bombsPerSalvo = FMath::Max(1, _bombImpactPoints.Num() / FMath::Max(1, salvoCount));

	// 프레임 스킵을 대비해서 while로 "지나간 살보"까지 모두 처리
	while (_nextSalvoIndex < salvoCount &&
		flightProgress >= _salvoDropTimes[_nextSalvoIndex])
	{
		const int32 baseIndex = _nextSalvoIndex * bombsPerSalvo;

		for (int32 i = 0; i < bombsPerSalvo; ++i)
		{
			const int32 impactIndex = baseIndex + i;
			if (!_bombImpactPoints.IsValidIndex(impactIndex))
			{
				continue;
			}

			const FVector impactPos = _bombImpactPoints[impactIndex];
			SpawnOneBombTowards(impactPos);
		}

		++_nextSalvoIndex;
	}
}

void AEagleStrikeController::SpawnOneBombTowards(const FVector& impactPos)
{
	if (!_eagleInstance || !_bombClass)
	{
		return;
	}

	UWorld* world = GetWorld();
	if (!world)
	{
		return;
	}

	// 현재 프레임에서의 실제 머즐 위치 (이글이 관리)
	const FVector spawnLocation = _eagleInstance->GetBombSpawnLocation();

	const FVector toTarget = impactPos - spawnLocation;
	FVector dir = toTarget.GetSafeNormal();
	if (dir.IsNearlyZero())
	{
		// 타겟이 너무 가까워 정규화가 안 되면, 일단 아래 방향으로 쏩니다.
		dir = FVector(0.0f, 0.0f, -1.0f);
	}

	const FRotator spawnRotation = dir.Rotation();

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();

	ABombProjectile* bomb = world->SpawnActor<ABombProjectile>(
		_bombClass,
		spawnLocation,
		spawnRotation,
		spawnParams
	);

	if (!bomb)
	{
		return;
	}

	// ProjectileMovement가 있으면 낙하 속도 적용
	if (UProjectileMovementComponent* pm = bomb->FindComponentByClass<UProjectileMovementComponent>())
	{
		pm->Velocity = dir * _bombSpeed;
	}
}


