// Fill out your copyright notice in the Description page of Project Settings.


#include "Hellpod.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AHellpod::AHellpod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
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
		_mesh->OnComponentHit.AddDynamic(this, &AHellpod::OnHit);
		_mesh->SetNotifyRigidBodyCollision(true);
	}
	
}

// Called every frame
void AHellpod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHellpod::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (isAlreadySpawned)
		return;

	if (ACharacter* hitCharacter = Cast<ACharacter>(OtherActor))
	{
		UGameplayStatics::ApplyDamage(hitCharacter, _damage, GetInstigatorController(), this, UDamageType::StaticClass());
	}

	isAlreadySpawned = true;

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this; 
	spawnParams.Instigator = GetInstigator();
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector spawnLocation = GetActorLocation();
	FRotator spawnRotation = GetActorRotation();

	AActor* spawnedActor = GetWorld()->SpawnActor<AActor>(_hellpodToSpawn, spawnLocation, spawnRotation, spawnParams);
	
	DestroySelf();
}

void AHellpod::DestroySelf()
{
	SetLifeSpan(1.0f); // 자기 자신을 1초뒤에 지운다
}

