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

	SpawnNextWave();
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

