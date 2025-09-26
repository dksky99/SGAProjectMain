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

#include "DrawDebugHelpers.h"

ASentryTurret::ASentryTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트
	_capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	_capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(_capsule);

	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	_mesh->SetupAttachment(_capsule);

	_muzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	// 어태치는 BeginPlay에서 소켓 스냅

	_muzzleFlashComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlash"));
	_muzzleFlashComponent->SetupAttachment(_mesh, TEXT("muzzleSocket"));
	_muzzleFlashComponent->bAutoActivate = false;

	_tracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tracer"));
	_tracerComponent->SetupAttachment(_mesh, TEXT("muzzleSocket"));
	_tracerComponent->bAutoActivate = false;

	// Perception
	_perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	_sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	_sightConfig->SightRadius = 3000.0f;
	_sightConfig->LoseSightRadius = 3500.0f;
	_sightConfig->PeripheralVisionAngleDegrees = 180.0f;
	_sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	_sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	_sightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	_perception->ConfigureSense(*_sightConfig);
	_perception->SetDominantSense(_sightConfig->GetSenseImplementation());

	_curAmmo = 0;
	_curHp = _maxHp;
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

	// 간단 스폰
	StartSpawnSimple();

	// 아이들 스캔 시작(초기 타깃 없음 가정)
	EnsureIdleTimer();
}

void ASentryTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpawnDescentSimple(DeltaTime);
	UpdateAimToTarget(DeltaTime);
	UpdateFireGate(DeltaTime);
}

// -------------------------------
// 외부 호출
// -------------------------------

void ASentryTurret::SetTargetActor(AActor* target)
{
	_currentTarget = target;
	EnsureIdleTimer();
}

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
	if (IsValid(_currentTarget))
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

	AActor* best = nullptr;
	float bestD2 = FLT_MAX;
	const FVector from = _muzzlePoint ? _muzzlePoint->GetComponentLocation() : GetActorLocation();

	for (AActor* actor : perceived)
	{
		if (!IsValid(actor)) continue;
		const float d2 = FVector::DistSquared(actor->GetActorLocation(), from);
		if (d2 < bestD2) { bestD2 = d2; best = actor; }
	}

	_currentTarget = best;
	EnsureIdleTimer();
}

// -------------------------------
// 조준(핵심): 두 축 속도 제한을 C++에서 통합
// -------------------------------

void ASentryTurret::BuildSmoothedYawTargetAndPitch(const FVector& trueTargetWS, float deltaSeconds, FVector& outYawLookAtWS, float& outPitchDeg, float& outYawErrDeg)
{
	// rotator(Yaw), gunhousing(Pitch)
	const FTransform yawW = _mesh->GetSocketTransform(_boneName_Yaw, RTS_World);
	const FTransform pitchW = _mesh->GetSocketTransform(_boneName_Pitch, RTS_World);

	const FVector O = yawW.GetLocation();
	const FVector G = pitchW.GetLocation();

	// 리그 기준 축(+X 전방 가정). +Y 전방 리그이면 벡터를 바꾸십시오.
	const FVector F0 = yawW.TransformVectorNoScale(FVector(1.0f, 0.0f, 0.0f)).GetSafeNormal(); // 전방(+X)
	const FVector R0 = yawW.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)).GetSafeNormal(); // 오른쪽(+Y)
	const FVector U0 = yawW.TransformVectorNoScale(FVector(0.0f, 0.0f, 1.0f)).GetSafeNormal(); // 업(+Z)

	// 1) Yaw 오차(도) 계산(수평 투영)
	const FVector toWS = trueTargetWS - O;
	const FVector toHorizWS = FVector::VectorPlaneProject(toWS, U0).GetSafeNormal();

	const float x = FVector::DotProduct(toHorizWS, F0);
	const float y = FVector::DotProduct(toHorizWS, R0);
	outYawErrDeg = FMath::RadiansToDegrees(FMath::Atan2(y, x));

	// 2) Yaw 속도 제한 → 누적 상태 업데이트
	const float yawStep = FMath::Clamp(outYawErrDeg, -_yawSpeedDegPerSec * deltaSeconds, _yawSpeedDegPerSec * deltaSeconds);
	_aimYawDeg = FMath::Clamp(FMath::UnwindDegrees(_aimYawDeg + yawStep), -_aimYawLimitDeg, _aimYawLimitDeg);

	// 3) 누적 Yaw로 부드러운 수평 전방 생성 → LookAt 타깃(수평만)
	const float yawRad = FMath::DegreesToRadians(_aimYawDeg);
	const FVector smoothFwd = (F0 * FMath::Cos(yawRad) + R0 * FMath::Sin(yawRad)).GetSafeNormal();

	const float kLookDist = 3000.0f;
	outYawLookAtWS = O + smoothFwd * kLookDist;

	// 4) Pitch 목표각 계산(부모의 수평 전방 smoothFwd 기준)
	const FVector toT = trueTargetWS - G;
	float alongFwd = FVector::DotProduct(toT, smoothFwd); // 전방 성분
	const float dz = FVector::DotProduct(toT, U0);       // 높이 성분

	// 뒤/특이점 폭주 방지
	const float kMinAlong = 30.0f;
	if (FMath::Abs(alongFwd) < kMinAlong)
	{
		alongFwd = (alongFwd >= 0.0f ? kMinAlong : -kMinAlong);
	}

	float desiredPitch = FMath::RadiansToDegrees(FMath::Atan2(dz, FMath::Abs(alongFwd)));

	// (옵션) Yaw 게이트: Yaw가 크게 어긋나 있으면 Pitch 영향 낮춤
	const float kYawGateDeg = 50.0f;
	const float pitchScale = FMath::Clamp(1.0f - (FMath::Abs(outYawErrDeg) / kYawGateDeg), 0.0f, 1.0f);
	desiredPitch *= pitchScale;

	// 5) Pitch 속도 제한 + 한계
	const float dPitch = FMath::FindDeltaAngleDegrees(_aimPitchDeg, desiredPitch);
	const float stepPitch = FMath::Clamp(dPitch, -_pitchSpeedDegPerSec * deltaSeconds, _pitchSpeedDegPerSec * deltaSeconds);

	_aimPitchDeg = FMath::Clamp(FMath::UnwindDegrees(_aimPitchDeg + stepPitch), -_aimPitchDownDeg, _aimPitchUpDeg);
	outPitchDeg = _aimPitchDeg;
}

void ASentryTurret::UpdateAimToTarget(float deltaSeconds)
{
	if (!_mesh || !_muzzlePoint) return;

	// 타깃 결정(없으면 아이들 포인트)
	const FVector muzzleLoc = _muzzlePoint->GetComponentLocation();

	const FVector trueTargetWS = IsValid(_currentTarget)
		? _currentTarget->GetActorLocation()
		: (_idleAimPointWS.IsNearlyZero() ? (muzzleLoc + _muzzlePoint->GetForwardVector() * 3000.0f) : _idleAimPointWS);

	// C++ 한 곳에서 두 축 속도 제한 → 결과 전달
	FVector yawLookAtWS = FVector::ZeroVector;
	float   outPitchDeg = 0.0f;
	float   yawErrDeg = 0.0f;

	BuildSmoothedYawTargetAndPitch(trueTargetWS, deltaSeconds, yawLookAtWS, outPitchDeg, yawErrDeg);

	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		if (USentryAnimInstance* si = Cast<USentryAnimInstance>(anim))
		{
			// rotator: LookAt(Location) = yawLookAtWS (AnimBP에서 Interp Off 권장)
			si->SetLookAtYawTargetWS(yawLookAtWS);

			// gunhousing: Transform(Modify) Bone(본 스페이스, 힌지축 1개) = outPitchDeg
			si->SetPitchDeg(outPitchDeg);
		}
	}
}

// -------------------------------
// 발사 게이트/LOS
// -------------------------------

void ASentryTurret::UpdateFireGate(float deltaSeconds)
{
	if (!_currentTarget || !IsValid(_currentTarget) || !_muzzlePoint)
	{
		if (_lastWantsFire) { _lastWantsFire = false; AIStopFire(); }
		return;
	}

	// 허용오차 코사인 캐시 갱신
	if (!FMath::IsNearlyEqual(_cachedAimTolDeg, _aimToleranceDeg, KINDA_SMALL_NUMBER))
	{
		_cachedAimTolDeg = _aimToleranceDeg;
		_cosAimTol = FMath::Cos(FMath::DegreesToRadians(_cachedAimTolDeg));
	}

	const FVector muzzleLoc = _muzzlePoint->GetComponentLocation();
	const FVector muzzleFwd = _muzzlePoint->GetForwardVector();
	const FVector toVec = _currentTarget->GetActorLocation() - muzzleLoc;

	bool bAimed = false;
	if (toVec.IsNearlyZero())
	{
		bAimed = true;
	}
	else
	{
		const FVector toDir = toVec.GetSafeNormal();
		const float dot = FVector::DotProduct(muzzleFwd, toDir);
		bAimed = (dot >= (_cosAimTol - KINDA_SMALL_NUMBER));
	}

	// LOS 갱신(레이트 리밋)
	if (bAimed)
	{
		if (_losCooldown <= 0.0f)
		{
			_cachedHasLOS = HasLineOfFire(muzzleLoc, _currentTarget->GetActorLocation());
			_losCooldown = _losCheckInterval;
		}
		else
		{
			_losCooldown -= deltaSeconds;
		}
	}
	else
	{
		_cachedHasLOS = false;
	}

	const bool wantsFire = bAimed && _cachedHasLOS;
	if (wantsFire != _lastWantsFire)
	{
		_lastWantsFire = wantsFire;
		wantsFire ? AIStartFire() : AIStopFire();
	}
}

bool ASentryTurret::HasLineOfFire(const FVector& from, const FVector& to) const
{
	FHitResult hit;
	FCollisionQueryParams params(NAME_None, true, this);
	params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(hit, from, to, ECC_Visibility, params);

	// 맞은 게 없거나, 맞은 것이 현재 타깃이면 통과
	return !bHit || (hit.GetActor() == _currentTarget);
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

	//SpawnBullet(muzzleLocation, fireDirection);
	PlayMuzzleFlash();

	_curAmmo--;
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}
}

void ASentryTurret::SpawnBullet(const FVector& muzzleLocation, const FVector& direction)
{
	if (!_bulletClass) return;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();

	const FRotator spawnRot = direction.Rotation();

	AGunBulletBase* bullet = GetWorld()->SpawnActor<AGunBulletBase>(
		_bulletClass,
		muzzleLocation,
		spawnRot,
		spawnParams
	);

	if (bullet)
	{
		bullet->InitializeProjectile();
	}
}

void ASentryTurret::PlayMuzzleFlash()
{
	if (_muzzleFlashComponent)
	{
		_muzzleFlashComponent->Deactivate();
		_muzzleFlashComponent->Activate(true);
	}
}

void ASentryTurret::PlayTracer(const FVector& endPoint)
{
	if (_tracerComponent)
	{
		_tracerComponent->SetVectorParameter(TEXT("BeamEnd"), endPoint);
		_tracerComponent->Deactivate();
		_tracerComponent->Activate(true);
	}
}

// -------------------------------
// 스폰/디스폰(간단)
// -------------------------------

void ASentryTurret::StartSpawnSimple()
{
	const float kDepthCm = 120.0f;
	const float kPitchSpawnDeg = 90.0f;

	const FVector loc = GetActorLocation();
	_spawnTargetZ = loc.Z;
	_isRaised = false;

	SetActorLocation(FVector(loc.X, loc.Y, loc.Z - kDepthCm));
	_aimPitchDeg = kPitchSpawnDeg;

	_currentTarget = nullptr;
	AIStopFire();
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
}

void ASentryTurret::StartDescentSimple()
{
	_currentTarget = nullptr;
	AIStopFire();
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
}

void ASentryTurret::UpdateSpawnDescentSimple(float deltaSeconds)
{
	const float kRiseSpeed = 300.0f;
	const float kSinkSpeed = 300.0f;
	const float kPitchLerpSpeed = 6.0f;
	const float kPitchSpawnDeg = 90.0f;
	const float kDepthCm = 120.0f;

	FVector loc = GetActorLocation();

	// 상승 중
	if (!_isRaised)
	{
		const float newZ = FMath::Min(loc.Z + kRiseSpeed * deltaSeconds, _spawnTargetZ);
		SetActorLocation(FVector(loc.X, loc.Y, newZ));
		_aimPitchDeg = FMath::FInterpTo(_aimPitchDeg, kPitchSpawnDeg, deltaSeconds, kPitchLerpSpeed);

		if (FMath::IsNearlyEqual(newZ, _spawnTargetZ, 0.5f))
		{
			_isRaised = true;
			EnsureIdleTimer();
		}
		return;
	}

	// 하강 조건(탄약 소진 등)
	const bool wantsSink = _curAmmo <= 0;
	if (wantsSink)
	{
		_aimPitchDeg = FMath::FInterpTo(_aimPitchDeg, kPitchSpawnDeg, deltaSeconds, kPitchLerpSpeed);

		const float groundZ = _spawnTargetZ;
		const float sinkZ = groundZ - kDepthCm;
		const float newZ = FMath::Max(loc.Z - kSinkSpeed * deltaSeconds, sinkZ);
		SetActorLocation(FVector(loc.X, loc.Y, newZ));

		if (FMath::IsNearlyEqual(newZ, sinkZ, 0.5f))
		{
			Destroy();
		}
	}
}

// -------------------------------
/* 아이들 스캔 */
// -------------------------------

void ASentryTurret::EnsureIdleTimer()
{
	FTimerManager& tm = GetWorldTimerManager();

	if (IsValid(_currentTarget))
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
	if (IsValid(_currentTarget))
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
		HandleOutOfAmmo();
	}

	return DamageAmount;
}

void ASentryTurret::HandleOutOfAmmo()
{
	AIStopFire();
	_currentTarget = nullptr;
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
	StartDescentSimple();
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
