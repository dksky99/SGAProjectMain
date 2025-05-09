// Fill out your copyright notice in the Description page of Project Settings.

#include "Throwable.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "../Character/HellDiver/HellDiver.h"

// Sets default values
AThrowable::AThrowable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = _mesh;


	_projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));	// 이 컴포넌트는 투사체의 물리 기반 이동과 중력, 반사 등을 자동으로 처리합니다
	_projectileMovement->bRotationFollowsVelocity = true;	// 투사체가 이동 방향(속도 벡터)을 따라 회전하도록 설정합니다
	_projectileMovement->ProjectileGravityScale = 1.0f;	// 중력의 영향을 받는 정도를 설정합니다 (1.0 = 기본 중력)
	_projectileMovement->InitialSpeed = 800.0f;	// 투사체가 생성될 때 초기 속도입니다
	_projectileMovement->MaxSpeed = 800.0f;	// 투사체가 가질 수 있는 최대 속도입니다 (속도 제한)
	_projectileMovement->bShouldBounce = true;	// 충돌 시 반사(튕김)를 활성화합니다
	_projectileMovement->Bounciness = 0.3f;	// 튕김의 반사 정도입니다 (1.0 = 완전 반사, 0.0 = 반사 없음)
	_projectileMovement->Friction = 0.3f;	// 마찰 계수입니다 (0 = 미끄러움, 1 = 매우 잘 멈춤)
}

// Called when the game starts or when spawned
void AThrowable::BeginPlay()
{
	Super::BeginPlay();

	if (_mesh)
	{
		_mesh->OnComponentHit.AddDynamic(this, &AThrowable::OnHit);
		_mesh->SetNotifyRigidBodyCollision(true);
	}

	_owner = Cast<AHellDiver>(GetOwner());
	
}

// Called every frame
void AThrowable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThrowable::Throw()
{
	if (!_owner || !_projectileMovement)
		return;

	FVector direction = _owner->GetActorForwardVector();
	float power = 100.0f; // _owner->힘 가져와서 설정

	_projectileMovement->Velocity = direction * power;

	GetWorldTimerManager().SetTimer(_destroyTimerHandle, this, &AThrowable::OnLifeTimeExpired, _lifeTime, false);

}

void AThrowable::OnLifeTimeExpired()
{
	DestroySelf();
}

void AThrowable::DestroySelf()
{
	GetWorldTimerManager().ClearTimer(_destroyTimerHandle);
	Destroy();
}

void AThrowable::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == _owner)
		return; // 무시: 자기 자신 또는 소유자
}

