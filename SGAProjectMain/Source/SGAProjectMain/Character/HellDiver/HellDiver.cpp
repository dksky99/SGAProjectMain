// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiver.h"
#include "GameFramework/Character.h" // 이게 필요함
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HellDiverStateComponent.h"
#include "HellDiverStatComponent.h"
#include "HellDiverInvenComponent.h"
#include "HellDiverAnimInstance.h"
#include "HelldiverReinforceManager.h"
#include "PakourComponent.h"


#include "MotionWarpingComponent.h"
#include <Kismet/GameplayStaticsTypes.h>
#include <Kismet/GameplayStatics.h>
#include "Components/SplineMeshComponent.h"
#include "Components/SplineComponent.h"
#include "../../Object/Explosive/Grenade/TimedGrenadeBase.h"
#include "../../Object/Stratagem/Stratagem.h"
#include "../../StratagemComponent.h"

#include "../StimPackComponent.h"

#include "../../Data/UnitAttackDataAsset.h"
#include "../../Data/CollisionCameraDataAsset.h"
#include "../../CGameInstance.h"

#include "../../Gun/GunBase.h"
#include "../../Object/Item/Backpack.h"
#include "../../Object/Item/ReloadBackpack.h"
#include "../../Object/Item/SampleResources.h"

#include "../../MainGameMode.h"

AHellDiver::AHellDiver(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UHellDiverStatComponent>(StatComponentName))
{
    GetCharacterMovement()->JumpZVelocity = 300.0f;

    _statComp= Cast<UHellDiverStatComponent>(Super::_statComponent);
    _stateComponent = CreateDefaultSubobject<UHellDiverStateComponent>("State");
    Super::_stateComp = _stateComponent;
    _stimPackComponent = CreateDefaultSubobject<UStimPackComponent>("StimPack");

    _stratagemComponent = CreateDefaultSubobject<UStratagemComponent>(TEXT("StratagemComponent"));

	_trajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("ThrowSpline"));
    _trajectorySpline->SetupAttachment(GetMesh()); // 또는 RootComponent

    _pakourComponent = CreateDefaultSubobject<UPakourComponent>(TEXT("PakourComponent"));
    _motionWarpComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpComponent"));

    _invenComponent = CreateDefaultSubobject<UHellDiverInvenComponent>(TEXT("InvenComponent"));

    SetGenericTeamId(FGenericTeamId((int32)ETeamID::HellDiver));
}

void AHellDiver::BeginPlay()
{
    Super::BeginPlay();

    SetCollisionState(_stateComponent->GetCharacterState());

    _stateComponent->_characterStateChanged.AddDynamic(this, &AHellDiver::SetCollisionState);
    auto anim = Cast<UHellDiverAnimInstance>(GetMesh()->GetAnimInstance());
    if (anim != nullptr)
    {
      
        anim->_moveChanged.AddDynamic(this->_stateComponent, &UHellDiverStateComponent::MoveChangeFinish);
        anim->_lookChanged.AddDynamic(this->_stateComponent, &UHellDiverStateComponent::LookChangeFinish);
    }

    //if (_gunClass1 && _gunClass2 && _gunClass3)
    //{
    //    // 임시 세팅
    //    SpawnGun(_gunClass1);
    //    SpawnGun(_gunClass2);
    //    SpawnGun(_gunClass3);

    //    EquipGun(0); // 첫 번째 무기 장착
    //}
}

void AHellDiver::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    auto statComponent = GetStatComponent();

    if (_stateComponent->GetCharacterState() == ECharacterState::Sprinting)
    {
        statComponent->ConsumeStamina(DeltaTime);

        if (statComponent->GetCurStamina() <= 0.f) // 스테미너가 완전히 소모될 경우
        {
            FinishSprint(); // 달리기 중단
        }
    }
    else
    {
        if (!_isSprintCoolTime) // 달리기 중단 후 3초가 지나야 스테미너 회복 시작
            statComponent->RecoverStamina(DeltaTime);
    }
}

UHellDiverStateComponent* AHellDiver::GetStateComponent()
{
    return _stateComponent;
}

UHellDiverStatComponent* AHellDiver::GetStatComponent()
{
   

    return _statComp;
}

UMotionWarpingComponent* AHellDiver::GetMotionWarp() const
{
    return _motionWarpComponent;
}

UHellDiverInvenComponent* AHellDiver::GetInvenComponent()
{
    return _invenComponent;
}

void AHellDiver::EquipGrenade()
{
	if (_heldThrowable || !_grenadeClass)
		return; // 이미 들고있다

    if (_curGrenade <= 0)
        return;
    //사용전에 총을 돌려놓는다.
    UnequipGun();

    SaveLastState(3);
	GetStateComponent()->SetWeaponState(EWeaponType::Grenade);
	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = this;

	FTransform spawnTransform = GetHandSocketTransform();
	_heldThrowable = GetWorld()->SpawnActor<ATimedGrenadeBase>(_grenadeClass, spawnTransform, params);

	if (_heldThrowable)
	{
		// 손 소켓에 부착
		_heldThrowable->AttachToHand(TEXT("cc_weaponbone_r_socket")); // 던질 수 있는 오브젝트에게 맡김
    }
}

void AHellDiver::EquipStratagem()
{
	if (_heldThrowable)
		return;

	TSubclassOf<AStratagem> selectedStratagem = _stratagemComponent->GetSelectedStratagemClass();
	if (!selectedStratagem)
		return;

    UnequipGun();
	GetStateComponent()->SetWeaponState(EWeaponType::StratagemDevice);

	FActorSpawnParameters params;
	params.Owner = this;
	params.Instigator = this;

	FTransform spawnTransform = GetHandSocketTransform();
	_heldThrowable = GetWorld()->SpawnActor<AStratagem>(selectedStratagem, spawnTransform, params);

	if (_heldThrowable)
	{
		_heldThrowable->AttachToHand(TEXT("cc_weaponbone_r_socket"));
	}

}

void AHellDiver::OnThrowReleased(class UAnimMontage* Montage, bool bInterrupted)
{
	if (_heldThrowable)
	{
		FRotator throwRot = GetControlRotation();
		throwRot.Pitch += 20.0f; // 투척 각도
		FVector throwDirection = throwRot.Vector();

		_heldThrowable->Throw(throwDirection); // AThrowable 기반 함수 호출

        auto grenade = Cast<ATimedGrenadeBase>(_heldThrowable);
		_heldThrowable = nullptr;

        if (grenade) // 수류탄이면 한개 차감
        {
            _curGrenade--;

            if (_grenadeChanged.IsBound())
                _grenadeChanged.Broadcast(_curGrenade, _maxGrenade);
		    //수류탄이라면 상태를 바꿀필요없다 그레네이드 상태에서 다시 그레네이드로 돌아가면 된다.
        }

		if (_stratagemComponent) // 현재 장착한 스트라타젬 사용 쿨타임 갱신
		{
			_stratagemComponent->CommitStratagemUse();
            //스트라타젬이라면 끝났을때 원래 상태로 돌아가야한다. 
           
		}
        //원상태로 돌린다 수류탄이라면 새로운 수류탄을 손에 쥐고 
        //총이였다면 다시 그총을 들게되도록 한다.
        BackupLastState();
	}
}

void AHellDiver::UpdateThrowSpline()
{
	if (!_heldThrowable || !_trajectorySpline)
		return;

	const FVector start = GetHandSocketTransform().GetLocation();
	const FRotator throwRot = GetControlRotation() + FRotator(20.f, 0.f, 0.f);
    const float power = GetStatComponent()->GetPower();
	const FVector velocity = throwRot.Vector() * power;

	FPredictProjectilePathParams params;
	params.StartLocation = start;
	params.LaunchVelocity = velocity;
	params.bTraceWithCollision = false;
	params.ProjectileRadius = 5.0f;
	params.MaxSimTime = 1.5f;
	params.SimFrequency = 15.f;
	params.OverrideGravityZ = -980.f; // Match gravity
	params.TraceChannel = ECC_Visibility;

	FPredictProjectilePathResult result;
    UGameplayStatics::PredictProjectilePath(GetWorld(), params, result);

	_trajectorySpline->ClearSplinePoints();

	for (const auto& point : result.PathData)
	{
		_trajectorySpline->AddSplinePoint(point.Location, ESplineCoordinateSpace::World);
	}
	_trajectorySpline->UpdateSpline();


    DrawThrowSplineMeshes();
}

void AHellDiver::ClearThrowSpline()
{
	// 궤적 시각화 제거
	for (USplineMeshComponent* mesh :_trajectoryMeshPool)
	{
		if (mesh)
		{
			mesh->DestroyComponent();
		}
	}
	_trajectoryMeshPool.Empty();

	// 포인트도 제거
	if (_trajectorySpline)
	{
		_trajectorySpline->ClearSplinePoints();
	}
}

void AHellDiver::DrawThrowSplineMeshes()
{
	if (!_trajectoryMesh || !_trajectoryMaterial || !_trajectorySpline)
		return;

	// 기존 궤적 메쉬 정리
	for (USplineMeshComponent* mesh : _trajectoryMeshPool)
	{
        if (mesh)
        {
            mesh->DestroyComponent();
        }
	}
	_trajectoryMeshPool.Empty();

	const int32 pointCount = _trajectorySpline->GetNumberOfSplinePoints();
    const int32 skipCount = 2; // 앞쪽 2개 구간은 시각화 생략
	for (int32 i = skipCount; i < pointCount - 1; ++i)
	{
		const FVector start = _trajectorySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector startTangent = _trajectorySpline->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector end = _trajectorySpline->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
		const FVector endTangent = _trajectorySpline->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

		USplineMeshComponent* mesh = NewObject<USplineMeshComponent>(this);
		mesh->SetMobility(EComponentMobility::Movable);
		mesh->AttachToComponent(_trajectorySpline, FAttachmentTransformRules::KeepRelativeTransform);
		mesh->RegisterComponent();
        mesh->SetCastShadow(false);
		mesh->SetStaticMesh(_trajectoryMesh);
		mesh->SetMaterial(0, _trajectoryMaterial);
		mesh->SetStartAndEnd(start, startTangent, end, endTangent);

		mesh->SetStartScale(FVector2D(0.2f, 0.2f));
		mesh->SetEndScale(FVector2D(0.2f, 0.2f));

		_trajectoryMeshPool.Add(mesh);
	}
}

void AHellDiver::StartThrowPreview()
{
    _isPreviewingThrow = true;
    if (_throwPreviewTimer.IsValid())
        return;
    GetWorldTimerManager().SetTimer(_throwPreviewTimer, this, &AHellDiver::UpdateThrowSpline, 0.05f, true);
}

void AHellDiver::StopThrowPreview()
{
    _isPreviewingThrow = false;

    GetWorldTimerManager().ClearTimer(_throwPreviewTimer);
    ClearThrowSpline();

    if (_heldThrowable)
    {
        //_heldThrowable = nullptr;
    }
}

bool AHellDiver::AttackMelee()
{
    UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());


    if (_twoHandedMelee==nullptr|| _oneHandedMelee==nullptr)
        return false;
    if (anim == nullptr)
        return false;
    if (_stateComp->ActionBegin() == false)
        return false;


    switch (_stateComponent->GetWeaponState())
    {
    case EWeaponType::Gun:

        _curAttackData = _twoHandedMelee;
        break;
    case EWeaponType::None:
    case EWeaponType::Grenade:
    case EWeaponType::StratagemDevice:

        _curAttackData = _oneHandedMelee;
        break;
    default:
        break;
    }


    const float Duration = anim->PlayAnimMontage(_curAttackData->Motion);

    SetMeleeColisions(_curAttackData);
    return true;
}

void AHellDiver::UseStimPack()
{
    if (_stateComponent->IsActionable() == false)
        return ;
    _stimPackComponent->UseStimPack();
}

void AHellDiver::StartSprint()
{
    auto statComponent = GetStatComponent();

    if (statComponent->GetCurStamina() <= 0.f)
        return;
    if (_stateComponent->StartSprint() == false)
        return;
    Sprinting();
}

void AHellDiver::FinishSprint()
{
    if (_stateComponent->FinishSprint() == false)
        return;
    Standing();

    _isSprintCoolTime = true; // 스테미너 쿨타임 시작 -> 그동안 스테미너 회복 안됨
    GetWorld()->GetTimerManager().SetTimer(_sprintCooldownHandle, [this]()
        {
            _isSprintCoolTime = false;
        }, 3.f, false); // 3초 후 쿨타임 종료
}

void AHellDiver::StartCrouch()
{
    if (_stateComponent->StartCrouch() == false)
        return;
    Crouch();
}

void AHellDiver::FinishCrouch()
{
    if (_stateComponent->FinishCrouch() == false)
        return;

    Standing();
}

void AHellDiver::StartProne()
{
    if (_stateComponent->StartProne() == false)
        return;
    Proning();
}

void AHellDiver::FinishProne()
{
    if (_stateComponent->FinishProne() == false)
        return;
    Standing();
}

void AHellDiver::Rolling()
{
    //롤링을 할때 왼쪽으로가다가 롤링을하면 화면의 정면을바라보며 왼쪽으로 뛰어야한다
    //즉 시전 직전에 액터가 키입력을 받은 바라본 방향을  바라보며 그상태로 엎드림상태로 되야한다.
    // 하지만 그방향으로 엎드리게된다 이러면안된다. 방법을 찾아보자.
    if (this->CanJump() == false)
        return;
    if (_stateComponent->IsRolling())
        return;
    if (_stateComponent->StartRolling()==false)
        return;
    Jump();
    StartProne();
    FVector forward;
    if (_vertical == 0 && _horizontal == 0)
    {
        forward = GetActorForwardVector();
    }
    else
    {

        forward = GetActorForwardVector() * _vertical + GetActorRightVector() * _horizontal;
    }
    forward.Normalize();
    SetActorRotation(forward.ToOrientationQuat());
    float forwardBoost = 500.0f; 
    FVector boost = forward * forwardBoost;

    // 4. 현재 Velocity에 더해줌
    GetCharacterMovement()->Velocity += boost;

}

void AHellDiver::FinishRolling()
{

    _stateComponent->FinishRolling();
}

void AHellDiver::Standing()
{
    auto statComponent = GetStatComponent();

    SetCollisionState(_stateComponent->GetCharacterState());
    _statComp->ChangeSpeed(statComponent->GetDefaultSpeed());
}

void AHellDiver::Sprinting()
{
    auto statComponent = GetStatComponent();

    SetCollisionState(_stateComponent->GetCharacterState());
    _statComp->ChangeSpeed(statComponent->GetSprintSpeed());
}

void AHellDiver::Crouching()
{
    auto statComponent = GetStatComponent();

    SetCollisionState(_stateComponent->GetCharacterState());
    _statComp->ChangeSpeed(statComponent->GetCrouchSpeed());
}

void AHellDiver::Proning()
{
    auto statComponent = GetStatComponent();

    SetCollisionState(_stateComponent->GetCharacterState());

    _statComp->ChangeSpeed(statComponent->GetProneSpeed());
}


void AHellDiver::InitWeapon()
{
    UCGameInstance* GI = Cast<UCGameInstance>(GetGameInstance());
    if (GI->GetGamePhase() == EGamePhase::InMission)
        EquipGun(0);
}

void AHellDiver::EquipGun(int32 index)
{
    _invenComponent->EquipGun(index);
    _stateComponent->SetWeaponState(EWeaponType::Gun);
}

void AHellDiver::PickupGun(AGunBase* gun)
{
    int32 index = gun->GetGunSlotIndex();
    if (index == -1) return;

    gun->SetOwner(this);
    gun->InitializeGun();
    _invenComponent->SetGun(gun); // 안벤토리 슬롯 관리
    _invenComponent->EquipGun(index);
    _stateComponent->SetWeaponState(EWeaponType::Gun);
}

void AHellDiver::SwitchGun(int32 index)
{
    if (!_invenComponent->CanSwitchGun(index))
        return; // 바꿀 수 없으면 중단하고 리턴 false

    // 바꿀 수 있다면 장전 중단
    if (_stateComponent->IsReloading())
        _invenComponent->GetEquippedGun()->CancelReload();

    // 현재 상태 저장 후
    bool wasAiming = _stateComponent->IsAiming();
    bool wasFiring = _stateComponent->IsFiring();

    // 하던 행동 중단
    _stateComponent->SetAiming(false);
    _stateComponent->SetFiring(false);

    // 이전 총 비활성화
    auto prevGun = _invenComponent->GetEquippedGun();
    prevGun->DeactivateGun();
    _invenComponent->PutBackWeapon(prevGun);
    OnPreSwitchGun(prevGun);
    // 총 변경
    EquipGun(index); 
    SaveLastState(index);

    // 현재 총 활성화
	auto newGun = _invenComponent->GetEquippedGun();
    _invenComponent->BringWeapon(newGun);
	OnPostSwitchGun(newGun);
    newGun->ActivateGun();

    _stateComponent->SetEquipIndex(index);

    if (wasAiming) // 에임 중이었을 경우 유지
        StartAiming();

    if (wasFiring) // 사격 중이었을 경우 유지
        StartFiring();
}

AGunBase* AHellDiver::GetEquippedGun()
{
    return _invenComponent->GetEquippedGun();
}

void AHellDiver::UnequipGun()
{

    // 이전 총 비활성화
    auto prevGun = _invenComponent->GetEquippedGun();
    prevGun->DeactivateGun();
    _invenComponent->PutBackWeapon(prevGun);
    OnPreSwitchGun(prevGun);
}

void AHellDiver::SaveLastState(int32 index)
{
    _lastState = index;

}

void AHellDiver::BackupLastState()
{
    switch (_lastState)
    {
    case 0:
    case 1:
    case 2:
    {
        SwitchGun(_lastState);
    }
    break;
    case 3:
    default:
        EquipGrenade();
        break;
    }
}

void AHellDiver::StartFiring()
{
    _stateComponent->SetFiring(true);
    _invenComponent->GetEquippedGun()->StartFire();
}

void AHellDiver::StopFiring()
{
    _stateComponent->SetFiring(false);
    _invenComponent->GetEquippedGun()->StopFire();
}

void AHellDiver::StartAiming()
{
    _stateComponent->SetAiming(true);
    _invenComponent->GetEquippedGun()->StartAiming();

}

void AHellDiver::StopAiming()
{
    _stateComponent->SetAiming(false);
    _invenComponent->GetEquippedGun()->StopAiming();
}

void AHellDiver::Reload()
{
    if (_stateComponent->IsReloading()) return;

    auto equippedGun = _invenComponent->GetEquippedGun();

    //if (auto projectileGun = Cast<AExplosiveGun>(equippedGun)) // 폭발성 총일 경우
    //{
    //    auto backpack = _invenComponent->GetBackpack(); // 가방이 없으면 리턴
    //    if (!backpack) return;

    //    auto reloadBackpack = Cast<AReloadBackpack>(backpack); // 가방이 있어도
    //    if (!reloadBackpack) return;                            // 장전용이 아니면 리턴
    //    if (reloadBackpack->GetCurBulletCount() <= 0) return;   // 혹은 가방에 총알이 없으면 리턴
    //}

    equippedGun->Reload();
}

bool AHellDiver::CanReloadUsingBackpack()
{
    if (auto reloadBackpack = Cast<AReloadBackpack>(_invenComponent->GetBackpack()))
        return reloadBackpack->GetCurSpareCount() > 0;

    return false;
}

void AHellDiver::EquipBackpack(ABackpack* backpack)
{
    _invenComponent->EquipBackpack(backpack);
}

void AHellDiver::UseBackpack(int32 amount)
{
    if (auto reloadBackpack = Cast<AReloadBackpack>(_invenComponent->GetBackpack()))
        reloadBackpack->ConsumeSpare(amount);
}

void AHellDiver::AddSample(FSampleBundle sample)
{
    _invenComponent->AddSample(sample);
}

void AHellDiver::RefillAllItem()
{
    RefillSpare();
    RefillGrenade();
    RefillStimPack();
}

void AHellDiver::RefillSpare()
{
    auto gunSlot = _invenComponent->GetAllGun();

    for (auto gun : gunSlot)
    {
        if (gun)
        {
            gun->RefillMag();
        }
    }
}

void AHellDiver::RefillGrenade()
{
    _curGrenade += 2;
    
    if (_curGrenade > _maxGrenade)
        _curGrenade = _maxGrenade;

    if (_grenadeChanged.IsBound())
        _grenadeChanged.Broadcast(_curGrenade, _maxGrenade);
}

void AHellDiver::RefillStimPack()
{
    _stimPackComponent->RefillStimPack();
}

void AHellDiver::StratagemInputting()
{
    if (_stratagemInputMontage==nullptr) return;
    UE_LOG(LogTemp, Display, TEXT("StratagemInput"));
    if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
    {

        UE_LOG(LogTemp, Display, TEXT("StratagemInputSuccess"));
        animInstance->PlayAnimMontage(_stratagemInputMontage);


    }
}

void AHellDiver::MotionChangeFinish()
{

}

void AHellDiver::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    float zVelocity = GetCharacterMovement()->Velocity.Z;

    UE_LOG(LogTemp, Log, TEXT("Landing Z Velocity: %f"), zVelocity);
    
    if (_stateComponent->IsRolling())
    {

        // 일정 시간 후 복구
        GetWorld()->GetTimerManager().SetTimer(
            _rollingTimerHandle, this, &AHellDiver::FinishRolling, 0.2, false
        );

    }

    if (zVelocity < -1200.f)
    {
       
       if (_stateComponent->IsRolling())
       {
                FinishRolling();

                KnockDown();

           

        }
        else
            HardLanding();

    }
    else if (zVelocity < -600.f)
    {
        
        SoftLanding();
    }

    else if (zVelocity < -200.f)
    {
        // 일반 착지
    }

}

void AHellDiver::SoftLanding()
{
    if (!_softLandingMontage) return;

    if (_stateComponent->GetCharacterState() != ECharacterState::Standing || _stateComponent->GetCharacterState() != ECharacterState::Sprinting)
    {
        KnockDown();
    }
    else
    {

        if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
        {

            animInstance->PlayAnimMontage(_softLandingMontage);

            // 재생 후 인스턴스 가져오기
            if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(_softLandingMontage))
            {
                _stateComponent->BeShocked();
                // 델리게이트 중복 방지
                MontageInstance->OnMontageEnded.Unbind();

                // 델리게이트 바인딩
                MontageInstance->OnMontageEnded.BindUObject(this, &AHellDiver::FinishLanding);

                UE_LOG(LogTemp, Error, TEXT("Success to get MontageInstance for %s"), *_softLandingMontage->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *_softLandingMontage->GetName());
            }

        }
    }
}

void AHellDiver::HardLanding()
{
    if (!_hardLandingMontage) return;
    if (_stateComponent->GetCharacterState() != ECharacterState::Standing || _stateComponent->GetCharacterState() != ECharacterState::Sprinting)
    {
        KnockDown();
    }
    else
    {


        if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
        {

            animInstance->PlayAnimMontage(_hardLandingMontage);

            // 재생 후 인스턴스 가져오기
            if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(_hardLandingMontage))
            {
                _stateComponent->BeShocked();

                // 델리게이트 중복 방지
                MontageInstance->OnMontageEnded.Unbind();

                // 델리게이트 바인딩
                MontageInstance->OnMontageEnded.BindUObject(this, &AHellDiver::FinishLanding);

                UE_LOG(LogTemp, Error, TEXT("Success to get MontageInstance for %s"), *_hardLandingMontage->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *_hardLandingMontage->GetName());
            }

        }
    }
}

void AHellDiver::FinishLanding(UAnimMontage* Montage, bool bInterrupted)
{
    _stateComponent->RecoveryShocked();
}

FRotator AHellDiver::Focusing()
{


    FRotator socketRot = GetMesh()->GetSocketRotation(TEXT("spine_05"));
    return socketRot;
}

void AHellDiver::Throwing()
{
    if (!_throwingMontage) return;
    if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance()))
    {

        animInstance->PlayAnimMontage(_throwingMontage);

        // 재생 후 인스턴스 가져오기
        if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(_throwingMontage))
        {

            // 델리게이트 중복 방지
            MontageInstance->OnMontageEnded.Unbind();

            // 델리게이트 바인딩
            MontageInstance->OnMontageEnded.BindUObject(this, &AHellDiver::OnThrowReleased);

            UE_LOG(LogTemp, Error, TEXT("Success to get MontageInstance for %s"), *_throwingMontage->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *_throwingMontage->GetName());
        }

    }
}

//float AHellDiver::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
//{
//    _statComponent->ChangeHp(-damageAmount);
//
//    UE_LOG(LogTemp, Log, TEXT("Damage : %f"), damageAmount);
//
//    if (_statComponent->IsDead())
//        Dead();
//
//    return -damageAmount;
//}

FTransform AHellDiver::GetLeftHandSocketTransform() const
{
    auto equippedGun = _invenComponent->GetEquippedGun();

    if (equippedGun == nullptr)
    {
        return GetActorTransform(); // fallback
    }
    FTransform temp = equippedGun->GetLeftHandleTrans();
    FVector resultLoc;
    FRotator resultRot;
    GetMesh()->TransformToBoneSpace(TEXT("hand_r"), temp.GetLocation(), temp.GetRotation().Rotator(), resultLoc,resultRot);
    temp.SetLocation(resultLoc+FVector(0,6,0));
    temp.SetRotation(resultRot.Quaternion());
    return temp ;
}

FTransform AHellDiver::GetMuzzleTransform() const
{
    auto equippedGun = _invenComponent->GetEquippedGun();

    if (equippedGun == nullptr)
    {
        return GetActorTransform(); // fallback
    }
    FTransform temp = equippedGun->GetMuzzleTrans();
    return temp;
}

void AHellDiver::KnockDown(float time)
{
    Super::KnockDown();

    _stateComponent->KnockDown();

    SetCollisionState(_stateComponent->GetCharacterState());

}

void AHellDiver::RecoverFromKnockDown()
{
    Super::RecoverFromKnockDown();

    _stateComponent->KnockDownRecovery();
    Proning();
}

void AHellDiver::Dead()
{

    //래그돌로 변하고 
    Super::Dead();

    UE_LOG(LogTemp, Display, TEXT("AHellDiver Dead CB : %s"), *this->GetName());
    _stateComponent->Dead();

    _invenComponent->DropGun(0);
    _invenComponent->DropGun(1);
    _invenComponent->DropGun(2);

    _invenComponent->DropBackpack();
    _invenComponent->DropSample();


    if (GetController())
    {

        AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
        if (!GM)return;
        if (!GM->GetHelldiverReinforceManager()) return;
        AController* temp = GetController();
        GetController()->UnPossess();
        GM->GetHelldiverReinforceManager()->ReturnDeadController(temp);

    }

    GetWorld()->GetTimerManager().SetTimer(_knockDownTimerHandle, this, &AHellDiver::AfterDead, 5.0f, false);
}

void AHellDiver::AfterDead()
{
    UE_LOG(LogTemp, Display, TEXT("AHellDiver AfterDead CB : %s"), *this->GetName());
    
    AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)return;
    if (!GM->GetHelldiverReinforceManager()) return;
    GM->GetHelldiverReinforceManager()->ReinforceHelldiver(GetActorLocation());
      


    //60초뒤 레벨에서 래그돌이 사라지고소환가능상태가됨.
    GetWorldTimerManager().SetTimer(_knockDownTimerHandle, this, &AHellDiver::RecoverFromDead, 60.0f, false);

}

void AHellDiver::RecoverFromDead()
{
    Super::RecoverFromDead();
    _stateComponent->SetLifeState(ELifeState::Alive);
    _isReadyToSpawn = true;
}

FTransform  AHellDiver::GetHandSocketTransform() const
{
	USkeletalMeshComponent* mesh = GetMesh();
	if (mesh && mesh->DoesSocketExist(TEXT("cc_weaponbone_r")))
	{
		return mesh->GetSocketTransform(TEXT("cc_weaponbone_r"));
	}
	return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip1SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("cc_pocket01_L")))
    {
        return mesh->GetSocketTransform(TEXT("cc_pocket01_L"));
    }
    return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip2SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("cc_pocket01_R")))
    {
        return mesh->GetSocketTransform(TEXT("cc_pocket01_R"));
    }
    return GetActorTransform(); // fallback
}

FTransform AHellDiver::GetEquip3SocketTransform() const
{
    USkeletalMeshComponent* mesh = GetMesh();
    if (mesh && mesh->DoesSocketExist(TEXT("cc_backpack")))
    {
        return mesh->GetSocketTransform(TEXT("cc_backpack"));
    }
    return GetActorTransform(); // fallback
}

void AHellDiver::SetCollisionState(ECharacterState newState)
{


    switch (newState)
    {
    case ECharacterState::Standing:
    case ECharacterState::Sprinting:
        SetStandingCollisionCamera();
        break;
    case ECharacterState::Crouching:
        SetCrouchingCollisionCamera();
        break;
    case ECharacterState::Proning:
    case ECharacterState::Knockdown:
        SetProningCollisionCamera();
        break;
    case ECharacterState::MAX:
    default:
        break;
    }

}

void AHellDiver::SetCollisionCamera(UCollisionCameraDataAsset data)
{
    

}

void AHellDiver::SetStandingCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_standingStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_standingStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_standingStance->_capsuleHalfHeight));
}

void AHellDiver::SetCrouchingCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_crouchingStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_crouchingStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_crouchingStance->_capsuleHalfHeight));
    
}

void AHellDiver::SetProningCollisionCamera()
{
    GetCapsuleComponent()->SetCapsuleRadius(_proningStance->_capsuleRadius);
    GetCapsuleComponent()->SetCapsuleHalfHeight(_proningStance->_capsuleHalfHeight);
    GetMesh()->SetRelativeLocation(FVector(0, 0, -_proningStance->_capsuleHalfHeight));
}
