// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "Physics/PhysicsInterfacePhysX.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/ConstraintTypes.h"
#include "StatComponent.h"

const FName ACharacterBase::StatComponentName = "StatComponent";

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_statComponent = CreateDefaultSubobject<UStatComponent>(StatComponentName);

}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (_statComponent->IsValidLowLevel())
	{
		// 부위 파괴
		_statComponent->OnPartDestroyed.AddDynamic(this, &ACharacterBase::OnPartDestroyed_Handler);
		// 사망
		_statComponent->OnDeath.AddDynamic(this, &ACharacterBase::OnDeath_Handler);
	}
	
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

	GetCapsuleComponent()->SetUsingAbsoluteRotation(false);
	// 충돌 및 이동 복원 : 나중에 확인하고 다시 확인.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

//float ACharacterBase::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
//{
//	return -damageAmount;
//}

void ACharacterBase::CharacterToRagdoll()
{



	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}

	USkeletalMeshComponent* TempMesh = GetMesh();
	if (TempMesh)
	{
		TempMesh->SetSimulatePhysics(true);
		TempMesh->SetAllBodiesSimulatePhysics(true);
		TempMesh->SetCollisionProfileName(TEXT("Ragdoll"));

		UPhysicsAsset* TempPhysicsAsset = TempMesh->GetPhysicsAsset();
		if (TempPhysicsAsset)
		{

			for (UPhysicsConstraintTemplate* TempConstraintTemplate : TempPhysicsAsset->ConstraintSetup)
			{
				if (TempConstraintTemplate)
				{

					FConstraintInstance& TempConstraintInstance = TempConstraintTemplate->DefaultInstance;
					TempConstraintInstance.SetLinearLimits(ELinearConstraintMotion::LCM_Free, ELinearConstraintMotion::LCM_Free, ELinearConstraintMotion::LCM_Free, 0.0f);

				}
			}
		}
	}

	UCapsuleComponent* TempCapsule = GetCapsuleComponent();
	if (TempCapsule)
	{
		TempCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TempMesh && TempCapsule)
	{
		TempMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		RootComponent = TempMesh;

		TempCapsule->AttachToComponent(TempMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		TempCapsule->SetRelativeLocation(FVector(0, 0, 88));
		TempCapsule->SetUsingAbsoluteRotation(true);
	}

}

void ACharacterBase::KnockDown()
{
	CharacterToRagdoll();

	// 3초 후 회복 함수 호출
	GetWorldTimerManager().SetTimer(_knockDownTimerHandle, this, &ACharacterBase::RecoverFromKnockDown, 3.0f, false);


}

void ACharacterBase::RecoverFromKnockDown()
{

	USkeletalMeshComponent* TempMesh = GetMesh();
	UCapsuleComponent* TempCapsule = GetCapsuleComponent();

	if (!TempMesh || !TempCapsule)
		return;

	// 1. 현재 위치 보정 (예: pelvis 본 기준)
	const FVector PelvisLocation = TempMesh->GetBoneLocation(FName("pelvis"),EBoneSpaces::WorldSpace);
	UE_LOG(LogTemp,Display,TEXT("Bone Loc : %f %f %f"),PelvisLocation.X, PelvisLocation.Y, PelvisLocation.Z)
	
	// 2. 메시 물리 비활성화
	TempMesh->SetSimulatePhysics(false);
	TempMesh->SetAllBodiesSimulatePhysics(false);
	TempMesh->SetCollisionProfileName(TEXT("CharacterMesh")); // 기본 충돌 프로파일로 복원

	// 3. 메시 재부착 (캡슐 기준)

	TempCapsule->DetachFromParent();
	RootComponent = TempCapsule;
	TempMesh->AttachToComponent(TempCapsule, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	TempMesh->SetRelativeLocation(FVector(0.f, 0.f, -TempCapsule->GetUnscaledCapsuleHalfHeight()));
	TempMesh->SetWorldRotation(FRotator(0, -90, 0));

	// 4. 캡슐 콜리전 복구
	TempCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SetActorLocation(PelvisLocation + FVector(0, 0, TempCapsule->GetUnscaledCapsuleHalfHeight()));
	// 5. 이동 컴포넌트 재활성화
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	// 6. 애니메이션 갱신 재개
	TempMesh->bPauseAnims = false;
	TempMesh->bNoSkeletonUpdate = false;




}

void ACharacterBase::Dead()
{

	CharacterToRagdoll();

	AController* CurrentController = GetController();
	if (CurrentController)
	{
		CurrentController->UnPossess();
	}

	SetLifeSpan(10.0f);


}

void ACharacterBase::OnPartDestroyed_Handler(EBodyPart part)
{
	 // 각 부위 파괴 동작 구현 
	if (part == EBodyPart::Head)
	{
		
	}
	else if (part == EBodyPart::Torso)
	{
	}
	else if (part == EBodyPart::LeftArm)
	{
	}
	else if (part == EBodyPart::RightArm)
	{
	}
	else if (part == EBodyPart::LeftLeg)
	{
	}
	else if (part == EBodyPart::RightLeg)
	{
	}

	// 부위 파괴 시 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("%s: Part Destroyed -> %d"), *GetName(), static_cast<int32>(part));
}

void ACharacterBase::RestoreAllParts()
{
}

void ACharacterBase::OnDeath_Handler()
{
	// 사망시 동작 구현

	// 사망 시 로그 출력
	UE_LOG(LogTemp, Error, TEXT("%s: Character Dead"), *GetName());
}

