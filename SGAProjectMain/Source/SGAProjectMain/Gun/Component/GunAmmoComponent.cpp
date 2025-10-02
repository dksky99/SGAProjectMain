// Fill out your copyright notice in the Description page of Project Settings.


#include "GunAmmoComponent.h"

#include "../GunBase.h"

#include "../../Character/CharacterAnimInstance.h"

// Sets default values for this component's properties
UGunAmmoComponent::UGunAmmoComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UGunAmmoComponent::SetAmmoData(const FGunData& gunData)
{
	_reloadType = gunData._reloadType;
	_needAmmoBag = gunData._needAmmoBag;

	_maxAmmo = gunData._maxAmmo;
	_curAmmo = gunData._maxAmmo;

	_maxSpare = gunData._maxSpare;
	_curSpare = gunData._initialSpare;
	_refillSpareAmount = gunData._refillAmount;

	_curAmmo--;
	_isChamberLoaded = true; // 미리 약실 채움
}


// Called when the game starts
void UGunAmmoComponent::BeginPlay()
{
	Super::BeginPlay();

	_gun = Cast<AGunBase>(GetOwner()); // TODO) Comp가 gun을 들고 있어야 하는가?

	// ...
	
}


// Called every frame
void UGunAmmoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UGunAmmoComponent::CanFire()
{
	// 약실이 채워져 있으면 발사 가능
	return (_isChamberLoaded);
}

void UGunAmmoComponent::ConsumeAmmo()
{
	_isChamberLoaded = false;	// 약실 탄 비움
	
	if (_curAmmo > 0) // 탄창에 탄약이 남아있을 경우
	{
		_curAmmo--; // 탄약 감소하고
		_isChamberLoaded = true; // 약실에 탄 채움
	}

	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo + _isChamberLoaded, _maxAmmo);
}

bool UGunAmmoComponent::CanReload()
{
	// 소유 탄약이 최대치 이상이면 무조건 안 됨
	if (_curAmmo + _isChamberLoaded >= _maxAmmo)
		return false;

	if (_needAmmoBag)	// 이 경우 배낭이 스패어 역할
	{
		if (_gun->GetOwnerCharacter()->CanReloadUsingBackpack())
			return true;
	}

	// 스패어가 남아있으면 장전 가능
	return (_curSpare != 0);
}

void UGunAmmoComponent::Reload()
{
	if (_reloadStage != EReloadStage::CloseBolt) // 이 경우 약실만 채우면 되므로 검증이 필요없음
	{
		if (!CanReload())
		{
			FinishReload();
			return;
		}
	}

	AHellDiver* owner = _gun->GetOwnerCharacter();

	if (!_reloadMontage)
	{
		FinishReload();
		return;
	}

	if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(owner->GetMesh()->GetAnimInstance()))
	{
		owner->GetStateComponent()->StartReload();
		animInstance->PlayAnimMontage(_reloadMontage);

		// 재생 후 인스턴스 가져오기
		if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(_reloadMontage))
		{
			// 델리게이트 중복 방지
			MontageInstance->OnMontageEnded.Unbind();

			// 델리게이트 바인딩
			MontageInstance->OnMontageEnded.BindUObject(this, &UGunAmmoComponent::OnReloadMontageEnded);

			UE_LOG(LogTemp, Error, TEXT("Success to get MontageInstance for %s"), *_reloadMontage->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *_reloadMontage->GetName());
		}

		int32 sectionIndex;

		if (_reloadType == EReloadType::Magazine)
		{
			if (_reloadStage == EReloadStage::Idle)
				_reloadStage = EReloadStage::RemoveMag;

			sectionIndex = PickReloadSection();		
		}
		else
			sectionIndex = 2;

		animInstance->JumpToSection(sectionIndex);

	}
}

void UGunAmmoComponent::OnReloadSectionEnded()	// 몽타주 섹션 종료 시 호출
{
	if (_reloadType == EReloadType::Magazine)
	{
		ChangeReloadStage();
	}
	else // 탄약 재장전일 경우
	{
		if (_needAmmoBag)
			_gun->GetOwnerCharacter()->UseBackpack(1);
		else
			_curSpare--;

		if (_isChamberLoaded)	// 약실에 탄이 채워져있다면
			_curAmmo++;			// 탄약 증가
		else
			_isChamberLoaded = true;	// 아니라면 약실 채움

		FinishReload();
		Reload();
	}

	BroadcastAmmoAndSpareChanged();
}

int32 UGunAmmoComponent::PickReloadSection()
{
	switch (_reloadStage)
	{
	case EReloadStage::Idle:					// 기본 상태라면
		_reloadStage = EReloadStage::RemoveMag; // 탄창 제거 시작 단계로
	case EReloadStage::RemoveMag:				// 의도된 fallthrough
		UE_LOG(LogTemp, Log, TEXT("RemoveMag"));
		return 0;						// 탄창 제거 애니메이션

	case EReloadStage::InsertMag:
		UE_LOG(LogTemp, Log, TEXT("InsertMag"));
		return 1;						// 탄창 삽입 애니메이션

	case EReloadStage::CloseBolt:
		UE_LOG(LogTemp, Log, TEXT("CloseBolt"));
		return 2;						// 약실 폐쇄 애니메이션
	}

	return -1;
}

void UGunAmmoComponent::ChangeReloadStage()		// 몽타주 섹션 종료 시 호출
{
	switch (_reloadStage)
	{
	case EReloadStage::RemoveMag:				// 탄창 제거가 끝나면
		_curAmmo = 0;							// 탄창에 남은 총알 비워짐
		_reloadStage = EReloadStage::InsertMag;	// 탄창 삽입 단계로 
		Reload();
		break;

	case EReloadStage::InsertMag:				// 탄창 삽입이 끝나면
		if (_needAmmoBag)
			_gun->GetOwnerCharacter()->UseBackpack(1);
		else
			_curSpare--;						// 여분 탄창 감소				
		_curAmmo = _maxAmmo;					// 탄약 전체 충전
		if (_isChamberLoaded)					// 약실에 탄이 있다면
		{
			_reloadStage = EReloadStage::Idle;	// 전술 재장전 -> CloseBolt 생략
		}
		else									// 약실에 탄이 없다면
		{
			_reloadStage = EReloadStage::CloseBolt; // 약실 폐쇄 단계로
			Reload();
		}
		break;

	case EReloadStage::CloseBolt:				// 약실 폐쇄가 끝나면
		_curAmmo--;
		_isChamberLoaded = true;				// 약실에 탄이 채워짐
		_reloadStage = EReloadStage::Idle;
		break;

	case EReloadStage::Idle:
		FinishReload();
		break;
	}
}

void UGunAmmoComponent::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	FinishReload();
}

void UGunAmmoComponent::CancelReload()
{
	if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(_gun->GetOwnerCharacter()->GetMesh()->GetAnimInstance()))
		animInstance->Montage_Stop(0.2f, _reloadMontage);

	FinishReload();
}

void UGunAmmoComponent::FinishReload()
{
	AHellDiver* owner = _gun->GetOwnerCharacter();

	if (!owner->GetStateComponent()->IsReloading())
		return;

	owner->GetStateComponent()->FinishReload();
}

void UGunAmmoComponent::RefillSpare()
{
	_curSpare += _refillSpareAmount;

	if (_curSpare > _maxSpare)
		_curSpare = _maxSpare;

	if (_spareChanged.IsBound())
		_spareChanged.Broadcast(_curSpare, _maxSpare);
}

void UGunAmmoComponent::BroadcastAmmoAndSpareChanged()
{
	if (_ammoChanged.IsBound())
		_ammoChanged.Broadcast(_curAmmo + _isChamberLoaded, _maxAmmo);
	

	if (_spareChanged.IsBound())
		_spareChanged.Broadcast(_curSpare, _maxSpare);
}

