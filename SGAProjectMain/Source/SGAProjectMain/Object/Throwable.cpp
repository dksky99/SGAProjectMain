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


	_projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement")); // �� ������Ʈ�� ����ü�� ������ �ڵ����� ������ݴϴ�
	_projectileMovement->bRotationFollowsVelocity = true; // ����ü�� ȸ�� ������ �ӵ�(���� ����)�� ���󰡰� �Ѵ�
	_projectileMovement->ProjectileGravityScale = 1.0f; // �߷��� ������ �󸶳� ������ �����մϴ�
	_projectileMovement->InitialSpeed = 10.f; // ó�� ������ ���� �⺻ �ӵ��Դϴ�
	_projectileMovement->MaxSpeed = 1000.f; // ����ü�� ���� �� �ִ� �ִ� �ӵ��Դϴ�
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
	float power = 100.0f; // _owner->�� �����ͼ� ����

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
		return; // ����: �ڱ� �ڽ� �Ǵ� ������
}

