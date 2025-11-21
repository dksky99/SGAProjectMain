// Fill out your copyright notice in the Description page of Project Settings.


#include "DropPod.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DamageEvents.h"

#include "HellPodBase.h"
#include "../Defensive/SentryTurret.h"
#include "../../../SGAProjectMain.h"

// Sets default values
ADropPod::ADropPod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	_mesh->SetNotifyRigidBodyCollision(true);
	_mesh->SetGenerateOverlapEvents(true);

	RootComponent = _mesh;

	_projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	_projectile->ProjectileGravityScale = 0.0f;
	_projectile->InitialSpeed = 0.f;
	_projectile->MaxSpeed = 10000.f;
	_projectile->bRotationFollowsVelocity = false;
	_projectile->UpdatedComponent = _mesh;
}

// Called when the game starts or when spawned
void ADropPod::BeginPlay()
{
	Super::BeginPlay();

	if (_mesh)
	{
		_mesh->OnComponentHit.AddDynamic(this, &ADropPod::OnHit);

		_mesh->OnComponentBeginOverlap.AddDynamic(this, &ADropPod::OnBeginOverlap);

		_mesh->SetNotifyRigidBodyCollision(true);

		_mesh->SetCollisionResponseToChannel(ECC_GameDamage, ECR_Overlap);
	}
	
}

void ADropPod::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!_isAlreadySpawned && IsLandableSurface(OtherActor, OtherComp))
	{
		_isGrounded = true;
		_isAlreadySpawned = true;

		HandleGroundLanding(Hit.ImpactPoint);
	}
}

void ADropPod::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherComp)
	{
		const bool bIsHitBoxProfile = (OtherComp->GetCollisionProfileName() == FName(TEXT("HitBox")));

		if (bIsHitBoxProfile)
		{
			if (_damagedCharacters.Contains(OtherActor))
			{
				return;
			}
			_damagedCharacters.Add(OtherActor);

			// 임팩트 좌표 폴백(스윕이 아닐 때를 대비)
			const FVector impact =
				(bFromSweep && SweepResult.bBlockingHit)
				? FVector(SweepResult.ImpactPoint)
				: FVector(OtherComp ? OtherComp->GetComponentLocation() : GetActorLocation());

			const FVector shotDirection = (impact - GetActorLocation()).GetSafeNormal();

			FHitResult hitInfo;
			if (bFromSweep && SweepResult.bBlockingHit)
			{
				hitInfo = SweepResult;
			}
			else
			{
				// 최소 유효 정보만 세팅
				hitInfo.Component = OtherComp;
				hitInfo.ImpactPoint = impact;
				hitInfo.Location = impact;
				hitInfo.bBlockingHit = false;
			}

			FCDamageEvent damageEvent;

			// FPointDamageEvent 기반 정보
			damageEvent.HitInfo = hitInfo;
			damageEvent.ShotDirection = shotDirection;

			// 커스텀 필드
			damageEvent.BaseDamage = _damage;
			damageEvent.DurabilityDamage = 0;
			damageEvent.DemolitionDamage = 0;
			damageEvent.PenetrationLevel = 10;

			damageEvent.IsExplosionDamage = false;
			damageEvent.ColComp = OtherComp;

			damageEvent.DamageTypeClass = UCDamageType::StaticClass();

			const float finalDamage = static_cast<float>(damageEvent.BaseDamage);

			OtherActor->TakeDamage(
				finalDamage,
				damageEvent,
				GetInstigatorController(),
				this
			);
		}
	}
}

bool ADropPod::IsLandableSurface(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (!OtherActor || !OtherComp)
		return false;

	// 캐릭터면 착지 불가
	if (OtherActor->IsA<ACharacter>())
		return false;

	// 센트리면 착지 불가
	if (OtherActor->IsA<ASentryTurret>())
		return false;

	// 이동 가능한 물체(아이템/소품 등)면 착지 불가
	const UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
	const bool compSim = OtherComp->IsSimulatingPhysics();
	const bool rootSim = (rootPrim && rootPrim->IsSimulatingPhysics());
	const bool compMov = (OtherComp->Mobility == EComponentMobility::Movable);
	const bool rootMov = (rootPrim && rootPrim->Mobility == EComponentMobility::Movable);
	const ECollisionChannel objType = OtherComp->GetCollisionObjectType();

	// WorldDynamic/PhysicsBody 이면서 물리 시뮬 또는 Movable이면 이동 가능 대상으로 간주
	const bool isDynamic = (objType == ECC_WorldDynamic || objType == ECC_PhysicsBody);
	if (isDynamic && (compSim || rootSim || compMov || rootMov))
		return false;

	// 나머지는 착지 가능
	return true;
}

void ADropPod::LaunchOverlappedActors(const FVector& hitPoint)
{
	FVector origin = hitPoint;
	FVector extent = _mesh->Bounds.BoxExtent;

	TArray<FOverlapResult> overlaps;
	FCollisionShape box = FCollisionShape::MakeBox(extent);
	FCollisionQueryParams params;
	params.AddIgnoredActor(this);

	// 이미 튕겨낸 액터 저장용
	TSet<AActor*> ejectedActors;

	// 헬포드 전체 높이 계산 (cm 단위)
	float hellpodHeight = extent.Z * 2.0f;
	constexpr float g = 980.f;  // 중력 가속도(cm/s²)
	// 헬포드 높이에 도달하기 위한 최소 초기속도
	float minVelocity = FMath::Sqrt(2.f * g * hellpodHeight);

	ECollisionChannel channelsToCheck[] = { ECC_Pawn, ECC_PhysicsBody };
	for (ECollisionChannel channel : channelsToCheck)
	{
		if (GetWorld()->OverlapMultiByChannel(overlaps, origin, FQuat::Identity, channel, box, params))
		{
			for (auto& result : overlaps)
			{
				AActor* overlappedActor = result.GetActor();
				if (!overlappedActor) continue;

				UPrimitiveComponent* primComp = Cast<UPrimitiveComponent>(result.Component.Get());
				if (!primComp) continue;

				// 이미 처리한 액터는 스킵
				if (ejectedActors.Contains(overlappedActor))
					continue;

				// ── 캐릭터 처리: 히트박스 컴포넌트인지 확인 ──
				if (ACharacter* character = Cast<ACharacter>(overlappedActor))
				{
					const bool bIsHitBoxProfile = (primComp->GetCollisionProfileName() == FName(TEXT("HitBox")));

					// 히트박스가 아니면 캐릭터는 튕기지 않음
					if (!(bIsHitBoxProfile)) continue;

					// 중복 방지 마킹
					ejectedActors.Add(overlappedActor);

					// 기존 질량역수 방식 속도 + 최소 보장 속도
					const float mass = FMath::Max(primComp->GetMass(), 1.0f);
					float rawVelocity = 800.0f / mass;
					float finalVelocityZ = FMath::Max(rawVelocity, minVelocity);

					character->LaunchCharacter(FVector(0.0f, 0.0f, finalVelocityZ), true, true);
					continue;
				}

				// ── 물리 오브젝트(아이템) 튕김 ──
				if (primComp->IsSimulatingPhysics())
				{
					ejectedActors.Add(overlappedActor);

					float mass = FMath::Max(primComp->GetMass(), 1.0f);
					float rawImpulseZ = mass * 300.0f;
					float minImpulseZ = mass * minVelocity;
					float finalImpulseZ = FMath::Max(rawImpulseZ, minImpulseZ);

					primComp->AddImpulse(FVector(0.0f, 0.0f, finalImpulseZ), NAME_None, true);
				}
			}
			overlaps.Reset();
		}
	}
}

void ADropPod::SpawnInternalActor(const FVector& SpawnLocation)
{
	if (!_dropPodToSpawn)
		return;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 헬포드면: 높이만큼 아래에서 스폰 준비
	if (_dropPodToSpawn->IsChildOf(AHellPodBase::StaticClass()))
	{
		float hellHeight = 0.0f;
		if (const AActor* cdo = _dropPodToSpawn->GetDefaultObject<AActor>())
		{
			if (const UPrimitiveComponent* primOnCDO = cdo->FindComponentByClass<UPrimitiveComponent>())
			{
				hellHeight = primOnCDO->Bounds.BoxExtent.Z * 2.0f;
			}
		}
		if (hellHeight <= 0.0f)
			hellHeight = 200.0f;  // 안전값

		// 아래로 내린 시작 위치 계산
		const FVector startBelow = SpawnLocation - FVector::UpVector * hellHeight;

		// 콜리전 끄고 아래에서 스폰
		AActor* spawned = GetWorld()->SpawnActorDeferred<AActor>(
			_dropPodToSpawn,
			FTransform(GetActorRotation(), startBelow),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (!spawned)
			return;

		if (UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(spawned->GetRootComponent()))
		{
			rootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 바닥과 충돌 방지
		}

		UGameplayStatics::FinishSpawningActor(spawned, spawned->GetActorTransform());

		return; 
	}

	// 헬포드가 아니면 원래 위치에 바로 스폰
	GetWorld()->SpawnActor<AActor>(_dropPodToSpawn, SpawnLocation, GetActorRotation(), spawnParams);
}

void ADropPod::HandleGroundLanding(const FVector& hitPoint)
{
	// 포드를 착지 지점으로 이동 (회전은 그대로: 수직 유지)
	_projectile->StopMovementImmediately();
	_projectile->SetComponentTickEnabled(false);
	SetActorLocation(hitPoint, false);

	_mesh->SetGenerateOverlapEvents(false);
	_mesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	LaunchOverlappedActors(hitPoint);
	SpawnInternalActor(hitPoint);
}

void ADropPod::DestroySelf()
{
	SetLifeSpan(1.0f); // 자기 자신을 1초뒤에 지운다
}
