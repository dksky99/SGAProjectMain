// SentryTurret.cpp

#include "SentryTurret.h"

#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Animation/AnimInstance.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

#include "../../../Gun/GunBulletBase.h"
#include "SentryAnimInstance.h"
#include "../../../Character/CharacterBase.h"
#include "../../../Character/StatComponent.h"
#include "../../../SGAProjectMain.h"

ASentryTurret::ASentryTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트
	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(_mesh);

	_muzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	// 어태치는 BeginPlay에서 소켓 스냅

	// Perception
	_perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	_sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	_sightConfig->SightRadius = 3000.0f;
	_sightConfig->LoseSightRadius = 3500.0f;
	_sightConfig->PeripheralVisionAngleDegrees = 180.0f;
	_sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	_sightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	_sightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	_perception->ConfigureSense(*_sightConfig);
	_perception->SetDominantSense(_sightConfig->GetSenseImplementation());

	_curAmmo = 0;
	_curHp = _maxHp;

	SetGenericTeamId(FGenericTeamId((int32)ETeamID::HellDiver));
}

void ASentryTurret::BeginPlay()
{
	Super::BeginPlay();

	_curAmmo = _maxAmmo;
	_curHp = _maxHp;

	// 머즐 포인트를 소켓에 스냅(위치/회전 일치)
	if (_muzzlePoint && _mesh)
	{
		_muzzlePoint->AttachToComponent(
			_mesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("muzzleSocket")
		);
	}

	// Perception 이벤트
	if (_perception)
	{
		_perception->OnPerceptionUpdated.AddDynamic(this, &ASentryTurret::OnPerceptionUpdated);
	}

	_cachedAimTolDeg = _aimToleranceDeg;
	_cosAimTol = FMath::Cos(FMath::DegreesToRadians(_cachedAimTolDeg));

	_anim = Cast<USentryAnimInstance>(_mesh->GetAnimInstance());

	// 센트리용 프로젝타일 데이터 초기화
	if (_projectileDataAsset)
	{
		// 에셋에 들어있는 기본 프로젝타일 데이터 복사
		_projectileData = _projectileDataAsset->_projectileData;

		// 센트리 고유 데미지로 덮어쓰기
		_projectileData._baseDamage = _sentryBaseDamage;
		_projectileData._vsDurableDamage = _sentryVsDurableDamage;
	}

	InitNiagaraEffects();

	// 스폰
	StartSpawn();
}

void ASentryTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpawnDescent(DeltaTime);
	UpdateAimToTarget(DeltaTime);
	UpdateFireGate(DeltaTime);
}

// -------------------------------
// 외부 호출
// -------------------------------

void ASentryTurret::AIStartFire()
{
	if (_curAmmo <= 0) { HandleOutOfAmmo(); return; }

	FTimerManager& tm = GetWorldTimerManager();
	if (tm.IsTimerActive(_fireTimerHandle))
	{
		return;
	}

	tm.SetTimer(_fireTimerHandle, this, &ASentryTurret::Fire, _fireInterval, true, 0.0f);
}

void ASentryTurret::AIStopFire()
{
	GetWorldTimerManager().ClearTimer(_fireTimerHandle);

	for (UNiagaraComponent* muzzleNS : _muzzlePool)
		muzzleNS->Deactivate();

	for (UNiagaraComponent* casingNS : _casingPool)
		casingNS->Deactivate();
}

// -------------------------------
// 인지/타깃 선정
// -------------------------------

void ASentryTurret::OnPerceptionUpdated(const TArray<AActor*>& /*UpdatedActors*/)
{
	UpdateTargetSelection();
}

void ASentryTurret::UpdateTargetSelection()
{
	if (!_perception) return;

	// 기존 타깃이 계속 보이면 유지
	if (IsEnemyActor(_currentTarget) && IsTargetAttackable(_currentTarget))
	{
		FActorPerceptionBlueprintInfo info;
		if (_perception->GetActorsPerception(_currentTarget, info))
		{
			for (const FAIStimulus& s : info.LastSensedStimuli)
			{
				if (s.WasSuccessfullySensed())
				{
					return;
				}
			}
		}
	}

	// 새 타깃: 머즐에서 가장 가까운 현재 보이는 액터
	TArray<AActor*> perceived;
	_perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), perceived);

	ACharacterBase* best = nullptr;
	float bestD2 = FLT_MAX;

	for (AActor* actor : perceived)
	{
		if (!IsValid(actor)) continue;
		if (!IsEnemyActor(actor)) continue;

		// 캐릭터만 통과: Cast 한 번으로 충분합니다.
		ACharacterBase* candidate = Cast<ACharacterBase>(actor);
		if (!candidate) continue;

		if (!IsTargetAttackable(candidate)) continue;

		const float d2 = FVector::DistSquared(candidate->GetActorLocation(), GetMuzzleLocation());
		if (d2 < bestD2)
		{
			bestD2 = d2;
			best = candidate;
		}
	}

	_currentTarget = best;
	EnsureIdleTimer();
}

// -------------------------------
// 조준(핵심): 두 축 속도 제한을 C++에서 통합
// -------------------------------
float ASentryTurret::CalcYaw_Sentry()
{

	FTransform root = _mesh->GetComponentTransform();


	FVector rootForward = root.GetUnitAxis(EAxis::X);

	FVector targetVector;

	if (_currentTarget == nullptr)
	{
		targetVector = _idleAimPointWS - root.GetLocation();
		targetVector.Normalize();
	}
	else
	{
		FTransform targetTransfrom = _currentTarget->GetTransform();

		targetVector = targetTransfrom.GetLocation() - root.GetLocation();
		targetVector.Normalize();
	}
	
	FRotator targetRotator = targetVector.Rotation();

	float yaw = FMath::FindDeltaAngleDegrees(rootForward.Rotation().Yaw, targetRotator.Yaw);
	
	return yaw;
}

float ASentryTurret::CalcPitch_Sentry()
{
	const FVector HousingLocation = _mesh->GetSocketLocation(_boneName_Pitch);
	FVector TargetLocation;

	if (_currentTarget == nullptr)
		TargetLocation = _idleAimPointWS;
	else
		TargetLocation = _currentTarget->GetActorLocation();

	const FVector Direction = (TargetLocation - HousingLocation).GetSafeNormal();
	//const FQuat RotationQuat = FQuat::MakeFromRotationVector(Direction).Rotator().Yaw;
	return FQuat::FindBetweenNormals(GetActorForwardVector(), Direction).Rotator().Pitch;
	//return Direction.ToOrientationRotator().Pitch;
}

void ASentryTurret::ApplyAimSpeedLimit(float deltaSeconds, float targetYawDeg, float targetPitchDeg)
{
	// 프레임당 최대 회전량(도)
	const float maxYawStepDeg = _yawSpeedDegPerSec * deltaSeconds;
	const float maxPitchStepDeg = _PitchSpeedDegPerSec * deltaSeconds;

	// Yaw: 360도 회전 가능(한계 각도 클램프 없음). 최단 경로로 속도 제한
	{
		const float delta = FMath::FindDeltaAngleDegrees(_rotatorZ_CurrentDeg, targetYawDeg);
		const float step = FMath::Clamp(delta, -maxYawStepDeg, maxYawStepDeg);
		_rotatorZ_CurrentDeg = FMath::UnwindDegrees(_rotatorZ_CurrentDeg + step);
		// Yaw는 별도 클램프 없음
	}

	// Pitch: 상(+)/하(−) 한계로 클램프
	{
		const float delta = FMath::FindDeltaAngleDegrees(_gunHousingZ_CurrentDeg, targetPitchDeg);
		const float step = FMath::Clamp(delta, -maxPitchStepDeg, maxPitchStepDeg);
		_gunHousingZ_CurrentDeg = FMath::UnwindDegrees(_gunHousingZ_CurrentDeg + step);
		if (_isRaised && !_isSinking && !_isTransitionalAlign)
			_gunHousingZ_CurrentDeg = FMath::Clamp(_gunHousingZ_CurrentDeg, -_aimPitchDownDeg, _aimPitchUpDeg);
	}
}

bool ASentryTurret::IsAngleAligned(float currentDeg, float targetDeg, float toleranceDeg) const
{
	return FMath::Abs(FMath::FindDeltaAngleDegrees(currentDeg, targetDeg)) <= toleranceDeg;
}

bool ASentryTurret::IsSentryReadyToFire()
{
	// 탄/머즐
	if (_curAmmo <= 0) return false;
	if (!_muzzlePoint) return false;

	// 상태(상승 완료 후, 정렬/하강 아님)
	if (!_isRaised || _isTransitionalAlign || _isSinking) return false;

	return true;
}

bool ASentryTurret::IsTargetAttackable(ACharacterBase* target) const
{
	if (!IsValid(target)) return false;

	UStatComponent* stat = target->GetStatComponent();
	if (!stat) return false;
	if (stat->IsDead()) return false;

	return true;
}

bool ASentryTurret::HasAnyShootableEnemy()
{
	   if (!_perception) return false;

    TArray<AActor*> perceived;
    _perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), perceived);

    for (AActor* actor : perceived)
    {
        if (!IsValid(actor) || !IsEnemyActor(actor)) continue;

        ACharacterBase* asChar = Cast<ACharacterBase>(actor);
        if (!asChar) continue;
        if (!IsTargetAttackable(asChar)) continue;

        return true;
    }
    return false;
}

void ASentryTurret::UpdateAimToTarget(float deltaSeconds)
{
	if (!_mesh || !_muzzlePoint) return;
	if (!_isRaised || _isTransitionalAlign || _isSinking) return;
	if (!IsValid(_currentTarget))
		EnsureIdleTimer();

	// 목표 각도 계산(절대값)
	float targetYawZDeg = CalcYaw_Sentry();
	float targetPitchZDeg = CalcPitch_Sentry();

	// 속도 제한을 적용해 현재각을 갱신
	ApplyAimSpeedLimit(deltaSeconds, targetYawZDeg, targetPitchZDeg);

	// 애님 인스턴스 전달
	_anim->SetBoneAngles(_rotatorZ_CurrentDeg, _gunHousingZ_CurrentDeg);
}

// -------------------------------
// 발사 게이트/LOS
// -------------------------------

void ASentryTurret::UpdateFireGate(float deltaSeconds)
{
	// 코사인 캐시는 기존대로 유지(연출용)
	if (!FMath::IsNearlyEqual(_cachedAimTolDeg, _aimToleranceDeg, KINDA_SMALL_NUMBER))
	{
		_cachedAimTolDeg = _aimToleranceDeg;
		_cosAimTol = FMath::Cos(FMath::DegreesToRadians(_cachedAimTolDeg));
	}

	// 센트리 준비 상태 확인
	if (!IsSentryReadyToFire())
	{
		if (_lastWantsFire) { _lastWantsFire = false; AIStopFire(); }
		return;
	}

	// 조준각 계산(연출용)은 그대로
	if (IsTargetAttackable(_currentTarget))
	{
		const FVector muzzleLoc = _muzzlePoint->GetComponentLocation();
		const FVector muzzleFwd = _muzzlePoint->GetForwardVector();
		const FVector toVec = _currentTarget->GetActorLocation() - muzzleLoc;

		if (!toVec.IsNearlyZero())
		{
			const FVector toDir = toVec.GetSafeNormal();
			const float dot = FVector::DotProduct(muzzleFwd, toDir);
			const bool bAimed = (dot >= (_cosAimTol - KINDA_SMALL_NUMBER));
			// bAimed는 필요 시 연출/스프레드에만 활용
		}
	}

	// 정책: 시야 내 ‘공격 가능한 적’이 1명이라도 있으면 발사 유지
	const bool wantsFire = HasAnyShootableEnemy();

	if (wantsFire != _lastWantsFire)
	{
		_lastWantsFire = wantsFire;
		wantsFire ? AIStartFire() : AIStopFire();
	}
}

// -------------------------------
// 발사/이펙트
// -------------------------------

void ASentryTurret::Fire()
{
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}

	const FVector muzzleLocation = _muzzlePoint->GetComponentLocation();
	const FVector fireDirection = _muzzlePoint->GetForwardVector();

	SpawnBullet(muzzleLocation, fireDirection);
	PlayMuzzleFX();
	PlayCasingFX();

	_curAmmo--;
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}
}

void ASentryTurret::SpawnBullet(const FVector& muzzleLocation, const FVector& direction)
{
	if (!_projectileDataAsset)
		return;

	// 에셋에서 실제 스폰할 총알 클래스를 가져옵니다.
	TSubclassOf<AGunBulletBase> bulletClass = _projectileDataAsset->_projectileClass;
	if (!bulletClass)
		return;

	const FRotator spawnRot = direction.Rotation();

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();
	spawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AGunBulletBase* bullet = GetWorld()->SpawnActor<AGunBulletBase>(
		bulletClass,
		muzzleLocation,
		spawnRot,
		spawnParams
	);

	if (bullet)
	{
		// 센트리용으로 준비해 둔 프로젝타일 데이터 주입
		bullet->InitializeProjectile(_projectileData);
	}
}

void ASentryTurret::InitNiagaraEffects()
{
	if (!_mesh)
		return;

	// 머즐 풀 초기화
	_muzzlePool.Empty();
	if (_muzzleNS && _muzzlePoolSize > 0)
	{
		_muzzlePool.Reserve(_muzzlePoolSize);

		for (int32 i = 0; i < _muzzlePoolSize; ++i)
		{
			UNiagaraComponent* c = NewObject<UNiagaraComponent>(this, TEXT("NSC_Muzzle_Pooled"));
			if (!c) continue;

			c->SetAsset(_muzzleNS);
			c->bAutoActivate = false;

			c->SetupAttachment(_muzzlePoint);
			
			c->RegisterComponent();
			_muzzlePool.Add(c);
		}

		_muzzlePoolIndex = 0;
	}

	// 탄피 풀 초기화
	_casingPool.Empty();
	if (_casingNS && _casingPoolSize > 0)
	{
		_casingPool.Reserve(_casingPoolSize);

		for (int32 i = 0; i < _casingPoolSize; ++i)
		{
			UNiagaraComponent* c = NewObject<UNiagaraComponent>(this, TEXT("NSC_Casing_Pooled"));
			if (!c) continue;

			c->SetAsset(_casingNS);
			c->bAutoActivate = false;

			c->SetupAttachment(_mesh);
			c->AttachToComponent(
				_mesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				_casingSocketName
			);

			c->RegisterComponent();
			_casingPool.Add(c);
		}

		_casingPoolIndex = 0;
	}
}

void ASentryTurret::PlayMuzzleFX()
{
	if (_muzzlePool.Num() == 0)
		return;

	UNiagaraComponent* c = _muzzlePool[_muzzlePoolIndex];
	_muzzlePoolIndex = (_muzzlePoolIndex + 1) % _muzzlePool.Num();
	if (!c) return;

	// 최신 위치 스냅(포인트/소켓 움직임 대응)
	c->AttachToComponent(
		_muzzlePoint,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);


	// 완전 재시작(동일 프레임 다중 호출 대비)
	c->DeactivateImmediate();
	c->ReinitializeSystem();
	c->Activate(true);
}

void ASentryTurret::PlayCasingFX()
{
	if (_casingPool.Num() == 0)
		return;

	UNiagaraComponent* c = _casingPool[_casingPoolIndex];
	_casingPoolIndex = (_casingPoolIndex + 1) % _casingPool.Num();

	if (!c) return;

	// 소켓 재스냅(무브먼트 대응)
	c->AttachToComponent(
		_mesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		_casingSocketName
	);

	c->DeactivateImmediate();
	c->ReinitializeSystem();
	c->Activate(true);
}

// -------------------------------
// 스폰/디스폰
// -------------------------------

bool ASentryTurret::IsEnemyActor(const AActor* Other) const
{
	if (!Other || Other == this)
		return false;

	// 센트리/상대 모두 Team 인터페이스로 판단
	const FGenericTeamId myId = GetGenericTeamId();

	FGenericTeamId otherId = FGenericTeamId::NoTeam;
	if (const IGenericTeamAgentInterface* teamAgent = Cast<IGenericTeamAgentInterface>(Other))
	{
		otherId = teamAgent->GetGenericTeamId();
	}
	else
	{
		// 컨트롤러/소유자 쪽에서 팀을 들고 있을 수도 있음 → 컨트롤러 우선 확인
		if (const APawn* pawn = Cast<APawn>(Other))
		{
			if (const AController* ctrl = pawn->GetController())
			{
				if (const IGenericTeamAgentInterface* ctrlTeam = Cast<IGenericTeamAgentInterface>(ctrl))
				{
					otherId = ctrlTeam->GetGenericTeamId();
				}
			}
		}
	}

	// 규약: 동일 TeamId → Friendly, 서로 다르고 NoTeam이 아니면 → Hostile, 아니면 Neutral
	if (otherId == FGenericTeamId::NoTeam)
	{
		return false; // Neutral
	}
	return myId != otherId; // 다르면 적대
}

void ASentryTurret::StartSpawn()
{
	const float kDepthCm = _mesh->GetSkeletalMeshAsset()->GetBounds().BoxExtent.Z * 2.0f;

	const FVector loc = GetActorLocation();
	_spawnTargetZ = loc.Z;

	_isRaised = false;
	_isSinking = false;
	_isTransitionalAlign = false;  // 상승 동안 정렬 단계 아님

	// 지면 아래에서 시작
	SetActorLocation(FVector(loc.X, loc.Y, loc.Z - kDepthCm));

	// 총구는 즉시 UpVector로 스냅(상승 중 계속 유지)
	_gunHousingZ_CurrentDeg = 90.0f;

	// 애님 인스턴스 전달
	_anim->SetBoneAngles(_rotatorZ_CurrentDeg, _gunHousingZ_CurrentDeg);

	_currentTarget = nullptr;
	AIStopFire();
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
}

void ASentryTurret::StartDescent()
{
	_isTransitionalAlign = true;

	_currentTarget = nullptr;
	AIStopFire();
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
}

void ASentryTurret::UpdateSpawnDescent(float deltaSeconds)
{
	const float kRiseSpeed = 300.0f;  // cm/s
	const float kSinkSpeed = 300.0f;  // cm/s
	const float kDepthCm = _mesh->GetSkeletalMeshAsset()->GetBounds().BoxExtent.Z * 2.0f;

	FVector loc = GetActorLocation();

	// 상승 단계: 위치만 상승, 총구는 항상 UpVector(90.0f)
	if (!_isRaised && !_isSinking && !_isTransitionalAlign)
	{
		const float newZ = FMath::Min(loc.Z + kRiseSpeed * deltaSeconds, _spawnTargetZ);
		SetActorLocation(FVector(loc.X, loc.Y, newZ));

		if (FMath::IsNearlyEqual(newZ, _spawnTargetZ, 0.5f))
		{
			// 상승 끝 → 전방 정렬 단계로 진입
			_isTransitionalAlign = true;
		}
		return;
	}

	// 상승 종료 직후 정렬 단계: 전방(0.0f)으로 "속도 제한 회전"만 수행
	if (_isTransitionalAlign && !_isRaised && !_isSinking)
	{
		const float desiredYawDeg = CalcYaw_Sentry();  // 필요 시 0.0f 고정 가능
		const float desiredPitchDeg = 0.0f;            // 전방

		ApplyAimSpeedLimit(deltaSeconds, desiredYawDeg, desiredPitchDeg);

		// 애님 인스턴스 전달
		_anim->SetBoneAngles(_rotatorZ_CurrentDeg, _gunHousingZ_CurrentDeg);

		if (IsAngleAligned(_gunHousingZ_CurrentDeg, 0.0f, 2.0f))
		{
			_isRaised = true;             // 전투 준비 완료
			_isTransitionalAlign = false; // 정렬 종료
			EnsureIdleTimer();
		}
		return;
	}

	// 전투 상태: 여기서는 위치/정렬을 건드리지 않음
	if (_isRaised && !_isSinking && !_isTransitionalAlign)
	{
		// 하강 조건(예: 탄약 소진) 감지 시, 먼저 Up 정렬 단계로 진입
		const bool wantsSink = (_curAmmo <= 0);
		if (wantsSink)
		{
			_isTransitionalAlign = true;  // Up 정렬 시작
			// _isSinking 은 정렬 완료 후로 미룹니다.
		}
		return;
	}

	// 하강 전 정렬 단계: UpVector(90.0f)로 "속도 제한 회전"만 수행
	if (_isTransitionalAlign && !_isSinking)
	{
		const float desiredYawDeg = CalcYaw_Sentry(); // 필요 시 0.0f
		const float desiredPitchDeg = 90.0f;          // 하늘

		ApplyAimSpeedLimit(deltaSeconds, desiredYawDeg, desiredPitchDeg);

		_anim->SetBoneAngles(_rotatorZ_CurrentDeg, _gunHousingZ_CurrentDeg);

		if (IsAngleAligned(_gunHousingZ_CurrentDeg, 90.0f, 2.0f))
		{
			_isSinking = true;            // 정렬 끝 → 실제 하강 시작
			_isTransitionalAlign = false;
		}
		return;
	}

	// 실제 하강: 위치만 하강(총구는 이미 UpVector로 정렬됨)
	if (_isSinking)
	{
		const float sinkZ = _spawnTargetZ - kDepthCm;
		const float newZ = FMath::Max(loc.Z - kSinkSpeed * deltaSeconds, sinkZ);
		SetActorLocation(FVector(loc.X, loc.Y, newZ));

		if (FMath::IsNearlyEqual(newZ, sinkZ, 0.5f))
		{
			Destroy();
		}
		return;
	}
}

// -------------------------------
/* 아이들 스캔 */
// -------------------------------

void ASentryTurret::EnsureIdleTimer()
{
	FTimerManager& tm = GetWorldTimerManager();

	if (IsTargetAttackable(_currentTarget))
	{
		tm.ClearTimer(_idleAimTimerHandle);
		return;
	}

	if (!tm.IsTimerActive(_idleAimTimerHandle))
	{
		OnIdleAimTimer(); // 즉시 1회
		tm.SetTimer(
			_idleAimTimerHandle,
			this,
			&ASentryTurret::OnIdleAimTimer,
			FMath::Max(0.1f, _idleScanInterval),
			true,
			_idleScanInterval
		);
	}
}

void ASentryTurret::OnIdleAimTimer()
{
	if (IsTargetAttackable(_currentTarget))
	{
		GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
		return;
	}

	if (!_muzzlePoint) return;

	const float kYawSweepDeg = 90.0f;
	const float kPitchSweepDeg = 30.0f;
	const float kDistanceCm = 4000.0f;

	const FRotator baseRot = _muzzlePoint->GetComponentRotation();
	const float yawDelta = FMath::FRandRange(-kYawSweepDeg, kYawSweepDeg);
	const float pitchDelta = FMath::FRandRange(-kPitchSweepDeg, kPitchSweepDeg);

	const FRotator offRot(pitchDelta, yawDelta, 0.0f);
	const FVector  dir = (baseRot + offRot).Vector().GetSafeNormal();

	_idleAimPointWS = _muzzlePoint->GetComponentLocation() + dir * kDistanceCm;
}

// -------------------------------
// HP/잔탄/기타
// -------------------------------

float ASentryTurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	_curHp -= DamageAmount;

	if (_curHp <= 0.0f)
	{
		//HandleOutOfAmmo();
		Destroy();
	}

	return DamageAmount;
}

void ASentryTurret::HandleOutOfAmmo()
{
	AIStopFire();
	_currentTarget = nullptr;
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
	StartDescent();
}

// -------------------------------
// 시야 제공 / Getter
// -------------------------------

void ASentryTurret::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	if (_muzzlePoint)
	{
		OutLocation = _muzzlePoint->GetComponentLocation();
		OutRotation = _muzzlePoint->GetComponentRotation();
	}
	else
	{
		OutLocation = GetActorLocation();
		OutRotation = GetActorRotation();
	}
}

FVector ASentryTurret::GetMuzzleLocation() const
{
	return _muzzlePoint ? _muzzlePoint->GetComponentLocation() : GetActorLocation();
}

FVector ASentryTurret::GetMuzzleForward() const
{
	return _muzzlePoint ? _muzzlePoint->GetForwardVector() : GetActorForwardVector();
}

FTransform ASentryTurret::GetMeshTransform() const
{
	return _mesh ? _mesh->GetComponentTransform() : GetActorTransform();
}

