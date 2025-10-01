// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplosiveGun.h"

#include "../Character/HellDiver/HellDiver.h"
#include "../Character/HellDiver/HellDiverAnimInstance.h"

//void AExplosiveGun::Reload()
//{
//	_owner->GetStateComponent()->SetReloading(true);
//
//	if (!_reloadMontage) return;
//
//	if (UCharacterAnimInstance* animInstance = Cast<UCharacterAnimInstance>(_owner->GetMesh()->GetAnimInstance()))
//	{
//		_owner->GetStateComponent()->StartReload();
//		animInstance->PlayAnimMontage(_reloadMontage);
//
//		// 재생 후 인스턴스 가져오기
//		if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(_reloadMontage))
//		{
//			// 델리게이트 중복 방지
//			MontageInstance->OnMontageEnded.Unbind();
//
//			// 델리게이트 바인딩
//			MontageInstance->OnMontageEnded.BindUObject(this, &AGunBase::FinishReload);
//
//			UE_LOG(LogTemp, Error, TEXT("Success to get MontageInstance for %s"), *_reloadMontage->GetName());
//		}
//		else
//		{
//			UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *_reloadMontage->GetName());
//		}
//
//		int32 sectionIndex = -1;
//		switch (_reloadStage)
//		{
//		case EReloadStage::None:
//			sectionIndex = 0;
//			break;
//
//		case EReloadStage::RemoveMag:
//			sectionIndex = 1;
//			break;
//
//		case EReloadStage::InsertMag:
//			sectionIndex = 2;
//			break;
//
//		case EReloadStage::CloseBolt:
//			sectionIndex = 3;
//			break;
//
//		case EReloadStage::RoundsReload:
//			sectionIndex = 3;
//			break;
//
//		default:
//			return;
//		}
//
//		if (sectionIndex >= 0)
//		{
//			animInstance->JumpToSection(sectionIndex);
//		}
//	}
//}
//
//void AExplosiveGun::ChangeReloadStage()
//{
//	_ammoComp->ChangeReloadStage();
//	switch (_reloadStage)
//	{
//	case EReloadStage::None:
//		_reloadStage = EReloadStage::RemoveMag;
//		_curAmmo = 0;
//		UE_LOG(LogTemp, Log, TEXT("None->RemoveMag"));
//		Reload();
//		break;
//
//	case EReloadStage::RemoveMag: // 탄창 제거 상태
//		_reloadStage = EReloadStage::InsertMag;
//		Reload();
//		UE_LOG(LogTemp, Log, TEXT("RemoveMag->InsertMag"));
//		break;
//
//	case EReloadStage::InsertMag: // 탄창 삽입 상태
//			_reloadStage = EReloadStage::CloseBolt;
//			Reload();
//			UE_LOG(LogTemp, Log, TEXT("InsertMag->CloseBolt"));
//		break;
//
//	case EReloadStage::CloseBolt:
//		_curAmmo = 1; // 한 발만 쏠 수 있음
//		_owner->GetStateComponent()->SetReloading(false);
//		_reloadStage = EReloadStage::None;
//		UE_LOG(LogTemp, Log, TEXT("CloseBolt->None"));
//		break;
//
//	case EReloadStage::RoundsReload:
//		_curAmmo++;
//		_curMag--;
//		_owner->GetStateComponent()->SetReloading(false);
//		Reload();
//		UE_LOG(LogTemp, Log, TEXT("RoundsReload"));
//		break;
//	}
//
//	if (_ammoChanged.IsBound())
//	{
//		_ammoChanged.Broadcast(_curAmmo, _gunData._maxAmmo);
//	}
//
//	if (_magChanged.IsBound())
//		_magChanged.Broadcast(_curMag, _gunData._maxMag);
//}
