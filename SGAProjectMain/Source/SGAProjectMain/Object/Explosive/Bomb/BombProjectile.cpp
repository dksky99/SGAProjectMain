// Fill out your copyright notice in the Description page of Project Settings.


#include "BombProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../ExplosionComponent.h"

// Sets default values
ABombProjectile::ABombProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 콜리전 루트
	_collisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	_collisionComponent->InitSphereRadius(15.0f);                      // 반경
	_collisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	_collisionComponent->SetNotifyRigidBodyCollision(true);            // Block 충돌 시 OnComponentHit 발생
	_collisionComponent->SetGenerateOverlapEvents(false);              // Overlap 비활성(필요 시 채널별 혼합 권장)
	RootComponent = _collisionComponent;                                    

	// 메시(렌더 전용, 콜리전 비활성)
	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));     
	_mesh->SetupAttachment(RootComponent);                                  
	_mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);             
	_mesh->SetGenerateOverlapEvents(false);                                 // 메시에서 Overlap 이벤트도 발생시키지 않습니다.

	// 투사체 이동: 낙하 방향을 바라보도록 설정
	_projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement")); 
	_projectileMovement->bRotationFollowsVelocity = true;                   // 속도 벡터 방향으로 액터가 자동 회전하도록 설정합니다(낙하 방향을 주시).
	_projectileMovement->bShouldBounce = false;                             // 지면/오브젝트 충돌 후 튀지 않도록(bounce 비활성) 설정합니다.
	_projectileMovement->SetUpdatedComponent(_collisionComponent);			// 갱신 대상 지정
	_projectileMovement->ProjectileGravityScale = 0.0f;						// 중력 off

	// 폭발 컴포넌트(데미지/반경/이펙트 재생 담당)
	_explosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));  

	// 히트 이벤트 바인딩
	_collisionComponent->OnComponentHit.AddDynamic(this, &ABombProjectile::OnHit);    
}

// Called when the game starts or when spawned
void ABombProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ABombProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABombProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (_explosionComponent)
		_explosionComponent->Explode(); // 오버랩→감쇠 데미지→이펙트 재생을 내부에서 처리
	
	Destroy();
}

