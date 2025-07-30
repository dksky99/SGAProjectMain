// Fill out your copyright notice in the Description page of Project Settings.


#include "Hellpod.h"

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

// Sets default values
AHellpod::AHellpod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	
	// 충돌 설정: 캐릭터, 아이템 등은 무시, 바닥은 Block
	_mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_mesh->SetCollisionObjectType(ECC_WorldDynamic);

	_mesh->SetCollisionResponseToAllChannels(ECR_Ignore);            // 일단 전부 무시
	_mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 바닥만 막기

	RootComponent = _mesh;

	_projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	_projectile->ProjectileGravityScale = 0.0f;
	_projectile->InitialSpeed = 0.f;
	_projectile->MaxSpeed = 10000.f;
	_projectile->bRotationFollowsVelocity = false;
}

// Called when the game starts or when spawned
void AHellpod::BeginPlay()
{
	Super::BeginPlay();

	if (_mesh)
	{
		_mesh->OnComponentBeginOverlap.AddDynamic(this, &AHellpod::OnBeginOverlap);
		_mesh->OnComponentHit.AddDynamic(this, &AHellpod::OnHit);

		_mesh->SetNotifyRigidBodyCollision(true);

		// 충돌 설정: 캐릭터는 관통 (Overlap), 바닥은 막기 (Block)
		_mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		_mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	}
	
}

// Called every frame
void AHellpod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_isRising)
	{
		FVector currentLocation = GetActorLocation();
		FVector newLocation = currentLocation + FVector(0, 0, _riseSpeed * DeltaTime);
		SetActorLocation(newLocation);

		// 목표 위치에 도달하면 상승 종료
		if (newLocation.Z >= _targetLocation.Z)
		{
			SetActorLocation(_targetLocation);
			_isRising = false;
			OnRiseFinished();
		}
	}
}

void AHellpod::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ACharacter* hitCharacter = Cast<ACharacter>(OtherActor))
	{
		if (_damagedCharacters.Contains(hitCharacter))
			return;

		_damagedCharacters.Add(hitCharacter);

		// 폭발(헬포드 충돌) 중심에서 명중 지점까지 방향 계산
		const FVector shotDirection = (SweepResult.ImpactPoint - GetActorLocation()).GetSafeNormal();

		// 스윕 결과에 담긴 FHitResult를 그대로 사용하여 포인트 데미지 호출
		UGameplayStatics::ApplyPointDamage(
			hitCharacter,                   // 데미지를 받을 액터
			_damage,                        // 적용할 기본 데미지 값
			shotDirection,                  // 데미지가 들어온 방향 벡터
			SweepResult,                    // 충돌 정보(FHitResult, Impact 컴포넌트 포함)
			GetInstigatorController(),      // 데미지를 유발한 컨트롤러
			this,                           // 데미지 발생 주체 액터
			UDamageType::StaticClass()      // 사용할 데미지 타입 클래스
		);
	}
}

void AHellpod::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (isAlreadySpawned)
		return;

	if (!_isGrounded && OtherComp && OtherComp->ComponentHasTag(FName("Ground")))
	{
		_isGrounded = true;

		float hellpodHeight = _mesh->Bounds.BoxExtent.Z * 2.0f;

		FVector sunkenLocation = GetActorLocation() - FVector(0, 0, hellpodHeight);
		SetActorLocation(sunkenLocation);

		_targetLocation = sunkenLocation + FVector(0, 0, hellpodHeight);

		GetWorldTimerManager().SetTimer(_riseTimerHandle, this, &AHellpod::StartRising, 0.5f, false);

		LaunchOverlappedActors(Hit.ImpactPoint);

	}
}

void AHellpod::StartRising()
{
	_isRising = true;
}

void AHellpod::OnRiseFinished()
{
	SpawnInternalActor();
	DestroySelf();
}

void AHellpod::LaunchOverlappedActors(const FVector& hitPoint)
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
				if (!overlappedActor)
					continue;

				UPrimitiveComponent* primComp = Cast<UPrimitiveComponent>(result.Component.Get());
				if (!primComp)
					continue;

				float mass = primComp->GetMass();
				if (mass <= 1.f) mass = 1.f;

				// ── 캐릭터 튕김 ──
				if (ACharacter* character = Cast<ACharacter>(overlappedActor))
				{
					// 기존 질량역수 방식 속도
					float rawVelocity = 800.f / mass;
					// 최소 보장 속도
					float finalVelocityZ = FMath::Max(rawVelocity, minVelocity);
					character->LaunchCharacter(FVector(0, 0, finalVelocityZ), true, true);
				}
				// ── 물리 오브젝트(아이템) 튕김 ──
				else if (primComp->IsSimulatingPhysics())
				{
					if (!ejectedActors.Contains(overlappedActor))
					{
						ejectedActors.Add(overlappedActor);

						// 기존 질량 기반 임펄스
						float rawImpulseZ = mass * 300.f;
						// 최소 보장 임펄스 = m * minVelocity
						float minImpulseZ = mass * minVelocity;
						// 더 큰 값을 사용하여 최소 헬포드 높이 이상으로 튕김
						float finalImpulseZ = FMath::Max(rawImpulseZ, minImpulseZ);

						primComp->AddImpulse(FVector(0, 0, finalImpulseZ), NAME_None, true);
					}
				}
			}
			overlaps.Reset();
		}
	}
}

void AHellpod::SpawnInternalActor()
{
	if (!_hellpodToSpawn)
		return;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.Instigator = GetInstigator();
	// 스폰 시 충돌이 있어도 가능하면 위치 조정 후 무조건 스폰하도록 설정
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	GetWorld()->SpawnActor<AActor>(_hellpodToSpawn, GetActorLocation(), GetActorRotation(), spawnParams);
}

void AHellpod::DestroySelf()
{
	SetLifeSpan(1.0f); // 자기 자신을 1초뒤에 지운다
}

