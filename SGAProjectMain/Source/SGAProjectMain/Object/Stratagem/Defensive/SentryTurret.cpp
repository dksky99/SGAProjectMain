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

#include "../../../Gun/GunBulletBase.h"
#include "../../../SGAProjectMain.h"

// Sets default values
ASentryTurret::ASentryTurret()
{
	PrimaryActorTick.bCanEverTick = true;
	
	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

	_curAmmo = 0;

	// 머즐 플래시 컴포넌트를 muzzle 본에 직접 어태치
	_muzzleFlashComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MuzzleFlash"));
	_muzzleFlashComponent->SetupAttachment(_mesh, TEXT("muzzle"));  // “muzzle” 본 이름 사용
	_muzzleFlashComponent->bAutoActivate = false;

	// 트레이서 컴포넌트를 muzzle 본에 어태치
	_tracerComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Tracer"));
	_tracerComponent->SetupAttachment(_mesh, TEXT("muzzle"));
	_tracerComponent->bAutoActivate = false;

	// 머즐 포인트 컴포넌트 생성
	_muzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	_muzzlePoint->SetupAttachment(_mesh, TEXT("muzzle"));
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
	// 이미 타이머가 실행 중이면 재시작하지 않음
	if (GetWorld()->GetTimerManager().IsTimerActive(_fireTimerHandle))
	{
		return;
	}

	// 일정 간격마다 Fire() 호출
	GetWorld()->GetTimerManager().SetTimer
	(
		_fireTimerHandle,
		this,
		&ASentryTurret::Fire,
		_fireInterval,
		true
	);
}

void ASentryTurret::AIStopFire()
{
	// 타이머가 유효하다면 해제
	GetWorldTimerManager().ClearTimer(_fireTimerHandle);
}

void ASentryTurret::Fire()
{
	// 탄약이 없으면 파괴 처리
	if (_curAmmo <= 0)
	{
		HandleOutOfAmmo();
		return;
	}


	// 총알 스폰
	// 머즐 위치와 방향 가져오기
	FVector muzzleLocation = _muzzlePoint->GetComponentLocation();
	FVector fireDirection = _muzzlePoint->GetForwardVector();
	SpawnBullet(muzzleLocation, fireDirection);

	// 이펙트 재생
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

	// 스폰 파라미터 설정
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();

	// 회전 값 계산
	FRotator spawnRot = direction.Rotation();

	// 탄환 스폰
	AGunBulletBase* bullet = GetWorld()->SpawnActor<AGunBulletBase>
		(
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


