// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructFieldActor.h"

#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemTypes.h"

void ADestructFieldActor::ActivateField(const FVector& Position, float Radius, float Strength)
{

    // 스칼라 데미지 필드
    URadialFalloff* DamageFalloff = NewObject<URadialFalloff>(GetTransientPackage());
    DamageFalloff->Magnitude = Strength; // Strength를 직접 Magnitude로 사용
    DamageFalloff->MinRange = 0.f;
    DamageFalloff->MaxRange = Radius;
    DamageFalloff->Default = 0.f;
    DamageFalloff->Radius = Radius;
    DamageFalloff->Position = Position;
    DamageFalloff->Falloff = EFieldFalloffType::Field_Falloff_Linear; // 선형 감쇠
    
    if (FieldSystemComponent)
    {
		// 필드 시스템 컴포넌트에 물리 필드 적용
        FieldSystemComponent->ApplyPhysicsField(
            true, // 활성화
			EFieldPhysicsType::Field_ExternalClusterStrain, // 스칼라 값을 GC의 파괴강도에 적용
			nullptr, // 필터 없음
            DamageFalloff // 이 스칼라 필드의 Magnitude가 DynamicState의 값을 결정 (높을수록 강하게 적용)
        );
    }
}
