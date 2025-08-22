// Fill out your copyright notice in the Description page of Project Settings.


#include "BombProjectile.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../ExplosionComponent.h"

// Sets default values
ABombProjectile::ABombProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 캡슐 콜리전 루트
	_collisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	_collisionComponent->InitCapsuleSize(15.0f, 40.0f);						// 반경, 하프 높이
	_collisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));  // 프리셋
	_collisionComponent->SetNotifyRigidBodyCollision(true);					// 물리적 블록 충돌 시 Hit 이벤트(OnComponentHit)가 발생하도록 설정합니다.
	_collisionComponent->SetGenerateOverlapEvents(false);					// Overlap 이벤트는 생성하지 않도록 합니다(충돌=Block만 사용).
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

	// 메시만 로컬 롤 회전(액터는 속도방향을 유지)
	if (_spinDegreesPerSecond != 0.0f && _mesh)
	{
		const float rollDelta = _spinDegreesPerSecond * DeltaSeconds;
		_mesh->AddLocalRotation(FRotator(0.0f, 0.0f, rollDelta));
	}
}

void ABombProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (_explosionComponent)
		_explosionComponent->Explode(); // 오버랩→감쇠 데미지→이펙트 재생을 내부에서 처리
	
	Destroy();
}

