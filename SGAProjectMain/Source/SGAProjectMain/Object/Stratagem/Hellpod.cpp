// Fill out your copyright notice in the Description page of Project Settings.


#include "Hellpod.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/DamageEvents.h"


#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"

#include "../../SGAProjectMain.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
// Sets default values
AHellpod::AHellpod()
{
	

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	
	// 충돌 설정: 캐릭터, 아이템 등은 무시, 바닥은 Block
	_mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_mesh->SetCollisionObjectType(ECC_WorldDynamic);

	_mesh->SetCollisionResponseToAllChannels(ECR_Ignore);             // 일단 전부 무시
	_mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap); // 바닥만 막기

	RootComponent = _mesh;

	_projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile"));
	_projectile->ProjectileGravityScale = 0.0f;
	_projectile->InitialSpeed = 0.f;
	_projectile->MaxSpeed = 10000.f;
	_projectile->bRotationFollowsVelocity = false;

	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	
}

// Called when the game starts or when spawned
void AHellpod::BeginPlay()
{
	Super::BeginPlay();
	if (_mesh)
	{
		_mesh->OnComponentBeginOverlap.AddDynamic(this, &AHellpod::OnBeginOverlap);
		//_mesh->OnComponentHit.AddDynamic(this, &ADropPod::OnHit);

		_mesh->SetNotifyRigidBodyCollision(true);

		_mesh->SetCollisionResponseToChannel(ECC_GameDamage, ECR_Overlap);
	}
}

// Called every frame
void AHellpod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHellpod::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//헬포드는 맵기준으로 북쪽이 위키 동쪽이 오른쪽키 이런식으로 이동.

void AHellpod::Move(const struct FInputActionValue& value)
{

	FVector2D moveVector = value.Get<FVector2D>();

	if (Controller != nullptr && moveVector.Length() > 0.01f)
	{

		if (moveVector.SquaredLength() > 1.0f)
		{
			moveVector.Normalize();
		}


		const FVector forward = FVector::ForwardVector;
		const FVector right = FVector::RightVector;


		AddMovementInput(forward, moveVector.Y);
		AddMovementInput(right, moveVector.X);
	}
}
void AHellpod::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}
void AHellpod::AttachHelldiverToHellpod(APlayerCharacter* helldiver)
{
}

void AHellpod::DettachHelldiverToHellpod()
{
}

