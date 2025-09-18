// SentryTurret.cpp

#include "SentryTurret.h"

#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "Animation/AnimInstance.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionSystem.h"

#include "../../../Gun/GunBulletBase.h"
#include "SentryAnimInstance.h"
#include "../../../../../../../Source/Runtime/Engine/Classes/Components/CapsuleComponent.h"

ASentryTurret::ASentryTurret()
{
	PrimaryActorTick.bCanEverTick = true;

	_capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	_capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(_capsule);

	// 메시
	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	_mesh->SetupAttachment(_capsule);

	// 머즐 포인트 (muzzle 본 기준)
	_muzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	// 어태치는 BeginPlay에서

	// 머즐 플래시
	_muzzleFlashComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlash"));
	_muzzleFlashComponent->SetupAttachment(_mesh, TEXT("muzzleSocket"));
	_muzzleFlashComponent->bAutoActivate = false;

	// 트레이서
	_tracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tracer"));
	_tracerComponent->SetupAttachment(_mesh, TEXT("muzzleSocket"));
	_tracerComponent->bAutoActivate = false;

	// Perception 구성
	_perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	_sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	_sightConfig->SightRadius = 3000.0f;							// 발견 반경(cm).
	_sightConfig->LoseSightRadius = 3500.0f;						// 시야 상실 반경.
	_sightConfig->PeripheralVisionAngleDegrees = 180.0f;			// 시야각(도). 이 값은 “한쪽” 각도이므로 실제 총 시야각은 약 140°(= 70×2) 입니다.
	_sightConfig->DetectionByAffiliation.bDetectEnemies = true;		// 적
	_sightConfig->DetectionByAffiliation.bDetectFriendlies = true;	// 아군
	_sightConfig->DetectionByAffiliation.bDetectNeutrals = true;	// 중립을 모두 감지 대상으로 허용합니다.

	_perception->ConfigureSense(*_sightConfig);								// PerceptionComponent에 “시야” 감각을 추가/구성합니다.
	_perception->SetDominantSense(_sightConfig->GetSenseImplementation());	// 대표 감각을 시야로 지정합니다.

	// 초기값
	_curAmmo = 0;
	_curHp = _maxHp;
}

void ASentryTurret::BeginPlay()
{
	Super::BeginPlay();

	_curAmmo = _maxAmmo;
	_curHp = _maxHp;

	if (_muzzlePoint)
	{
		_muzzlePoint->AttachToComponent(
			_mesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,  // 위치/회전 소켓과 일치
			TEXT("muzzleSocket")
		);
	}


	if (_perception)
	{
		_perception->OnPerceptionUpdated.AddDynamic(this, &ASentryTurret::OnPerceptionUpdated);
	}

	_cachedAimTolDeg = _aimToleranceDeg;
	_cosAimTol = FMath::Cos(FMath::DegreesToRadians(_cachedAimTolDeg));

	if (_deployMontage)
	{
		if (UAnimInstance* anim = _mesh->GetAnimInstance())
		{
			anim->OnMontageEnded.AddDynamic(this, &ASentryTurret::OnDeployMontageEnded);
			PlayDeployMontage(true); // 처음→끝
		}
	}
}

void ASentryTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAimToTarget(DeltaTime);
	UpdateFireGate(DeltaTime);
}

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

	tm.SetTimer(
		_fireTimerHandle,
		this,
		&ASentryTurret::Fire,
		_fireInterval,
		true,
		0.0f
	);
}

void ASentryTurret::AIStopFire()
{
	GetWorldTimerManager().ClearTimer(_fireTimerHandle);
}

void ASentryTurret::OnPerceptionUpdated(const TArray<AActor*>& /*UpdatedActors*/)
{
	UpdateTargetSelection();
}

void ASentryTurret::UpdateTargetSelection()
{
	if (!_perception)
		return;

	// 현재 타깃이 계속 보이는 중이면 유지
	if (IsValid(_currentTarget))
	{
		FActorPerceptionBlueprintInfo info;
		if (_perception->GetActorsPerception(_currentTarget, info))
			// GetActorsPerception은 AI Perception Component가 특정 액터를 어떻게 인지했는지(어떤 센스로, 성공했는지, 마지막 위치는 어디인지 등) 를 가져오는 함수입니다.
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

	// 새 타깃 선정: 현재 보이는 것 중 '머즐에서' 가장 가까운
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

void ASentryTurret::UpdateAimToTarget(float deltaSeconds)
{
	if (!_mesh || !_muzzlePoint) return;

	// 몽타주 재생 중에는 내부 조준각을 갱신하지 않음(드리프트 방지)
	if (IsDeployMontagePlaying())
	{
		if (UAnimInstance* anim = _mesh->GetAnimInstance())
		{
			if (USentryAnimInstance* si = Cast<USentryAnimInstance>(anim))
			{
				const float newAlpha = FMath::FInterpTo(si->GetAimAlpha(), 0.0f, deltaSeconds, 12.0f);
				si->SetAimAlpha(newAlpha);
			}
		}
		return;
	}

	float desiredYawRaw = _aimYawDeg;
	float desiredPitchRaw = _aimPitchDeg;

	const FVector muzzleLoc = _muzzlePoint->GetComponentLocation();
	FVector toTarget = FVector::ZeroVector;

	if (IsValid(_currentTarget))
	{
		// 타깃 추종: 아이들 타이머는 꺼 둡니다
		toTarget = _currentTarget->GetActorLocation() - muzzleLoc;

		if (GetWorldTimerManager().IsTimerActive(_idleAimTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
		}
	}
	else
	{
		// 타깃 없음: 아이들 목표 사용(캐시가 비어 있으면 즉시 1회 생성)
		if (_idleAimPointWS.IsNearlyZero())
		{
			OnIdleAimTimer();
		}
		toTarget = _idleAimPointWS - muzzleLoc;

		// 타이머가 꺼져 있으면 켭니다
		if (!GetWorldTimerManager().IsTimerActive(_idleAimTimerHandle))
		{
			EnsureIdleTimer();
		}
	}

	if (!toTarget.IsNearlyZero())
	{
		// 월드 → 머즐 로컬: 현재 총구 축 기준의 방향 오차를 구함
		const FVector    dirWS = toTarget.GetSafeNormal();
		const FTransform muzzleXf = _muzzlePoint->GetComponentTransform();
		const FVector    dirMS = muzzleXf.InverseTransformVectorNoScale(dirWS).GetSafeNormal();

		// 오차각: 지금 +X에서 타깃까지 얼마나 더 돌아야 하는지
		const float yawErrDeg = FMath::RadiansToDegrees(FMath::Atan2(dirMS.Y, dirMS.X));

		const float planarLen = FVector(dirMS.X, dirMS.Y, 0.0f).Size();
		const float pitchErrDeg = FMath::RadiansToDegrees(
			FMath::Atan2(dirMS.Z, FMath::Max(planarLen, SMALL_NUMBER))
		);

		// 목표 절대각 = 현재각 + 오차각 (기구 한계 클램프)
		desiredYawRaw = FMath::Clamp(FMath::UnwindDegrees(_aimYawDeg + yawErrDeg), -_aimYawLimitDeg, _aimYawLimitDeg);
		desiredPitchRaw = FMath::Clamp(FMath::UnwindDegrees(_aimPitchDeg + pitchErrDeg), -_aimPitchDownDeg, _aimPitchUpDeg);
	}

	// 최단호 + 각속도 제한으로 스무스 추종
	const float deltaYaw = FMath::FindDeltaAngleDegrees(_aimYawDeg, desiredYawRaw);
	const float deltaPitch = FMath::FindDeltaAngleDegrees(_aimPitchDeg, desiredPitchRaw);

	const float yawStep = FMath::Clamp(deltaYaw, -_yawSpeedDegPerSec * deltaSeconds, _yawSpeedDegPerSec * deltaSeconds);
	const float pitchStep = FMath::Clamp(deltaPitch, -_pitchSpeedDegPerSec * deltaSeconds, _pitchSpeedDegPerSec * deltaSeconds);

	_aimYawDeg = FMath::UnwindDegrees(_aimYawDeg + yawStep);
	_aimPitchDeg = FMath::Clamp(FMath::UnwindDegrees(_aimPitchDeg + pitchStep), -_aimPitchDownDeg, _aimPitchUpDeg);

	// AnimBP 전달
	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		if (USentryAnimInstance* si = Cast<USentryAnimInstance>(anim))
		{
			si->SetAimAngles(_aimYawDeg, _aimPitchDeg);

			const float targetAlpha = IsDeployMontagePlaying() ? 0.0f : 1.0f;
			const float newAlpha = FMath::FInterpTo(si->GetAimAlpha(), targetAlpha, deltaSeconds, 8.0f);
			si->SetAimAlpha(newAlpha);
		}
	}
}

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

	// 조준 OK일 때만, 일정 주기로 LOS 갱신
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

float ASentryTurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	_curHp -= DamageAmount;

	if (_curHp <= 0.0f)
	{
		Destroy();
	}

	return DamageAmount;
}

void ASentryTurret::HandleOutOfAmmo()
{
	AIStopFire();
	_currentTarget = nullptr;                 // 조준 영향 제거(AnimBP가 AimAlpha를 0으로 수렴)
	GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);

	if (_deployMontage && _mesh && _mesh->GetAnimInstance())
	{
		PlayDeployMontage(false);             // 끝→처음 역재생
		return;                               // 파괴는 콜백에서
	}

	Destroy();
}

void ASentryTurret::EnsureIdleTimer()
{
	FTimerManager& tm = GetWorldTimerManager();

	// 타깃이 있으면 아이들 타이머는 불필요
	if (IsValid(_currentTarget))
	{
		tm.ClearTimer(_idleAimTimerHandle);
		return;
	}

	// 타깃이 없으면 주기적으로 임의 목표점을 갱신
	if (!tm.IsTimerActive(_idleAimTimerHandle))
	{
		// 첫 목표를 즉시 한 번 만들고, 이후 반복 예약
		OnIdleAimTimer();
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
	// 타깃이 생겼으면 즉시 중지
	if (IsValid(_currentTarget))
	{
		GetWorldTimerManager().ClearTimer(_idleAimTimerHandle);
		return;
	}

	if (!_muzzlePoint) return;

	constexpr float kYawSweepDeg = 90.0f;   // 좌우 범위
	constexpr float kPitchSweepDeg = 30.0f;    // 미세 위아래
	constexpr float kDistanceCm = 4000.0f; // 목표점 거리

	const FRotator baseRot = _muzzlePoint->GetComponentRotation();
	const float    yawDelta = FMath::FRandRange(-kYawSweepDeg, kYawSweepDeg);
	const float    pitchDelta = FMath::FRandRange(-kPitchSweepDeg, kPitchSweepDeg);

	const FRotator offRot(pitchDelta, yawDelta, 0.0f);
	const FVector  dir = (baseRot + offRot).Vector().GetSafeNormal();

	_idleAimPointWS = _muzzlePoint->GetComponentLocation() + dir * kDistanceCm;
}

void ASentryTurret::OnDeployMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != _deployMontage) return;

	if (_hasPlayedDeployMontage) Destroy();

	// 정방향 종료 → 아이들/탐색 재개
	if (!IsValid(_currentTarget))
	{
		_hasPlayedDeployMontage = true;
		EnsureIdleTimer(); // 총구가 아이들 스캔으로 자연스럽게 움직임
	}
}

void ASentryTurret::PlayDeployMontage(bool bForward)
{
	if (!_mesh) return;
	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		if (bForward)
		{
			// 정방향
			anim->Montage_Play(_deployMontage, 1.0f);
		}
		else
		{
			// 역방향 : 음수 속도로 플레이 → 곧바로 끝으로 점프
			const float len = _deployMontage->GetPlayLength();
			const float endPos = FMath::Max(0.0f, len - 0.001f); // 끝에서 조금은 떨어져 있어야함

			anim->Montage_Play(_deployMontage, -1.0f);
			anim->Montage_SetPosition(_deployMontage, endPos);
		}
	}
}

bool ASentryTurret::IsDeployMontagePlaying() const
{
	if (!_mesh || !_deployMontage) return false;
	if (UAnimInstance* anim = _mesh->GetAnimInstance())
	{
		return anim->Montage_IsPlaying(_deployMontage);
	}
	return false;
}

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
