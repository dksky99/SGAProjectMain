// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBulletBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGunBulletBase::AGunBulletBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    _collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    _collisionComp->SetNotifyRigidBodyCollision(true); // OnHit 이벤트에 필요
    _collisionComp->SetCollisionProfileName(TEXT("BlockAll"));
    RootComponent = _collisionComp;

    // 이동 컴포넌트
    _projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    //_projectileMovement->InitialSpeed = 3000.f;
    //_projectileMovement->MaxSpeed = 3000.f;
    //_projectileMovement->bRotationFollowsVelocity = true;
    //_projectileMovement->ProjectileGravityScale = 0.f;

}

// Called when the game starts or when spawned
void AGunBulletBase::BeginPlay()
{
	Super::BeginPlay();

    _collisionComp->OnComponentHit.AddDynamic(this, &AGunBulletBase::OnHit);

    if (GetOwner())
        _collisionComp->IgnoreActorWhenMoving(GetOwner(), true);
    if (GetInstigator())
        _collisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
}

// Called every frame
void AGunBulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGunBulletBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this)
    {
        UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, nullptr);
    }

    UE_LOG(LogTemp, Log, TEXT("Bullet Hit!"));
    Destroy();
}

void AGunBulletBase::InitializeProjectile()
{

}

