// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"

#include "Physics/PhysicsInterfacePhysX.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/ConstraintTypes.h"
#include "StatComponent.h"
#include "CharacterStateComponent.h"
#include "CharacterAnimInstance.h"


#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"


#include "MotionWarpingComponent.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage

#include "../Data/UnitAttackDataAsset.h"

#include "../Object/Corpse.h"

#include "Kismet/GameplayStatics.h"

const FName ACharacterBase::StatComponentName = "StatComponent";

// Sets default values
ACharacterBase::ACharacterBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_statComponent = CreateDefaultSubobject<UStatComponent>(StatComponentName);


	_stimuliSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	_stimuliSourceComp->bAutoRegister = true;

	// 어떤 감각에 반응할지 등록
	_stimuliSourceComp->RegisterForSense(UAISense_Sight::StaticClass());
	_stimuliSourceComp->RegisterForSense(UAISense_Hearing::StaticClass());
	_stimuliSourceComp->RegisterForSense(UAISense_Damage::StaticClass());
	_stimuliSourceComp->RegisterWithPerceptionSystem();
	
	GetCharacterMovement()->MaxStepHeight = 45.f;
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

void ACharacterBase::MakeSound(float loudness,FString soundName)
{
	UAISense_Hearing::ReportNoiseEvent(
		this,
		this->GetActorLocation(),
		loudness,
		this,
		0.0f,
		*soundName
	);

	DrawDebugSphere(GetWorld(), this->GetActorLocation(), 10.f*loudness, 10, FColor::Yellow, false, 1.f);

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

	UE_LOG(LogTemp, Display, TEXT("CharacterToRagdoll CB : %s"),*this->GetName());


	UCharacterMovementComponent* movementComponent = GetCharacterMovement();

	if (movementComponent)
	{
		movementComponent->StopMovementImmediately();
		movementComponent->DisableMovement();
	}

	USkeletalMeshComponent* tempMesh = GetMesh();
	if (tempMesh)
	{
		tempMesh->SetSimulatePhysics(true);
		tempMesh->SetAllBodiesSimulatePhysics(true);
		tempMesh->SetCollisionProfileName(TEXT("pawn"));

		UPhysicsAsset* tempPhysicsAsset = tempMesh->GetPhysicsAsset();
		if (tempPhysicsAsset)
		{

			for (UPhysicsConstraintTemplate* tempConstraintTemplate : tempPhysicsAsset->ConstraintSetup)
			{
				if (tempConstraintTemplate)
				{

					FConstraintInstance& tempConstraintInstance = tempConstraintTemplate->DefaultInstance;
					tempConstraintInstance.SetLinearLimits(ELinearConstraintMotion::LCM_Free, ELinearConstraintMotion::LCM_Free, ELinearConstraintMotion::LCM_Free, 0.0f);

				}
			}
		}
	}

	UCapsuleComponent* tempCapsule = GetCapsuleComponent();
	if (tempCapsule)
	{
		tempCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (tempMesh && tempCapsule)
	{
		tempMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		RootComponent = tempMesh;

		tempCapsule->AttachToComponent(tempMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		tempCapsule->SetRelativeLocation(FVector(0, 0, 88));
		tempCapsule->SetUsingAbsoluteRotation(true);
	}

}

void ACharacterBase::KnockDown(float time)
{
	CharacterToRagdoll();

	// 3초 후 회복 함수 호출
	GetWorldTimerManager().SetTimer(_knockDownTimerHandle, this, &ACharacterBase::RecoverFromKnockDown, time, false);


}

void ACharacterBase::RecoverFromKnockDown()
{

	USkeletalMeshComponent* tempMesh = GetMesh();
	UCapsuleComponent* tempCapsule = GetCapsuleComponent();

	if (!tempMesh || !tempCapsule)
		return;

	// 1. 현재 위치 보정 (예: pelvis 본 기준)
	const FVector pelvisLocation = tempMesh->GetBoneLocation(FName("pelvis"),EBoneSpaces::WorldSpace);
	UE_LOG(LogTemp,Display,TEXT("Bone Loc : %f %f %f"),pelvisLocation.X, pelvisLocation.Y, pelvisLocation.Z)
	
	// 2. 메시 물리 비활성화
	tempMesh->SetSimulatePhysics(false);
	tempMesh->SetAllBodiesSimulatePhysics(false);
	tempMesh->SetCollisionProfileName(TEXT("CharacterMesh")); // 기본 충돌 프로파일로 복원

	// 3. 메시 재부착 (캡슐 기준)

	tempCapsule->DetachFromParent();
	RootComponent = tempCapsule;
	tempMesh->AttachToComponent(tempCapsule, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	tempMesh->SetRelativeLocation(FVector(0.f, 0.f, -tempCapsule->GetUnscaledCapsuleHalfHeight()));
	tempMesh->SetWorldRotation(FRotator(0, -90, 0));

	// 4. 캡슐 콜리전 복구
	tempCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	SetActorLocation(pelvisLocation + FVector(0, 0, tempCapsule->GetUnscaledCapsuleHalfHeight()));
	// 5. 이동 컴포넌트 재활성화
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	// 6. 애니메이션 갱신 재개
	tempMesh->bPauseAnims = false;
	tempMesh->bNoSkeletonUpdate = false;




}

void ACharacterBase::Dead()
{
	UE_LOG(LogTemp, Display, TEXT("Dead CB : %s"), *this->GetName());
	//래그돌로 변하고 
	//사망시 컨트롤러를 내려놓고 

	_isReadyToSpawn = false;
	CharacterToRagdoll();
	UE_LOG(LogTemp, Display, TEXT("CharacterToRagdoll CB : %s"), *this->GetName());
	




}

void ACharacterBase::RecoverFromDead()
{
	//우선 래그돌상태에서 회복하고
	RecoverFromKnockDown();

	
	//소환 가능 상태가 됨.
	UnitDeactivate();

	_isReadyToSpawn = true;

}

void ACharacterBase::SpawnGhost()
{
	// 사망 직후 CurrentMesh는 이미 래그돌 상태
	USkeletalMeshComponent* originalMesh = GetMesh();
	
	// GhostActor 스폰
	FActorSpawnParameters params;
	ACorpse* ghost = GetWorld()->SpawnActor<ACorpse>(
		ACorpse::StaticClass(),
		GetActorTransform(),
		params);

	// 메시 설정 (원본과 동일한 Skeleton/Mesh)
	ghost->InitCorpseMesh(originalMesh);

	ghost->SetActorScale3D(GetActorScale3D());






}

void ACharacterBase::ResetUnit()
{
	_statComponent->Reset();
	_stateComp->Reset();
	//RecoverFromKnockDown();

	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	SetActorHiddenInGame(false);
}

void ACharacterBase::UnitDeactivate()
{

	
	if (GetController())
	{
		GetController()->UnPossess();
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);


	SetActorLocation(FVector::ZeroVector);

}
void ACharacterBase::UnitActivate()
{

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
}



bool ACharacterBase::IsReadyToSpawn()
{
	if (GetController())
		return false;

	return _isReadyToSpawn;
}

void ACharacterBase::ReadyToSpawn()
{
	_isReadyToSpawn = true;
	if (GetWorld()->GetTimerManager().IsTimerActive(_respawnTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(_respawnTimer);
	}
}

bool ACharacterBase::GetTargetLook(FVector& loc,FVector& dir) 
{
	loc = GetActorLocation();
	dir = GetActorForwardVector();
	return true;
}

void ACharacterBase::Spawn()
{
	_isReadyToSpawn = false;
	ResetUnit();
	//스폰몽타주가 있을시. 몽타주가 끝나고 컨트롤러 결합을할지 고민중
	if (_spawnMontage)
	{
		if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			_stateComp->ActionBegin();
			animInstance->PlayAnimMontage(_spawnMontage);
			_reservedFunction.BindUObject(this, &ACharacterBase::SpawnProcessFinish);
			// 재생 후 인스턴스 가져오기
			if (FAnimMontageInstance* montageInstance = animInstance->GetActiveInstanceForMontage(_spawnMontage))
			{

			}
		}
		else
		{
			SpawnProcessFinish();
		}
	}
	else
			SpawnProcessFinish();
}

void ACharacterBase::SpawnProcessFinish()
{

}



UStatComponent* ACharacterBase::GetStatComponent()
{
	return _statComponent;
}

UCharacterStateComponent* ACharacterBase::GetStateComponent()
{
	return _stateComp;
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
	Dead();

	// 사망 시 로그 출력
	UE_LOG(LogTemp, Error, TEXT("%s: Character Dead"), *GetName());
}

bool ACharacterBase::IsTargetable() const
{
	return _statComponent->IsDead()==false;
}

FTransform ACharacterBase::GetTargetTransform() const
{
	return GetActorTransform();
}

void ACharacterBase::InitMeleeColliders()
{
	for (auto col : _meleeColliders)
	{
		col.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		col.Value->SetCollisionObjectType(ECC_WorldDynamic);
		col.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
		col.Value->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Pawn만 감지
		col.Value->OnComponentBeginOverlap.AddDynamic(this, &ACharacterBase::OnWeaponOverlap);
	}
	_activateColliders.Reserve(_meleeColliders.Num());
}

void ACharacterBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && OtherActor != this)
	{
		if (CheckHitted(OtherActor))
			return;

		float finalDamage = _curAttackData->Attack;

		FVector shotDirection = SweepResult.ImpactNormal; // 데미지 방향

		UGameplayStatics::ApplyPointDamage(
			OtherActor,                     // 데미지를 받을 액터
			finalDamage,                    // 적용할 기본 데미지 값
			shotDirection,                  // 데미지가 들어온 방향 벡터
			SweepResult,                    // 충돌 정보(FHitResult)
			GetInstigatorController(),      // 데미지를 유발한 컨트롤러
			this,                           // 데미지 발생 주체 액터
			UDamageType::StaticClass()      // 사용할 데미지 타입 클래스
		);


		AddHitted(OtherActor);

	}

}

bool ACharacterBase::AttackMelee()
{
	UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


	if (_meleeAttackDatas.IsEmpty())
		return false;
	if (anim == nullptr)
		return false;
	if (_stateComp->ActionBegin() == false)
		return false;
	int32 randomIndex = FMath::RandRange(0, _meleeAttackDatas.Num() - 1);

	

	const float Duration = anim->PlayAnimMontage(_meleeAttackDatas[randomIndex]->Motion);

	_curAttackData = _meleeAttackDatas[randomIndex];
	SetMeleeColisions(_meleeAttackDatas[randomIndex]);
	return true;
}

void ACharacterBase::ActionEnd()
{
	ReleaseMeleeColision();

	GetStateComponent()->ActionEnd();

	if (_reservedFunction.IsBound())
		_reservedFunction.Unbind();

	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (AnimInstance->GetCurrentActiveMontage() != nullptr)
		{
			AnimInstance->Montage_Stop(0.25f);

		}
	}

}


void ACharacterBase::SetMeleeColisions(class UUnitAttackDataAsset* data)
{
	for (FName colName : data->ActiveColliders)
	{
		if (_meleeColliders.Contains(colName))
		{
			_activateColliders.Add(_meleeColliders[colName]);
		}
	}
}

void ACharacterBase::ReleaseMeleeColision()
{
	DeactivateMeleeColision();
	_activateColliders.Empty();
}

void ACharacterBase::ActivateMeleeColision()
{
	for (auto col : _activateColliders)
	{

		col->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ACharacterBase::DeactivateMeleeColision()
{
	for (auto col : _activateColliders)
	{

		col->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	ClearHitted();
	
}

bool ACharacterBase::CheckHitted(AActor* target)
{
	if (_hitted.Contains(target))
		return true;
	return false;
}

void ACharacterBase::AddHitted(AActor* target)
{
	_hitted.AddUnique(target);
}

void ACharacterBase::ClearHitted()
{
	_hitted.Empty();
}

void ACharacterBase::TakeHitted(FVector hitPoint, float hitPower)
{
	//피격시 충격이 저항력보다 크면 경직, 헬다이버는 충격량이2배이상이면 넉다운 적은 피격방향에따라 다른 애니메이션 혹은 단순히 몽타주 캔슬만.
	if (_statComponent->GetImpactResistance() < hitPower)
		return;

	PlayHitReaction();




}

void ACharacterBase::PlayHitReaction(float time)
{
	//행동을 강제 종료,
	ActionEnd();
	_stateComp->ActionBegin();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (_hitReactionMontage) // 피격 몽타주 애셋. 에셋과 별개로 히트 리커버리는 일정.
		{
			AnimInstance->Montage_Play(_hitReactionMontage);
		}

	}
	
	GetWorld()->GetTimerManager().SetTimer(_knockDownTimerHandle, this, &ACharacterBase::HitRecovery,time, false);
	
}

void ACharacterBase::HitRecovery()
{
	ActionEnd();
	//
}
