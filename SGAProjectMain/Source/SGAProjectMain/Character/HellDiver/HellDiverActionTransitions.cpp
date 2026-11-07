#include "HellDiver.h"
#include "HellDiverInvenComponent.h"
#include "../CharacterAnimLayerInstance.h"
#include "../../Gun/GunBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/Skeleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"

UAnimMontage* AHellDiver::PlayChangeAnimation(UAnimSequenceBase* Sequence, FName Slot)
{
    UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    USkeleton* Skeleton = GetMesh() && GetMesh()->GetSkeletalMeshAsset() ? GetMesh()->GetSkeletalMeshAsset()->GetSkeleton() : nullptr;
    if (!Anim || !Sequence || !Skeleton || !Sequence->GetSkeleton()
        || Skeleton != Sequence->GetSkeleton() || Sequence->GetPlayLength() <= 0.f
        || !Skeleton->ContainsSlotName(Slot))
    {
        UE_LOG(LogTemp, Warning, TEXT("Action transition rejected: clip=%s skeleton=%s clipSkeleton=%s length=%f slot=%s registered=%d"),
            *GetPathNameSafe(Sequence), *GetPathNameSafe(Skeleton), *GetPathNameSafe(Sequence ? Sequence->GetSkeleton() : nullptr),
            Sequence ? Sequence->GetPlayLength() : 0.f, *Slot.ToString(), Skeleton && Skeleton->ContainsSlotName(Slot));
        return nullptr;
    }
    _changeAnimationHost = Anim;
    // Existing sequence properties also accept authored montage assets.
    if (UAnimMontage* Montage = Cast<UAnimMontage>(Sequence))
    {
        if (!Montage->IsValidSlot(Slot)) return nullptr;
        return Anim->Montage_Play(Montage) > 0.f ? Montage : nullptr;
    }
    return Anim->PlaySlotAnimationAsDynamicMontage(Sequence, Slot, .1f, .1f, 1.f, 1);
}

void AHellDiver::PrepareMotionChange()
{
    if (!_stateComponent->IsMotionChanging()) return;
    _playingMotionChangeId = _stateComponent->GetMotionChangeId();
    _changeAnimationHost = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    GetCharacterMovement()->StopMovementImmediately();
    if (!_changeAnimationHost.IsValid()) _stateComponent->FinishMotionChange(_playingMotionChangeId);
}

UAnimSequenceBase* AHellDiver::GetWeaponChangeAnimation(int32 Index, bool bEquipping) const
{
    return (Index == 1 ? (bEquipping ? _pistolEquipAnimation : _pistolUnequipAnimation)
        : (bEquipping ? _rifleEquipAnimation : _rifleUnequipAnimation)).LoadSynchronous();
}

void AHellDiver::SwitchGun(int32 Index)
{
    if (!_stateComponent->CanStartExclusiveAction() || !_invenComponent->CanSwitchGun(Index)) return;
    if (_stateComponent->GetWeaponState() == EWeaponType::Gun
        && _invenComponent->GetEquippedGun() == _invenComponent->GetGunInSlot(Index)) return;
    // Validate both phases before changing inventory or acquiring a lock.
    if (!GetWeaponChangeAnimation(Index, true)
        || (_stateComponent->GetWeaponState() == EWeaponType::Gun
            && !GetWeaponChangeAnimation(_stateComponent->GetEquipIndex(), false))) return;
    if (!_stateComponent->BeginWeaponChange()) return;
    _pendingSwitchIndex = Index;
    _pendingSwitchGun = _invenComponent->GetGunInSlot(Index);
    _playingWeaponChangeId = _stateComponent->GetWeaponChangeId();
    if (AGunBase* Gun = _invenComponent->GetEquippedGun(); IsValid(Gun))
    {
        Gun->StopFire();
        Gun->StopAiming();
        if (_stateComponent->IsReloading()) Gun->CancelReload();
    }
    _stateComponent->SetFiring(false);
    _stateComponent->SetAiming(false);
    const bool NeedsHolster = _stateComponent->GetWeaponState() == EWeaponType::Gun;
    if (NeedsHolster && PlayWeaponChangePhase(false, _playingWeaponChangeId)) return;
    if (!NeedsHolster && CommitWeaponChange(Index) && PlayWeaponChangePhase(true, _playingWeaponChangeId)) return;
    _stateComponent->FinishWeaponChange(_playingWeaponChangeId);
    _pendingSwitchIndex = INDEX_NONE;
    _pendingSwitchGun.Reset();
}

bool AHellDiver::PlayWeaponChangePhase(bool bEquipping, uint32 RequestId)
{
    const int32 Index = bEquipping ? _pendingSwitchIndex : _stateComponent->GetEquipIndex();
    _weaponChangeMontage = PlayChangeAnimation(GetWeaponChangeAnimation(Index, bEquipping), TEXT("UpperBody"));
    if (!_weaponChangeMontage) return false;
    FOnMontageEnded End;
    End.BindUObject(this, &AHellDiver::OnWeaponChangeEnded, RequestId, bEquipping);
    _changeAnimationHost->Montage_SetEndDelegate(End, _weaponChangeMontage);
    return true;
}

void AHellDiver::OnWeaponChangeEnded(UAnimMontage* Montage, bool bInterrupted, uint32 RequestId, bool bEquipping)
{
    if (RequestId != _playingWeaponChangeId || Montage != _weaponChangeMontage
        || RequestId != _stateComponent->GetWeaponChangeId() || !_stateComponent->IsWeaponChanging()) return;
    _weaponChangeMontage = nullptr;
    if (!bInterrupted && !bEquipping && _stateComponent->IsActionable()
        && CommitWeaponChange(_pendingSwitchIndex) && PlayWeaponChangePhase(true, RequestId)) return;
    _stateComponent->FinishWeaponChange(RequestId);
    _pendingSwitchIndex = INDEX_NONE;
    _pendingSwitchGun.Reset();
}

void AHellDiver::UpdateAnimationChanges()
{
    const bool StanceActive = _stateComponent->IsMotionChanging();
    if (!StanceActive && !_weaponChangeMontage) return;
    UAnimInstance* Current = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
    if (!_stateComponent->IsActionable() || !Current || Current != _changeAnimationHost.Get()
        || (_weaponChangeMontage && (!_stateComponent->IsWeaponChanging()
            || !Current->Montage_IsActive(_weaponChangeMontage))))
        CancelAnimationChanges();
}

void AHellDiver::CancelAnimationChanges()
{
    UAnimInstance* Anim = _changeAnimationHost.Get();
    UAnimMontage* Weapon = _weaponChangeMontage;
    _stateComponent->CancelAnimationChanges();
    _weaponChangeMontage = nullptr;
    _pendingSwitchIndex = INDEX_NONE;
    _pendingSwitchGun.Reset();
    if (Anim && Weapon) Anim->Montage_Stop(.1f, Weapon);
}

void AHellDiver::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CancelAnimationChanges();
    Super::EndPlay(EndPlayReason);
}
