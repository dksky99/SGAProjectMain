// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/ImpactMarker.h"
#include "../UI/CrosshairWidget.h"
#include "Blueprint/UserWidget.h"

#include "Component/GunFireComponent.h"
#include "Component/GunAmmoComponent.h"
#include "Component/GunEffectComponent.h"
#include "Component/Damage/GunDamageComponent.h"
#include "Component/Attachment/GunAttachmentComponent.h"
#include "Component/Attachment/GunTacticalLightComponent.h"
#include "Component/Attachment/GunScopeComponent.h"

#include "../CGameInstance.h"

#include "../Character/HellDiver/HellDiver.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "../SGAProjectMain.h"

// Sets default values
AGunBase::AGunBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_gunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	_gunMesh->SetCollisionProfileName("PhysicsActor");
	_gunMesh->SetGenerateOverlapEvents(true);
	_gunMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	RootComponent = _gunMesh;

	_interactionMark->SetupAttachment(RootComponent);

	if (_mesh)  // AItemBase의 StaticMesh 삭제
	{ 
		_mesh->DestroyComponent();
		_mesh->SetHiddenInGame(true);
	}
}

// Called when the game starts or when spawned
void AGunBase::BeginPlay()
{
	Super::BeginPlay();

	auto gameInstance = Cast<UCGameInstance>(GetWorld()->GetGameInstance());
	if (gameInstance)
	{
		const auto data = gameInstance->GetGunDataFromTable(_gunID);
		SetGunData(data);
	}

	_interactableInfo._interactionText = _gunData._name;
	_interactableInfo._type = EInteractableIconType::Gun;

	InitializeMark();

	if (_gunData._crosshairClass)
	{
		_crosshair = CreateWidget<UCrosshairWidget>(GetWorld(), _gunData._crosshairClass);

		if (_crosshair)
		{
			_crosshair->AddToViewport();
			_crosshair->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AGunBase::SetGunComponent()
{
	_fireComp = NewObject<UGunFireComponent>(this, _gunData._fireComponentClass, TEXT("FireComponent"));
	_fireComp->RegisterComponent();
	_fireComp->_fireEvent.AddUObject(this, &AGunBase::Fire);
	_ammoComp = NewObject<UGunAmmoComponent>(this, _gunData._ammoComponentClass, TEXT("AmmoComponent"));
	_ammoComp->RegisterComponent();
	_damageComp = NewObject<UGunDamageComponent>(this, _gunData._damageComponentClass, TEXT("DamageComponent"));
	_damageComp->RegisterComponent();
	_effectComp = NewObject<UGunEffectComponent>(this, UGunEffectComponent::StaticClass(), TEXT("EffectComponent"));
	_effectComp->RegisterComponent();

	for (auto attachCompClass : _gunData._attachmentComponentClasses)
	{
		auto attachComp = NewObject<UGunAttachmentComponent>(this, attachCompClass);
		attachComp->RegisterComponent();
		_attachmentComps.AddUnique(attachComp);
	}

	if (auto* lightComp = FindComponentByClass<UGunTacticalLightComponent>())
		_lightComp = lightComp;
	if (auto* scopeComp = FindComponentByClass<UGunScopeComponent>())
		_scopeComp = scopeComp;
}

// Called every frame
void AGunBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RecoverRecoil(DeltaTime);
	if (!_isActive) return;
	if (!_owner) return;

	if (!_owner->GetStateComponent()->IsAiming())
		return;

	FHitResult hitResult = GetHitResult(ECC_Visibility); // 가시성 충돌 채널 (UI용)
	FVector hitPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	for (auto attachComp : _attachmentComps)
		attachComp->UpdateAttachment(_gunMesh, hitPoint);

	if (!IsValid(_marker))
		_marker = GetWorld()->SpawnActor<AImpactMarker>(_gunData._impactMarkerClass, hitPoint, FRotator::ZeroRotator);
	else
		_marker->SetActorLocation(hitPoint);
}

bool AGunBase::CanFire()
{
	if (!_owner)
		return false;
	if (_owner->GetStateComponent()->IsReloading())
		return false;
	if (!_ammoComp->CanFire())
		return false;
	return true;
}

void AGunBase::StartFire()
{
	if (!CanFire()) return;

	_fireComp->StartFire();
}

void AGunBase::Fire()
{
	_damageComp->FireShot(GetMuzzleLoc(), GetMuzzleRot(), _owner->GetStateComponent()->IsAiming());
	
	ApplyFireRecoil(); // 반동 계산

	_ammoComp->ConsumeAmmo();

	_effectComp->PlayFireEffect();
}

void AGunBase::StopFire()
{
	_fireComp->StopFire();
}

void AGunBase::StartAiming()
{
	if (_owner->GetStateComponent()->IsReloading())
		return;

	_owner->GetStateComponent()->SetAiming(true);

	if (_marker)
		_marker->SetActorHiddenInGame(false);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Visible);

	for (auto attachComp : _attachmentComps)
		attachComp->OnAimChanged(true);
}

void AGunBase::StopAiming()
{
	if (!_owner) return;
	_owner->GetStateComponent()->SetAiming(false);

	if (_marker)
		_marker->SetActorHiddenInGame(true);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Hidden);

	for (auto attachComp : _attachmentComps)
		attachComp->OnAimChanged(false);
}

void AGunBase::InitializeGun()
{
	if (AHellDiver* owner = Cast<AHellDiver>(GetOwner()))
	{
		SetOwner(owner);
		_owner = owner;
		//AttachToHand();
		UE_LOG(LogTemp, Log, TEXT("Initialize Gun"));
	}

	DeactivateGun();
}

void AGunBase::ActivateGun()
{
	_isActive = true;
	SetActorHiddenInGame(false);

	_recoilToRecover = FRotator::ZeroRotator;

	for (auto attachComp : _attachmentComps)
		attachComp->ActivateAttachment(true);

	_ammoComp->BroadcastAmmoAndSpareChanged(); // TODO) 여기서 해주는 것이 맞는가?
}

void AGunBase::DeactivateGun()
{
	StopFire();
	StopAiming();

	_isActive = false;

	if (_marker)
		_marker->SetActorHiddenInGame(true);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Hidden);

	for (auto attachComp : _attachmentComps)
		attachComp->ActivateAttachment(false);
}

void AGunBase::AttachToHand()
{
	if (_owner)
	{
		if (USkeletalMeshComponent* characterMesh = _owner->GetMesh())
		{
			// 물리 & 충돌 비활성화
			if (_gunMesh)
			{
				_gunMesh->SetSimulatePhysics(false);
				_gunMesh->SetEnableGravity(false);
				_gunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			DetachRootComponentFromParent();
			AttachToComponent(characterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("cc_weaponbone_r_socket"));
		}
	}
}

void AGunBase::Reload()
{
	if (!_ammoComp->CanReload())
		return;

	StopAiming();
	StopFire();

	_ammoComp->Reload();
}

void AGunBase::OnReloadSectionEnded()
{
	_ammoComp->OnReloadSectionEnded();
}

void AGunBase::CancelReload()
{
	_ammoComp->CancelReload();
}

void AGunBase::RefillMag()
{
	_ammoComp->RefillSpare();
}

void AGunBase::RecoverRecoil(float DeltaTime)
{
	if (!_owner) return;

	APlayerController* playerController = Cast<APlayerController>(_owner->GetController());
	if (playerController)
	{
		float recoverPitch = FMath::FInterpTo(_recoilToRecover.Pitch, 0.f, DeltaTime, _recoilRecoverSpeed);
		float recoverYaw = FMath::FInterpTo(_recoilToRecover.Yaw, 0.f, DeltaTime, _recoilRecoverSpeed);

		// 변화량을 반대로 되돌림
		playerController->AddPitchInput(_recoilToRecover.Pitch - recoverPitch);
		playerController->AddYawInput(recoverYaw - _recoilToRecover.Yaw);

		_recoilToRecover.Pitch = recoverPitch;
		_recoilToRecover.Yaw = recoverYaw;
	}
}

void AGunBase::ApplyFireRecoil()
{
	float recoilMultiplier = GetRecoilMultiplier();

	// 수직 반동
	float vertical = FMath::RandRange(0.9f, 1.1f) * _gunData._verticalRecoil / _verticalRecoilDamp;
	// 수평 반동
	float horizontal = FMath::RandRange(-1.f, 1.f) * _gunData._horizontalRecoil / _horizontalRecoilDamp;

	APlayerController* playerController = Cast<APlayerController>(_owner->GetController());
	if (playerController)
	{
		playerController->AddPitchInput(-vertical * recoilMultiplier);
		playerController->AddYawInput(horizontal * recoilMultiplier);

		// 카메라 복구용
		_recoilToRecover.Pitch += vertical * recoilMultiplier;
		_recoilToRecover.Yaw += horizontal * recoilMultiplier;
	}
}

float AGunBase::GetRecoilMultiplier()
{
	float base = 1.f;

	if (_owner)
	{
		FVector velocity = _owner->GetVelocity();

		// 움직이고 있지 않다면
		if (velocity.SizeSquared() < FMath::Square(10.f))
		{
			switch (_owner->GetStateComponent()->GetCharacterState())
			{
			case ECharacterState::Standing:
				base = 0.8f;
				break;
			case ECharacterState::Crouching:
				base = 0.6f;
				break;
			case ECharacterState::Proning:
				base = 0.4f;
				break;
			}
		}
		// 움직이고 있다면
		else
		{
			switch (_owner->GetStateComponent()->GetCharacterState())
			{
			case ECharacterState::Standing:
				base = 1.5f;
				break;
			case ECharacterState::Crouching:
				base = 1.f;
				break;
			}
		}
	}
	return base;
}


FHitResult AGunBase::GetHitResult(ECollisionChannel TraceChannel)
{
	// 총구 위치에서 총구가 향하는 방향으로 발사
	FVector muzzleLocation = _gunMesh->GetSocketLocation(TEXT("Muzzle"));
	FVector fireDirection = _gunMesh->GetSocketRotation(TEXT("Muzzle")).Vector();

	// 조준하고 있지 않을 경우 탄퍼짐
	if (!_owner->GetStateComponent()->IsAiming())
	{
		fireDirection = FMath::VRandCone(fireDirection, FMath::DegreesToRadians(_gunData._shakeAmount));
	}

	FVector end = muzzleLocation + fireDirection * 10000.f;

	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(
		hitResult,
		muzzleLocation,
		end,
		TraceChannel);

	//FVector hitPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	//DrawDebugLine(GetWorld(), hitResult.TraceStart, hitPoint, FColor::Yellow, false, 1.0f);

	return hitResult;
}

void AGunBase::ChangeFireMode()
{
	_fireComp->ChangeFireMode();
}

void AGunBase::ChangeTacticalLightMode()
{
	if (!_lightComp) return;

	_lightComp->ChangeTacticalLightMode(_owner->GetStateComponent()->IsAiming());
}

void AGunBase::ChangeScopeMode()
{
	if (!_scopeComp) return;

	_scopeComp->ChangeScopeMode();
}

FGunModes AGunBase::GetGunModes()
{
	FGunModes gunModes;
	if (_fireComp)
		gunModes._fireModes = _fireComp->GetFireModes();
	if (_lightComp)
		gunModes._lightModes = _lightComp->GetLightModes();
	if (_scopeComp)
		gunModes._scopeModes = _scopeComp->GetScopeModes();
	return gunModes;
}

void AGunBase::PickupItem(AHellDiver* player)
{
	player->PickupGun(this);

	Super::PickupItem(_owner);
}

int32 AGunBase::GetGunSlotIndex()
{
	if (_gunData._slotType == EGunSlotType::Primary)
		return 0;
	else if (_gunData._slotType == EGunSlotType::Secondary)
		return 1;
	else if (_gunData._slotType == EGunSlotType::Support)
		return 2;
	else
		return -1; // 유효하지 않음
}

void AGunBase::SetGunData(const FGunData& gunData)
{
	_gunData = gunData;
	SetGunComponent();
	_fireComp->SetFireModeData(_gunData._fireModes);
	_ammoComp->SetAmmoData(gunData);
	_damageComp->SetDamageData(gunData);
	_effectComp->InitializeEffect(_gunMesh);
	for (auto attachComp : _attachmentComps)
		attachComp->InitializeAttachment(_gunMesh);
}

int32 AGunBase::GetCurAmmo()
{
	return _ammoComp->GetCurAmmo();
}

EFireMode AGunBase::GetCurFireMode()
{
	return _fireComp->GetCurFireMode();
}

ETacticalLightMode AGunBase::GetCurLightMode()
{
	if (_lightComp)
		return _lightComp->GetCurLightMode();
	else 
		return ETacticalLightMode::LightOff;
}

int32 AGunBase::GetCurScopeMode()
{
	if (_scopeComp)
		return _scopeComp->GetCurScopeMode();
	else 
		return 0;
}

FTransform AGunBase::GetMuzzleTrans()
{
	return  _gunMesh->GetSocketTransform(TEXT("Muzzle"),RTS_World);
}

FVector AGunBase::GetMuzzleLoc()
{
	return _gunMesh->GetSocketLocation(TEXT("Muzzle"));;
}

FRotator AGunBase::GetMuzzleRot()
{
	return _gunMesh->GetSocketRotation(TEXT("Muzzle"));
}

FTransform AGunBase::GetLeftHandleTrans()
{
	if(_gunMesh==nullptr)
		return GetActorTransform();
	if(_gunMesh->DoesSocketExist(TEXT("LeftGrip")))
		return _gunMesh->GetSocketTransform(TEXT("LeftGrip"),RTS_World);
	return _gunMesh->GetComponentTransform();
}

void AGunBase::ShowHitMarker(EHitOutcome hitOutcome)
{
	_crosshair->ShowHitMarker(hitOutcome);
}

