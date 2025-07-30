// Fill out your copyright notice in the Description page of Project Settings.


#include "Corpse.h"
#include "Components/SkeletalMeshComponent.h"   

#include "Physics/PhysicsInterfacePhysX.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/PhysicsConstraintTemplate.h"
#include "PhysicsEngine/ConstraintTypes.h"
// Sets default values
ACorpse::ACorpse()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    _corpseMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CorpseMesh"));
    SetRootComponent(_corpseMesh);

}


// Called when the game starts or when spawned
void ACorpse::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACorpse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACorpse::InitCorpseMesh(USkeletalMeshComponent* meshComp, UMaterialInterface* material)
{
	USkeletalMesh* mesh = meshComp->SkeletalMesh;
    _corpseMesh->SetSkeletalMesh(mesh);
    if (material)
    {
        for (int32 i = 0; i < _corpseMesh->GetNumMaterials(); ++i)
        {
            _corpseMesh->SetMaterial(i, material);
        }
    }


	CopyPose(meshComp);



	// 3) Bone 트랜스폼 즉시 갱신
	_corpseMesh->RefreshBoneTransforms();
	_corpseMesh->UpdateComponentToWorld();


	// 래그돌 기본 세팅
	_corpseMesh->SetSimulatePhysics(true);
	_corpseMesh->SetAllBodiesSimulatePhysics(true);
	_corpseMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	_corpseMesh->RecreatePhysicsState();

	UPhysicsAsset* tempPhysicsAsset = _corpseMesh->GetPhysicsAsset();
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

void ACorpse::CopyPose(USkeletalMeshComponent* meshComp)
{

		TArray<FTransform> LocalTransforms = meshComp->GetBoneSpaceTransforms();
		if (LocalTransforms.IsEmpty()) // during incremental unregistering this might be empty
		{
			return;
		}

		if (_corpseMesh->GetSkinnedAsset() == meshComp->GetSkinnedAsset()
			&& LocalTransforms.Num() == _corpseMesh->BoneSpaceTransforms.Num())
		{

			Exchange(_corpseMesh->BoneSpaceTransforms, LocalTransforms);
		}
		else
		{
			// The meshes don't match, search bone-by-bone (slow path)

			// first set the localatoms to ref pose from our current mesh
			_corpseMesh->BoneSpaceTransforms = _corpseMesh->GetSkinnedAsset()->GetRefSkeleton().GetRefBonePose();

			// Now overwrite any matching bones
			const int32 NumSourceBones = meshComp->GetSkinnedAsset()->GetRefSkeleton().GetNum();

			for (int32 SourceBoneIndex = 0; SourceBoneIndex < NumSourceBones; ++SourceBoneIndex)
			{
				const FName SourceBoneName = meshComp->GetBoneName(SourceBoneIndex);
				const int32 TargetBoneIndex = _corpseMesh->GetBoneIndex(SourceBoneName);

				if (TargetBoneIndex != INDEX_NONE)
				{
					_corpseMesh->BoneSpaceTransforms[TargetBoneIndex] = LocalTransforms[SourceBoneIndex];
				}
			}
		}


	

}

