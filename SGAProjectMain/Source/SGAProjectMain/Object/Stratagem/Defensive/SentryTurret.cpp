// Fill out your copyright notice in the Description page of Project Settings.

#include "SentryTurret.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h" 
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ASentryTurret::ASentryTurret()
{
	PrimaryActorTick.bCanEverTick = true;
	// ─────────── 터렛 몸통(Static Mesh) ───────────
	_bodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = _bodyMesh;
	_bodyMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// ─────────── 터렛 총구(Static Mesh) ───────────
	_barrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	_barrelMesh->SetupAttachment(_bodyMesh);
	_barrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	_curAmmo = 0;

	// 머즐 플래시 컴포넌트
	_muzzleFlashComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlashComponent"));
	_muzzleFlashComponent->SetupAttachment(_barrelMesh);
	_muzzleFlashComponent->bAutoActivate = false;
	// 에디터에서 해당 컴포넌트의 Template(ParticleSystem)을 할당해야 합니다.

	// 트레이서 컴포넌트
	_tracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TracerComponent"));
	_tracerComponent->SetupAttachment(_barrelMesh);
	_tracerComponent->bAutoActivate = false;
	// 에디터에서 해당 컴포넌트의 Asset(NiagaraSystem)을 할당해야 합니다.
}

void ASentryTurret::BeginPlay()
{
	Super::BeginPlay();

	// 블루프린트에서 설정된 _maxAmmo 값을 그대로 사용하여 _curAmmo 초기화
	_curAmmo = _maxAmmo;
	_curHp = _maxHp;
}

void ASentryTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASentryTurret::AIStartFire()
{
	// 이미 타이머가 활성화되어 있거나, fireInterval이 0 이하일 경우 리턴
	if (_fireTimerHandle.IsValid() || _sentryData._fireInterval <= 0.0f)
	{
		return;
	}

	// SetTimer로 _sentryData._fireInterval 간격마다 Fire() 호출을 반복 설정
	GetWorldTimerManager().SetTimer(
		_fireTimerHandle,
		this,
		&ASentryTurret::Fire,
		_sentryData._fireInterval,
		true,   // 반복 모드
		0.0f    // 첫 호출 딜레이
	);
}

void ASentryTurret::AIStopFire()
{
	// 타이머가 유효하다면 해제
	GetWorldTimerManager().ClearTimer(_fireTimerHandle);
}

void ASentryTurret::Fire()
{
	// (0) 탄약이 없으면 파괴 처리
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}

	// (1) 라인트레이스를 수행해서 Hit 정보를 가져온다.
	FHitResult Hit = GetHitResult();

	// (2) Start → End 지점을 삼항 연산으로 정해 디버그 라인 그리기
	FVector Start = Hit.TraceStart;
	FVector End = Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd;

	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		false,   // Depth Test 불필요
		1.0f,    // 1초 동안 표시
		0,       // 스크린 오프셋 없음
		2.0f     // 선 두께
	);

	// 머즐 플래시 재생
	PlayMuzzleFlash();

	// 트레이서 재생
	PlayTracer(End);

	// (3) Hit이 발생한 경우에만 데미지 계산 → 대미지 적용
	if (Hit.bBlockingHit)
	{
		// TraceStart와 ImpactPoint 사이의 거리(cm) → m 단위로 변환
		float DistanceCm = FVector::Dist(Hit.TraceStart, Hit.ImpactPoint);
		float DistanceMeters = DistanceCm / 100.0f;

		// CalculateDamage() 호출
		float DamageToApply = CalculateDamage(DistanceMeters);

		// ApplyDamage 호출 예시
		ApplyHitDamage(Hit, DamageToApply);
	}

	_curAmmo--;
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}
}

FHitResult ASentryTurret::GetHitResult() const
{
	FHitResult Hit;

	// 총구 소켓이 없으므로 Actor 위치 & 회전으로 시작점과 방향을 잡음
	FVector TraceStart = GetActorLocation();
	FVector TraceDirection = GetActorForwardVector();  // Actor의 정면
	FVector TraceEnd = TraceStart + (TraceDirection * _range);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel
	(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		Params
	);

	// 나중 디버그 라인용으로 보관
	Hit.TraceStart = TraceStart;
	Hit.TraceEnd = TraceEnd;
	return Hit;
}

float ASentryTurret::CalculateDamage(float distance) const
{
	// DistanceMeters 기준 거리별 데미지 감쇠 계산
	// (SentryData._falloff25 / _falloff50 / _falloff100 사용)
	if (distance <= 25.0f)
	{
		float Alpha = distance / 25.0f;
		float Falloff = FMath::Lerp(0.0f, _sentryData._falloff25, Alpha);
		return _sentryData._baseDamage * (1.0f - Falloff);
	}
	else if (distance <= 50.0f)
	{
		float Alpha = (distance - 25.0f) / 25.0f;
		float Falloff = FMath::Lerp(_sentryData._falloff25, _sentryData._falloff50, Alpha);
		return _sentryData._baseDamage * (1.0f - Falloff);
	}
	else if (distance <= 100.0f)
	{
		float Alpha = (distance - 50.0f) / 50.0f;
		float Falloff = FMath::Lerp(_sentryData._falloff50, _sentryData._falloff100, Alpha);
		return _sentryData._baseDamage * (1.0f - Falloff);
	}
	else
	{
		return _sentryData._baseDamage * (1.0f - _sentryData._falloff100);
	}
}

void ASentryTurret::ApplyHitDamage(const FHitResult& hit, float damage) 
{
	// (1) 명중한 액터 가져오기
	AActor* HitActor = hit.GetActor();
	if (!HitActor)
	{
		return;
	}

	// (2) Unreal의 데미지 시스템 사용 예시
	UGameplayStatics::ApplyDamage(
		HitActor,
		damage,
		GetInstigatorController(), // 발사자를 전달(없으면 nullptr)
		this,                      // 데미지를 준 주체(=this)
		UDamageType::StaticClass() // 기본 데미지 타입
	);

	// 원한다면, 맞은 액터가 파괴 가능한 인터페이스를 구현했다면 호출하거나,
	// 테스트용으로 Destroy()를 바로 해 버려도 됩니다.
	// 예시:
	// HitActor->Destroy();
}

void ASentryTurret::PlayMuzzleFlash()
{
	if (_muzzleFlashComponent)
	{
		_muzzleFlashComponent->Deactivate();
		_muzzleFlashComponent->Activate(true);
	}
}

void ASentryTurret::PlayTracer(const FVector& EndPoint)
{
	if (_tracerComponent)
	{
		_tracerComponent->SetVectorParameter(TEXT("BeamEnd"), EndPoint);
		_tracerComponent->Deactivate();
		_tracerComponent->Activate(true);
	}
}

float ASentryTurret::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 데미지 적용
	_curHp -= DamageAmount;

	// 체력 0 이하 시 파괴 로직
	if (_curHp <= 0.0f)
	{
		HandleOutOfAmmo();
	}

	return DamageAmount;
}

void ASentryTurret::HandleOutOfAmmo()
{
	AIStopFire();
	Destroy();
}


