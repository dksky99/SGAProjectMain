// Fill out your copyright notice in the Description page of Project Settings.


#include "OrbitalPrecisionStrike.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AOrbitalPrecisionStrike::AOrbitalPrecisionStrike()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;

	_projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	_projectile->ProjectileGravityScale = 1.0f;
	_projectile->InitialSpeed = 0.f;
	_projectile->MaxSpeed = 2000.f;
	_projectile->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void AOrbitalPrecisionStrike::BeginPlay()
{
	Super::BeginPlay();

	_mesh->OnComponentHit.AddDynamic(this, &AOrbitalPrecisionStrike::OnHit);
	_mesh->SetNotifyRigidBodyCollision(true);
}

// Called every frame
void AOrbitalPrecisionStrike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOrbitalPrecisionStrike::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),               // 1. 월드 컨텍스트
		_damage,                  // 2. 데미지 수치
		GetActorLocation(),       // 3. 폭발 중심 위치
		_radius,                  // 4. 범위 반경
		nullptr,                  // 5. 데미지 타입 (기본 데미지)
		{},                       // 6. 무시할 액터들
		this,                     // 7. 데미지를 유발한 액터 (보통 자기 자신)
		GetInstigatorController(),// 8. 공격자 컨트롤러 (플레이어 or AI)
		true                      // 9. 반경 안 모두 동일 데미지
	);

	if (_explosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), _explosionEffect, GetActorLocation());
	}

	Destroy();
}

