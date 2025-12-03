// Fill out your copyright notice in the Description page of Project Settings.


#include "BombardmentController.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ABombardmentController::ABombardmentController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABombardmentController::BeginPlay()
{
	Super::BeginPlay();

	
}

void ABombardmentController::SpawnNextWave()
{
	if (_waveIdx >= _waveCount)
	{
		Destroy();
		return;
	}

	_shellIdxInWave = 0;

	GetWorldTimerManager().SetTimer(
		_intraWaveTimer,
		this,
		&ABombardmentController::SpawnNextShell,
		_intraWaveInterval,
		true
	);
}

void ABombardmentController::SpawnNextShell()
{
	if (_shellIdxInWave >= _shellsPerWave)
	{
		GetWorldTimerManager().ClearTimer(_intraWaveTimer);

		_waveIdx++;

		GetWorldTimerManager().SetTimer(
			_waveTimer,
			this,
			&ABombardmentController::SpawnNextWave,
			_waveInterval,
			false
		);
		return;
	}

	const FVector target = SampleTargetLocation();
	SpawnOneShellAt(target);

	_shellIdxInWave++;
}

void ABombardmentController::SpawnOneShellAt(const FVector& target)
{
	if (!_projectileClass)
	{
		return;
	}

	// 투하 원점: 현재는 (0,0,0) 상공 고정
	const FVector dropOrigin = FVector::ZeroVector;
	const FVector spawnLocation = dropOrigin + FVector(0.0f, 0.0f, _dropHeight);

	// 방향 = 타깃 - 스폰 위치
	const FVector direction = (target - spawnLocation).GetSafeNormal();
	const FRotator spawnRotation = direction.Rotation();

	FActorSpawnParameters sp;
	sp.Owner = _owner ? _owner : this;
	sp.Instigator = _owner ? Cast<APawn>(_owner) : nullptr;
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* shell = GetWorld()->SpawnActor<AActor>(_projectileClass, spawnLocation, spawnRotation, sp);
	if (shell)
	{
		// ProjectileMovement가 있으면 낙하 속도 적용
		if (UProjectileMovementComponent* pm = shell->FindComponentByClass<UProjectileMovementComponent>())
		{
			pm->Velocity = direction * _dropSpeed;
		}
	}
}

FVector ABombardmentController::SampleTargetLocation() const
{
	// 랜덤 산포: _impactCenter 기준으로 반경 _scatterRadius 내 균등 샘플링
	const float r = _rng.FRandRange(0.0f, _scatterRadius);
	const float th = _rng.FRandRange(0.0f, 2.0f * PI);
	return _impactCenter + FVector(FMath::Cos(th) * r, FMath::Sin(th) * r, 0.0f);
}

void ABombardmentController::InitializeBombardment(float startDelay)
{
	// 피격 중심은 컨트롤러 스폰 위치(= 신호기 위치)
	_impactCenter = GetActorLocation();

	// 오너 참조
	_owner = GetOwner();

	// RNG 초기화
	if (_seed != 0)
	{
		_rng.Initialize(_seed);
	}

	_waveIdx = 0;
	_shellIdxInWave = 0;

	_startDelay = FMath::Max(startDelay, 0.0f);

	FTimerHandle startTimer;
	// _startDelay 후에 첫 웨이브 시작
	GetWorldTimerManager().SetTimer(
		startTimer,
		this,
		&ABombardmentController::SpawnNextWave,
		_startDelay,
		false
	);
}

float ABombardmentController::GetEstimatedFirstImpactDelay() const
{
	// 웨이브나 포탄 수가 0이면 폭격이 일어나지 않으므로 0.0f 반환
	if (_waveCount <= 0 || _shellsPerWave <= 0)
	{
		return 0.0f;
	}

	const float speed = FMath::Max(_dropSpeed, KINDA_SMALL_NUMBER);

	// 첫 번째 포탄 스폰 시점 = BeginPlay 기준 _startDelay + _intraWaveInterval 후
	const float firstShellSpawnDelay = _startDelay + _intraWaveInterval;

	// 스폰 위치: (0, 0, _dropHeight)
	const FVector spawnLocation = FVector(0.0f, 0.0f, _dropHeight);

	// 도착 지점: 폭격 중심(_impactCenter) 근처라고 보고, 중심까지 거리로 근사
	const FVector impactLocation = _impactCenter;

	// 스폰 위치 → 폭격 중심까지 실제 이동 거리
	const float travelDistance = FVector::Dist(spawnLocation, impactLocation);
	const float dropTime = travelDistance / speed;

	// BeginPlay 기준 첫 탄 도착까지 = 스폰 딜레이 + 이동 시간
	return firstShellSpawnDelay + dropTime;
}

float ABombardmentController::GetEstimatedBombardDuration() const
{
	// 웨이브나 포탄 수가 0이면 폭격이 일어나지 않으므로 0.0f 반환
	if (_waveCount <= 0 || _shellsPerWave <= 0)
	{
		return 0.0f;
	}

	const int32 waves = FMath::Max(_waveCount, 0);
	const int32 shellsPerWaveMinusOne = FMath::Max(_shellsPerWave - 1, 0);
	const int32 wavesMinusOne = FMath::Max(_waveCount - 1, 0);

	const float intraWaveTimeTotal =
		static_cast<float>(waves) * static_cast<float>(shellsPerWaveMinusOne) * _intraWaveInterval;
	const float interWaveTimeTotal =
		static_cast<float>(wavesMinusOne) * _waveInterval;

	// 첫 탄 도착 이후 → 마지막 탄 도착까지 걸리는 시간
	return intraWaveTimeTotal + interWaveTimeTotal;
}

