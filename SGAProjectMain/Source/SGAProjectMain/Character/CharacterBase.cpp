// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "StatComponent.h"

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACharacterBase::UpDown(float value)
{
	if (abs(value) < 0.01f)
	{
		return;
	}
	_vertical = value;
	FRotator yawOnly = FRotator(0.f, GetControlRotation().Yaw, 0.f);
	FVector forward = FRotationMatrix(yawOnly).GetUnitAxis(EAxis::X); // 전방 방향

	AddMovementInput(forward * value * 10.0f);
}

void ACharacterBase::RightLeft(float value)
{
	if (abs(value) < 0.01f)
	{
		return;
	}
	_horizontal = value;
	FRotator yawOnly = FRotator(0.f, GetControlRotation().Yaw, 0.f);
	FVector right = FRotationMatrix(yawOnly).GetUnitAxis(EAxis::Y); // 우측 방향

	AddMovementInput(right * value * 10.0f);
}


// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float zVelocity = GetCharacterMovement()->Velocity.Z;

	UE_LOG(LogTemp, Log, TEXT("Landing Z Velocity: %f"), zVelocity);

	if (zVelocity < -1200.f)
	{
		// 하드랜딩
	}
	else if (zVelocity < -400.f)
	{
		// 일반 착지
	}
}

void ACharacterBase::KnockDown(float time)
{
	// 이동 멈추기
	GetCharacterMovement()->DisableMovement();

	// 캡슐과 메시 분리 충돌 처리
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	// 물리 시뮬레이션 시작
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	// 일정 시간 후 복구
	GetWorld()->GetTimerManager().SetTimer(
		_knockDownTimerHandle, this, &ACharacterBase::KnockDownRecovery, time, false
	);
}

void ACharacterBase::KnockDownRecovery()
{
	// 물리 시뮬레이션 중지
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->bBlendPhysics = false;

	// 메시를 캡슐로 재정렬
	FVector MeshLocation = GetMesh()->GetComponentLocation();
	FRotator MeshRotation = GetMesh()->GetComponentRotation();
	SetActorLocation(MeshLocation);
	SetActorRotation(FRotator(0, MeshRotation.Yaw, 0)); // 수평 방향만 유지

	// 메시 위치 보정
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
	GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);

	// 충돌 및 이동 복원 : 나중에 확인하고 다시 확인.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

float ACharacterBase::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
{
	return -damageAmount;
}

void ACharacterBase::Dead()
{
	SetLifeSpan(3.0f);
}

