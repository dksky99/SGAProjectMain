// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverDropPod.h"
#include "../../../Character/PlayerCharacter.h"

AHellDiverDropPod::AHellDiverDropPod()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AHellDiverDropPod::BeginPlay()
{
	Super::BeginPlay();
	_desiredVelXY = FVector::ZeroVector;
}

void AHellDiverDropPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 낙하 중에만 수평/수직 속도 주입
	if (!_isGrounded)
	{
		UpdateVelocityImmediate();
	}

	// 착지 후에도 상승 보간은 돌아야 합니다
	if (_spawnedPlayer && _raiseElapsed < _raiseDuration)
	{
		UpdateRaisePlayer(DeltaTime);
	}
}

void AHellDiverDropPod::SpawnInternalActor(const FVector& SpawnLocation)
{
	if (!_dropPodToSpawn)
		return;

	// 헬다이버인지 확인(블루프린트 포함)
	if (!_dropPodToSpawn->IsChildOf(APlayerCharacter::StaticClass()))
	{
		// 헬다이버가 아니면 부모 기본 동작(헬포드/일반 액터 스폰 로직) 사용
		Super::SpawnInternalActor(SpawnLocation);
		return;
	}

	// 지면 아래에서 헬다이버 스폰
	const FVector landing = GetActorLocation(); // 착지 지점
	const FVector spawnLoc = landing - FVector(0.0f, 0.0f, _spawnDepth);
	const FRotator spawnRot = GetActorRotation();

	FActorSpawnParameters sp;
	sp.Owner = this;
	sp.Instigator = GetInstigator();
	sp.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* spawned = GetWorld()->SpawnActor<AActor>(_dropPodToSpawn, spawnLoc, spawnRot, sp);
	if (!spawned)
		return;

	// 헬다이버로 캐스팅
	APlayerCharacter* player = Cast<APlayerCharacter>(spawned);
	if (!player)
		return;

	_spawnedPlayer = player;

	// 상승 중 끼임 방지: 일단 충돌 끔
	if (UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(_spawnedPlayer->GetRootComponent()))
	{
		rootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 상승 진행도 초기화
	_raiseElapsed = 0.0f;
}

void AHellDiverDropPod::UpdateRaisePlayer(const float dt)
{
	_raiseElapsed += dt;

	const float t = FMath::Clamp(_raiseElapsed / _raiseDuration, 0.0f, 1.0f);
	const FVector landing = GetActorLocation();
	const FVector start = landing - FVector(0.0f, 0.0f, _spawnDepth);
	const FVector target = landing + FVector(0.0f, 0.0f, 2.0f);  // 살짝 지상 위

	const FVector pos = FMath::Lerp(start, target, t);
	_spawnedPlayer->SetActorLocation(pos, false, nullptr, ETeleportType::TeleportPhysics);

	if (t >= 1.0f)
	{
		FinishRaiseAndPossess();
	}
}

void AHellDiverDropPod::FinishRaiseAndPossess()
{
	// 충돌 활성화
	if (UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(_spawnedPlayer->GetRootComponent()))
	{
		rootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// 컨트롤러 인계: Owner 우선, 없으면 InstigatorController
	if (AController* c = Cast<AController>(GetOwner()))
	{
		c->Possess(_spawnedPlayer);
	}
	else if (AController* c2 = GetInstigatorController())
	{
		c2->Possess(_spawnedPlayer);
	}


	// 여기서 드랍포드 액터 Tick 비활성화
	SetActorTickEnabled(false);

	// 필요 시 드랍포드 정리(예: 상호작용 오브젝트로 남기거나 파괴)
	// Destroy();
}

void AHellDiverDropPod::ApplyHorizontalInput(const float axisX, const float axisY, const FRotator& controlYawOnly)
{
	if (_isGrounded)
	{
		_desiredVelXY = FVector::ZeroVector;
		return;
	}

	// 컨트롤러 Yaw 기준 전/우 벡터
	const FVector forward = FRotationMatrix(controlYawOnly).GetUnitAxis(EAxis::X);
	const FVector right = FRotationMatrix(controlYawOnly).GetUnitAxis(EAxis::Y);

	// 누적/가속 없이 즉시 최대 속도로 반응
	const FVector dir = forward * axisY + right * axisX;

	// 수평 성분만 유지
	const FVector horizontal = FVector(dir.X, dir.Y, 0.0f).GetClampedToMaxSize(1.0f);

	_desiredVelXY = horizontal * _maxHorizontalSpeed;

		UE_LOG(LogTemp, Display,
			TEXT("[Input] axisX=%.3f axisY=%.3f  yaw=%.2f  forward=%s  right=%s  dir=%s  horizontal=%s  desiredXY=%s (max=%.1f)"),
			axisX, axisY,
			controlYawOnly.Yaw,
			*forward.ToString(),
			*right.ToString(),
			*dir.ToString(),
			*horizontal.ToString(),
			*_desiredVelXY.ToString(),
			_maxHorizontalSpeed
		);
}

void AHellDiverDropPod::UpdateVelocityImmediate()
{
	if (_projectile)
	{
		// 즉시 반응: 매 프레임 수평은 _desiredVelXY, 수직은 고정 낙하속도
		const FVector newVel(_desiredVelXY.X, _desiredVelXY.Y, -_fallSpeed);
		_projectile->Velocity = newVel;
	}
}
