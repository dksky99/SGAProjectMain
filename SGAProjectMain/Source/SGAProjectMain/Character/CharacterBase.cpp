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
//#include "../Object/CDamageType.h"

#include "../CGameInstance.h"

#include "Kismet/GameplayStatics.h"

const FName ACharacterBase::StatComponentName = "StatComponent";

const TMap<FName, EBodyPart> ACharacterBase::PartTagMap =
{
	{TEXT("Core"), EBodyPart::Core},
	{TEXT("Head"), EBodyPart::Head},
	{TEXT("Torso"), EBodyPart::Torso},
	{TEXT("Tail"), EBodyPart::Tail},
	{TEXT("LeftClaw"), EBodyPart::LeftClaw},
	{TEXT("RightClaw"), EBodyPart::RightClaw},
	{TEXT("LeftArm"), EBodyPart::LeftArm},
	{TEXT("RightArm"), EBodyPart::RightArm},
	{TEXT("LeftLeg"), EBodyPart::LeftLeg},
	{TEXT("RightLeg"), EBodyPart::RightLeg}
};


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

	//여기서 레이어데이터를 게임인스턴스에서 가져오자.
	InitUnit();

}

const TMap<FName, EBodyPart>& ACharacterBase::GetPartTagMap()
{
	return PartTagMap;
}

// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (_statComponent->IsValidLowLevel())
	{
		//TODO : 
		// 부위 파괴이벤트 바인드
		//_statComponent->OnPartDestroyed.AddDynamic(this, &ACharacterBase::OnPartDestroyed_Handler);
		// 사망
		_statComponent->OnDeath.AddDynamic(this, &ACharacterBase::OnDeath_Handler);

		PartInit();
	}
	
}

void ACharacterBase::InitUnit()
{
	UGameInstance* gc = GetGameInstance();
	if (gc == nullptr)
		return;
	UCGameInstance* ugc=Cast<UCGameInstance>(gc);
	if (ugc == nullptr)
		return;
	const FProcessedUnitData* data= ugc->GetProcessedUnitData(GetClass());
	if (data == nullptr)
	{

		UE_LOG(LogTemp, Error, TEXT("UnitDataInit Fail : Non data"));
		return;
	}


	_statComponent->InitData(data);
	_stateComp->InitData(data->_resistData);
	UE_LOG(LogTemp, Display, TEXT("UnitDataInit Success"));
}

FCDamageEvent ACharacterBase::AttackDataToDamageEvent(UUnitAttackDataAsset* attackData)
{
	FCDamageEvent event;

	event.BaseDamage = attackData->Attack;
	event.DemolitionDamage = attackData->DemolitionAttack;
	event.DurabilityDamage = attackData->DurabilityAttack;
	event.PenetrationLevel = attackData->PenetrationLevel;
	event.Stagger = attackData->Stagger;
	event.PushForce = attackData->PushForce;
	event.DamageTypeClass = attackData->DamageType;
	event.IsExplosionDamage = false;


	return event;
}

void ACharacterBase::PartInit()
{
	if (_statComponent == nullptr)
		return;
	auto partDatas = _statComponent->GetPartDatas();
	
	if(partDatas->IsEmpty())
		return;
	auto part = partDatas->Find(EBodyPart::Core);
	if (part)
	{
		//기본적인 사망효과.
		if (part->PartStats.IsEmpty()==false)
			part->PartStats[0]._onPartDestroyed.AddDynamic(this, &ACharacterBase::OnDeath_Handler);
	}

}

void ACharacterBase::Critical()
{
	auto main = _statComponent->GetCoreStat();

	_statComponent->ChangeHp(main, (float)(main->_partHP));
	
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
	UE_LOG(LogTemp, Display, TEXT("CharacterToRagdoll CB : %s"), *this->GetName());

	UCharacterMovementComponent* movementComponent = GetCharacterMovement();
	if (movementComponent)
	{
		movementComponent->StopMovementImmediately();
		movementComponent->DisableMovement();
	}

	USkeletalMeshComponent* tempMesh = GetMesh();
	UCapsuleComponent* tempCapsule = GetCapsuleComponent();

	if (tempMesh && tempCapsule)
	{
		// 1. 메시 분리 (원래 월드 위치 유지)
		tempMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// 2. 캡슐 콜리전 비활성화
		tempCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// 3. 메시를 새로운 루트 컴포넌트로 설정
		RootComponent = tempMesh;

		// 4. 물리 시뮬레이션 활성화 (여기가 Full Simulation 시작 지점)
		tempMesh->SetSimulatePhysics(true);
		tempMesh->SetAllBodiesSimulatePhysics(true);
		// *참고: 콜리전 프로파일은 "Ragdoll" 등으로 명확히 설정하는 것을 권장합니다.*
		tempMesh->SetCollisionProfileName(TEXT("pawn"));

		// 5. 캡슐을 메시 컴포넌트에 다시 어태치
		tempCapsule->AttachToComponent(tempMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		tempCapsule->SetRelativeLocation(FVector(0, 0, 88));
		tempCapsule->SetUsingAbsoluteRotation(true);
	}

	// 6. 물리 제약 조건(Constraints) 설정 (기존 로직 유지)
	if (tempMesh)
	{
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

	// 1. 현재 랙돌 메시의 펠비스 위치를 기준으로 목표 위치 계산
	const FVector pelvisLocation = tempMesh->GetBoneLocation(FName("pelvis"), EBoneSpaces::WorldSpace);
	UE_LOG(LogTemp, Display, TEXT("Bone Loc : %f %f %f"), pelvisLocation.X, pelvisLocation.Y, pelvisLocation.Z);

	// 2. 물리 시뮬레이션 비활성화 및 메시 설정 복원
	tempMesh->SetSimulatePhysics(false);
	tempMesh->SetAllBodiesSimulatePhysics(false);
	tempMesh->SetCollisionProfileName(TEXT("CharacterMesh")); // 기본 충돌 프로파일로 복원

	// 3. 루트 컴포넌트를 캡슐로 변경
	tempCapsule->DetachFromParent(); // 기존 로직
	RootComponent = tempCapsule;

	// 4. 캡슐 위치를 랙돌 위치 기준으로 순간 이동 (경고 해결 핵심)
	// 캡슐의 위치 = 펠비스 위치 + 캡슐의 반 높이 (캐릭터를 서 있는 상태로 만듦)
	FVector TargetActorLocation = pelvisLocation + FVector(0, 0, tempCapsule->GetUnscaledCapsuleHalfHeight());

	// **TeleportPhysics 플래그를 사용하여 순간 이동을 엔진에 알립니다.**
	SetActorLocation(
		TargetActorLocation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics
	);

	// 5. 메시 재부착 및 상대 위치/회전 재설정
	tempMesh->AttachToComponent(tempCapsule, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	tempMesh->SetRelativeLocation(FVector(0.f, 0.f, -tempCapsule->GetUnscaledCapsuleHalfHeight()));
	tempMesh->SetWorldRotation(FRotator(0, -90, 0)); // T-Pose 회전 복구

	// 6. 캡슐 콜리전 복구
	tempCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 7. 이동 컴포넌트 재활성화
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	// 8. 애니메이션 갱신 재개
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

void ACharacterBase::UnitUnable()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance() ;

	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.0f);
	}
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (CMC)
	{
		CMC->StopMovementImmediately();
	}
}

void ACharacterBase::UnitRecoverFromUnable()
{

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



		FCDamageEvent event=AttackDataToDamageEvent(_curAttackData);
		event.ShotDirection = SweepResult.ImpactNormal;
		event.ColComp = OtherComp;
		
		



		OtherActor->TakeDamage(0.f, event, GetInstigatorController(), this);

		//UGameplayStatics::ApplyPointDamage(
		//	OtherActor,                     // 데미지를 받을 액터
		//	finalDamage,                    // 적용할 기본 데미지 값
		//	shotDirection,                  // 데미지가 들어온 방향 벡터
		//	SweepResult,                    // 충돌 정보(FHitResult)
		//	GetInstigatorController(),      // 데미지를 유발한 컨트롤러
		//	this,                           // 데미지 발생 주체 액터
		//	UDamageType::StaticClass()      // 사용할 데미지 타입 클래스
		//);


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
//
EBodyPart ACharacterBase::GetHittedPart(const FCDamageEvent* DamageEvent)
{
	FUnitPartStat* temp = nullptr;
	EBodyPart result = EBodyPart::Core;
	if (_statComponent )
	{
		
		auto HitComponent = DamageEvent->ColComp;
		{

			for (auto tag : HitComponent->ComponentTags)
			{

				 if (PartTagMap.Contains(tag))
				{
					 result = PartTagMap[tag];
					 break;
				}
			}

	
		}

		



	}
	return result;

}
FUnitPartStat* ACharacterBase::GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent, FVector hitLoc)
{
	auto datas=_statComponent->GetPartData(part);
	if (datas == nullptr)
		return nullptr;
	return &datas->PartStats[0];
}
//피격시 같은 부위를 맞았어도 조건에 따라 등일수있고 배일수 있다 그러니 델리게이트를 통해 어떤파트인지 세분화시킬 필요가 있을듯? 하다? 
// 굳이 델리게이트가 필요할까? 그냥 오버라이드 하나로 될것같기도하다.
// 반환받을것은 파트 , 입력할것은 태그와 충돌위치, 
// 자료로 필요한것도 다시 생각해보자 난 지금 파트스탯을 각부위별로 배분해놨다 이럴필요없을지도 모른다 맵 한곳에 몰아넣고 태그이름을 키로 생성해놓자. 
// 구분되는 특수한 경우가 겉의 표면을 파괴했을때 새로운 부위가 나타나는것. 그리고 등, 배로 둘의 레이어가 다를경우 이렇게 두가지 정도가 있다.
// 이때 같은 태그로 피해를 봤지만 먼저 첫번쨰가 파괴되야 두번째 레이어가 나타나는경우가 있고 
// 같은태그로 피해를 봤지만 맞는 위치의 높이에 따라 배냐 등이냐가 될수있다. 
// 우선 캐릭터의 takeDamage는 공통이다 중요한건 데미지이벤트로부터 받은 피격위치와 태그를 통핸 레이어가 중요하다. 
// 아무래도 태그마다 별개의 배열을 두는건 필요할듯하다. 맵에 하나의 키에 여러 값을 넣는건 불가하고 풀링했던것처럼 구조체와 그안에 배열을 넣는방식은 좀 귀찮다.
// 어짜피 태그가 많아져봐야 머리가슴배, 다리6개가 최대. enum의 값을 더 늘리자 
//폭발컴포넌트는 폭발로 생긴 피해를 각각의 부위에 전달하는게 옳다고 한다. 만약 범위가 엄청커서 모든부위가 폭발에겹쳐졌다면 그에 맞게 부위수만큼 TakeDamage를 걸어줘야 마땅하다.

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	//커스텀 데미지이벤트. 이곳에 피해를 입은 부위와 일반피해, 내구피해, 철거력, 관통력 등을 가져올 수 있다.그리고 상태이상을 유발한다면 얼마나가중할지도 포함된다.
	if (DamageEvent.GetTypeID() == FCDamageEvent::ClassID)
	{
		// 1. FCDamageEvent 타입으로 안전하게 캐스팅
		// &DamageEvent는 FDamageEvent const*이므로, FCDamageEvent const*로 static_cast합니다.
		const FCDamageEvent* CustomEvent = static_cast<const FCDamageEvent*>(&DamageEvent);
		//데미지타입을 가져온다 여기에는 피해의 속성과 이것이 추가적인상태이상수치를 유발하는지 여부를 가져온다.
		//데미지타입이 있다면 그것으로하고 없다면 기본클래스를 만들어 사용.
		const UCDamageType* CustomDamageType = Cast<UCDamageType>(CustomEvent->DamageTypeClass->GetDefaultObject())!=nullptr ?
			Cast<UCDamageType>(CustomEvent->DamageTypeClass->GetDefaultObject())  :
			Cast<UCDamageType>(UCDamageType::StaticClass()->GetDefaultObject());

		//상태이상부여가 걸려있다면 상태이상을 건다.
		if (CustomDamageType->_abnormalityType != EAbnormality::Max)
			_stateComp->AddAbnormality(CustomDamageType->_abnormalityType);

		_stateComp->CheckStagger(CustomEvent);

		EBodyPart part = GetHittedPart(CustomEvent);

		FUnitPartStat* partStat= GetHittedPartStat(part);
		//만약 파트가 폭발피해에 면역이라면 대신 코어가 맞도록하자
		if (partStat->_partExplosionImmunity == 1.f && CustomEvent->IsExplosionDamage == true)
		{
			partStat = _statComponent->GetCoreStat();
		}
		//본격적인 피해판정.
		
			
		_statComponent->ProcessDamage(partStat, CustomEvent, CustomDamageType->_damageType);




	}


	// 기본 로직을 반드시 호출
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}


void ACharacterBase::WeakStagger(float time)
{
	//이동을 저지. 이동속도를 0으로 바꾸고 아주잠깐 슬로우를 걸어 미약한 저지력을 만든다.
	GetCharacterMovement()->StopMovementImmediately();
	_stateComp->AddAbnormality(EAbnormality::LightStagger);
}

void ACharacterBase::StrongStagger(float time)
{
	//이동 저지 + 실행중이던 행동 저지.행동불능 잠시.
	ActionEnd();
	GetCharacterMovement()->StopMovementImmediately();
	_stateComp->AddAbnormality(EAbnormality::StrongStagger);
	

}

void ACharacterBase::KnockBack(FVector dir)
{
	// 그방향으로 밀림
	float knockbackCo = 10.f;
	LaunchCharacter(dir* knockbackCo, false, false);

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
	//TODO : 충격을 받았을떄 휘청거림 혹은 밀려남 구현 필요
	//피격시 충격이 저항력보다 크면 경직, 헬다이버는 충격량이2배이상이면 넉다운 적은 피격방향에따라 다른 애니메이션 혹은 단순히 몽타주 캔슬만.
	//if (_statComponent->GetImpactResistance() < hitPower)
		return;

	//PlayHitReaction();




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
