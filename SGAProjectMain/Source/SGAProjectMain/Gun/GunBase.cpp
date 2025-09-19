// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"

#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/ImpactMarker.h"
#include "Blueprint/UserWidget.h"

#include "GunFireComponent.h"
#include "GunAmmoComponent.h"

#include "../CGameInstance.h"

#include "../Character/HellDiver/HellDiver.h"
#include "../Character/HellDiver/HellDiverStateComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SpotLightComponent.h"

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

	_fireComp = CreateDefaultSubobject<UGunFireComponent>(TEXT("GunFireComponent"));
	_ammoComp = CreateDefaultSubobject<UGunAmmoComponent>(TEXT("GunAmmoComponent"));


	_interactionMark->SetupAttachment(RootComponent);

	if (_mesh)  // AItemBase의 StaticMesh 삭제
	{ 
		_mesh->DestroyComponent();
		_mesh->SetHiddenInGame(true);
	}

	_tacticalLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	_tacticalLight->SetupAttachment(RootComponent);
	_tacticalLight->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> fireNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire.NS_WeaponFire'"));
	if (fireNS.Succeeded())
	{
		_fireNS = fireNS.Object;
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> shellEjectNS(TEXT("/Script/Niagara.NiagaraSystem'/Game/Graphics/Gun/Effect/NS_WeaponFire_ShellEject.NS_WeaponFire_ShellEject'"));
	if (shellEjectNS.Succeeded())
	{
		_shellEjectNS = shellEjectNS.Object;
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

	_fireComp->_fireEvent.AddUObject(this, &AGunBase::Fire);

	_interactableInfo._interactionText = _gunData._name;
	_interactableInfo._type = EInteractableIconType::Gun;

	InitializeMark();

	if (_gunData._crosshairClass)
	{
		_crosshair = CreateWidget<UUserWidget>(GetWorld(), _gunData._crosshairClass);

		if (_crosshair)
		{
			_crosshair->AddToViewport();
			_crosshair->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (_gunData._laserFX)
	{
		_laserpointer = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_gunData._laserFX,
			_gunMesh,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (_gunData._laserImpactFX)
	{
		_laserImpact = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_gunData._laserImpactFX,
			_gunMesh,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}

	if (_fireNS)
	{
		_fireEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_fireNS,
			_gunMesh,
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);
		_fireEffect->SetAutoDestroy(false);
		_fireEffect->Deactivate();
	}

	if (_shellEjectNS)
	{
		_shellEjectEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
			_shellEjectNS,
			_gunMesh,
			TEXT("ShellEject"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false
		);
		_shellEjectEffect->SetAutoDestroy(false);
		_shellEjectEffect->Deactivate();
	}

	_tacticalLightMode = _gunData._lightModes[0];
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

	if (_laserpointer)
		UseLaserPoint(hitPoint);

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
	ExecuteShot(); // 라인트레이스
	
	ApplyFireRecoil(); // 반동 계산

	_ammoComp->ConsumeAmmo();

	PlayFireEffect();
}

void AGunBase::StopFire()
{
	_fireComp->StopFire();
}

void AGunBase::ExecuteShot()
{
	FColor drawColor = FColor::Green; // 디버깅용

	FHitResult hitResult = GetHitResult(ECC_GameDamage); // 데미지 충돌 채널

	if (hitResult.bBlockingHit)
	{
		drawColor = FColor::Red;
		float distance = FVector::Dist(hitResult.TraceStart, hitResult.ImpactPoint);
		float finalDamage = CalculateDamage(distance / 100.f);

		if (finalDamage < 0) return;

		if (ACharacterBase* character = Cast<ACharacterBase>(hitResult.GetActor()))
		{
			//UGameplayStatics::ApplyDamage(character, finalDamage, _owner->GetController(), this, nullptr);

			const FVector shotDirection = (hitResult.TraceStart - hitResult.ImpactPoint).GetSafeNormal();
			UGameplayStatics::ApplyPointDamage(
				character,                             // 데미지를 받을 액터
				finalDamage,                           // 적용할 데미지 값
				shotDirection,                         // 데미지가 들어온 방향(단위 벡터)
				hitResult,                             // 충돌에 대한 자세한 정보(FHitResult)
				_owner->GetController(),               // 데미지를 일으킨 컨트롤러(Instigator)
				this,                                  // 데미지를 발생시킨 액터(Damage Causer)
				UDamageType::StaticClass()             // 사용할 데미지 타입 클래스
			);
		}
	}

	FVector hitPoint = hitResult.bBlockingHit ? hitResult.ImpactPoint : hitResult.TraceEnd;
	DrawDebugLine(GetWorld(), hitResult.TraceStart, hitPoint, drawColor, false, 1.0f);
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

	if (_laserpointer && _laserImpact)
	{
		_laserpointer->SetVisibility(true);
		_laserImpact->SetVisibility(true);
	}

	if (_tacticalLight)
		UseTacticalLight(true);
}

void AGunBase::StopAiming()
{
	if (!_owner) return;
	_owner->GetStateComponent()->SetAiming(false);

	if (_marker)
		_marker->SetActorHiddenInGame(true);
	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Hidden);

	if (_laserpointer && _laserImpact)
	{
		_laserpointer->SetVisibility(false);
		_laserImpact->SetVisibility(false);
	}

	if (_tacticalLight)
		UseTacticalLight(false);
}

void AGunBase::InitializeGun()
{
	if (AHellDiver* owner = Cast<AHellDiver>(GetOwner()))
	{
		SetOwner(owner);
		_owner = owner;
		AttachToHand();
		UE_LOG(LogTemp, Log, TEXT("Initialize Gun"));
	}

	DeactivateGun();
}

void AGunBase::ActivateGun()
{
	_isActive = true;
	SetActorHiddenInGame(false);

	_recoilToRecover = FRotator::ZeroRotator;

	if (_laserpointer && _laserImpact)
	{
		_laserpointer->SetVisibility(false);
		_laserImpact->SetVisibility(false);
	}

	_ammoComp->BroadcastAmmoAndSpareChanged(); // TODO) 여기서 해주는 것이 맞는가?
}

void AGunBase::DeactivateGun()
{
	StopFire();
	StopAiming();

	_isActive = false;

	if (_tacticalLight && _owner)
		UseTacticalLight(_owner->GetStateComponent()->IsAiming());

	if (_marker)
		_marker->SetActorHiddenInGame(true);

	if (_crosshair)
		_crosshair->SetVisibility(ESlateVisibility::Hidden);

	if (_laserpointer && _laserImpact)
	{
		_laserpointer->SetVisibility(false);
		_laserImpact->SetVisibility(false);
	}
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

float AGunBase::CalculateDamage(float distance) // distance는 meter 단위
{
	if (distance <= 25.f) // 25m까지
	{
		float alpha = distance / 25.0f;
		float falloff = FMath::Lerp(0.0f, _gunData._falloff25, alpha);
		return _gunData._baseDamage * (1.0f - falloff);
	}
	else if (distance <= 50.f) // 50m까지
	{
		float alpha = (distance - 25.0f) / 25.0f;
		float falloff = FMath::Lerp(_gunData._falloff25, _gunData._falloff50, alpha);
		return _gunData._baseDamage * (1.0f - falloff);
	}
	else if (distance <= 100.f) // 100m까지
	{
		float alpha = (distance - 50.0f) / 50.0f;
		float falloff = FMath::Lerp(_gunData._falloff50, _gunData._falloff100, alpha);
		return _gunData._baseDamage * (1.0f - falloff);
	}
	else
	{
		// 50~100m 구간의 감속 기울기
		float perMeterFalloff = (_gunData._falloff100 - _gunData._falloff50) / 50.0f;

		// 100m 이후부터는 50~100m 구간의 감속 기울기 사용
		float extraFalloff = perMeterFalloff * ((distance - 100.f));
		float finalFalloff = _gunData._falloff100 + extraFalloff;

		return _gunData._baseDamage * (1.0f - finalFalloff);
	}
}

//void AGunBase::TickRecoil(float DeltaTime)
//{
//	if (_owner->GetStateComponent()->IsFiring())
//		return;
//
//	float recoilMultiplier = GetRecoilMultiplier();
//
//	static float RecoilTime = 0.f;
//	RecoilTime += DeltaTime * 2.f * recoilMultiplier;
//
//	// 임시값
//	float amplitudePitch = 0.12f;
//	float amplitudeYaw = 0.04f;
//
//	_recoilOffset.Pitch += FMath::Sin(RecoilTime) * amplitudePitch * recoilMultiplier;
//	_recoilOffset.Yaw += FMath::Cos(RecoilTime / 2) * amplitudeYaw * recoilMultiplier;
//}

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
	if (!_tacticalLight) return;

	if (_gunData._lightModes.Num() <= 1)
		return;

	_lightIndex = (_lightIndex + 1) % _gunData._lightModes.Num();
	_tacticalLightMode = _gunData._lightModes[_lightIndex];

	UseTacticalLight(_owner->GetStateComponent()->IsAiming());
}

void AGunBase::ChangeScopeMode()
{
	if (_gunData._scopeModes.Num() <= 1)
		return;

	_scopeIndex = (_scopeIndex + 1) % _gunData._scopeModes.Num();
	_scopeMode = _gunData._scopeModes[_scopeIndex];
}

void AGunBase::UseLaserPoint(FVector hitPoint)
{
	FVector start;

	if (_gunMesh && _gunMesh->DoesSocketExist(TEXT("LaserPoint")))
	{
		start = _gunMesh->GetSocketLocation(TEXT("LaserPoint"));
	}
	else
	{
		start = GetActorLocation();
	}

	FVector end = hitPoint;

	if (_laserpointer)
	{
		_laserpointer->SetVectorParameter("Beam Start", start);
		_laserpointer->SetVectorParameter("Beam End", end);
	}

	if (_laserImpact)
		_laserImpact->SetWorldLocation(end);
}

void AGunBase::UseTacticalLight(bool isAiming)
{
	if (!_tacticalLight) return;

	switch (_tacticalLightMode)
	{
	case ETacticalLightMode::LightOn:
		_tacticalLight->SetVisibility(true);
		break;

	case ETacticalLightMode::LightOff:
		_tacticalLight->SetVisibility(false);
		break;

	case ETacticalLightMode::LightAuto:
		_tacticalLight->SetVisibility(isAiming);
		break;
	}
}

void AGunBase::PickupItem(AHellDiver* player)
{
	player->PickupGun(this);
}

void AGunBase::PlayFireEffect()
{
	if (_fireEffect)
	{
		_fireEffect->Activate(true);
	}

	if (_shellEjectEffect)
	{
		_shellEjectEffect->Activate(true);
	}
}

void AGunBase::SetGunData(const FGunData& gunData)
{
	_gunData = gunData;
	_fireComp->SetFireModeData(_gunData._fireModes);
	_ammoComp->SetAmmoData(gunData);
}

EFireMode AGunBase::GetCurFireMode()
{
	return _fireComp->GetCurFireMode();
}

int32 AGunBase::GetCurAmmo()
{
	return _ammoComp->GetCurAmmo();
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

