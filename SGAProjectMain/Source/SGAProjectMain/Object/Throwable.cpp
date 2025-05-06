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


	_projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement")); // 이 컴포넌트가 투사체의 궤적을 자동으로 계산해줍니다
	_projectileMovement->bRotationFollowsVelocity = true; // 투사체의 회전 방향이 속도(방향 벡터)를 따라가게 한다
	_projectileMovement->ProjectileGravityScale = 1.0f; // 중력의 영향을 얼마나 받을지 설정합니다
	_projectileMovement->InitialSpeed = 10.f; // 처음 생성될 때의 기본 속도입니다
	_projectileMovement->MaxSpeed = 1000.f; // 투사체가 가질 수 있는 최대 속도입니다
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

