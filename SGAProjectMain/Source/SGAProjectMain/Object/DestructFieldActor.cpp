// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructFieldActor.h"

#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemTypes.h"

void ADestructFieldActor::ActivateField(const FVector& Position, float Radius, float Strength)
{
//    URadialFalloff* Falloff = NewObject<URadialFalloff>();
//    Falloff->Magnitude = 1.0f;
//    Falloff->MinRange = 0.f;
//    Falloff->MaxRange = Radius;
//    Falloff->Default = 0.f;
//    Falloff->Radius = Radius;
//    Falloff->Position = Position;
//    Falloff->Falloff = EFieldFalloffType::Field_Falloff_Linear;
//
//    URadialVector* Vector = NewObject<URadialVector>();
//    Vector->Magnitude = Strength;
//    Vector->Position = Position;
//
//    UOperatorField* Operator = NewObject<UOperatorField>();
//    Operator->LeftField = Falloff;
//    Operator->RightField = Vector;
//    Operator->Operation = EFieldOperationType::Field_Multiply;
//
//    if (FieldSystemComponent)
//    {
//        FieldSystemComponent->ApplyPhysicsField(
//            true,
//            EFieldPhysicsType::Field_ExternalClusterStrain,
//            nullptr,
//            Operator
//        );
//    }
    // 스칼라 데미지 필드 (RadialFalloff를 Magnitude로 사용)
    URadialFalloff* DamageFalloff = NewObject<URadialFalloff>();
    DamageFalloff->Magnitude = Strength; // Strength를 직접 Magnitude로 사용
    DamageFalloff->MinRange = 0.f;
    DamageFalloff->MaxRange = Radius;
    DamageFalloff->Default = 0.f;
    DamageFalloff->Radius = Radius; // Falloff의 Radius와 동일하게 설정
    DamageFalloff->Position = Position;
    DamageFalloff->Falloff = EFieldFalloffType::Field_Falloff_Linear; // 선형 감쇠
    
    // 필드를 적용할 Geometry Collection 컴포넌트 (선택 사항: 특정 컴포넌트에만 적용하고 싶다면)
    // UGeometryCollectionComponent* TargetGCComponent = ...; // 여기에 대상 컴포넌트를 가져오는 로직 추가
    
    if (FieldSystemComponent)
        {
        // Field_DynamicState를 사용하여 해당 영역의 조각들을 동적 상태로 만들고, 파괴를 유도
        // 이는 직접적인 '데미지' 필드라기보다는 물리적 상태를 변경하여 파괴가 가능하게 함
        FieldSystemComponent->ApplyPhysicsField(
            true, // Enable
            EFieldPhysicsType::Field_DynamicState, // 동적 상태 필드
            nullptr, // 특정 컴포넌트 타겟팅: 모든 Geometry Collection에 영향을 주려면 nullptr 유지, 아니면 TargetGCComponent
            DamageFalloff // 이 스칼라 필드의 Magnitude가 DynamicState의 값을 결정 (높을수록 강하게 적용)
        );
    }
}
