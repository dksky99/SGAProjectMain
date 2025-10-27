// Fill out your copyright notice in the Description page of Project Settings.


#include "DestructableObject.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "NavAreas/NavArea_Default.h"  // NavArea_Default를 사용하기 위해 포함
#include "../../Navigation/NavArea/NavArea_Destructable.h"


// Sets default values
ADestructableObject::ADestructableObject()
{
    PrimaryActorTick.bCanEverTick = false;
    // 1. 컴포넌트 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f)); // 적절한 크기로 설정 필요

    // 2. 초기 값 설정
    bIsDestroyed = false;
    Health = 100.0f; // 초기 체력

    // C++에서 NavArea_Obstacle 클래스 설정
    // 기본적으로 컴포넌트의 AreaClass는 TSubclassOf<class UNavArea> 타입입니다.
    PreDestructionNavArea = UNavArea_Destructable::StaticClass();

    // 파괴 후에는 기본 통과 가능 영역으로 설정 (NavArea_Default 사용 또는 nullptr)
    PostDestructionNavArea = nullptr; // nullptr은 내비게이션에 영향을 주지 않음을 의미합니다.

    // 3. 충돌 컴포넌트의 초기 NavArea 설정
    CollisionComponent->AreaClass = PreDestructionNavArea;

    // 4. 데미지 이벤트 바인딩
    OnTakeAnyDamage.AddDynamic(this, &ADestructableObject::HandleDamage);

}

void ADestructableObject::HandleDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (bIsDestroyed) return;

    Health -= Damage;

    if (Health <= 0.0f)
    {
        DestroyObstacle();
    }
}
void ADestructableObject::DestroyObstacle()
{
    if (bIsDestroyed) return;
    bIsDestroyed = true;

    // 1. 충돌 및 시각적 변화
    MeshComponent->SetVisibility(false, true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 2. 내비게이션 영역 변경 (핵심)
    CollisionComponent->AreaClass = PostDestructionNavArea;
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 3. 내비게이션 메시 갱신 요청 (대체 코드)

    // UPrimitiveComponent::SetCanEverAffectNavigation(bool bAffect) 함수는 런타임에 내비게이션에 영향을 줄지 여부를 설정합니다.
    // NavArea를 변경하는 것 외에, 이 함수를 호출하여 내비게이션 업데이트를 강제로 트리거할 수 있습니다.
    // 하지만, AreaClass를 변경하는 것만으로 충분할 수도 있습니다.

    // 가장 확실한 방법은 컴포넌트의 내비게이션 상태를 새로 고침하는 것입니다.
    // UBoxComponent는 UPrimitiveComponent를 상속받으며, UPrimitiveComponent는 UNavRelevantInterface를 구현합니다.
    // 다음 함수를 호출하여 내비게이션 시스템에 변경 사항을 알립니다.

    // **방법 A: 컴포넌트 자체에서 Refresh 요청**
    CollisionComponent->UpdateNavigationBounds();

    // **방법 B: 액터의 모든 컴포넌트에서 Refresh 요청 (선택 사항)**
    // 이 함수는 엔진 내부적으로 UNavigationSystemV1::UpdateNavObject와 유사한 작업을 수행하도록 트리거합니다.
    UpdateComponentTransforms();

    // 액터를 파괴하는 경우 (Destroy() 호출)
    // Dynamic/Dynamic Modifiers Only 설정이 되어있다면, 액터가 소멸하면서 남긴 내비게이션 정보가 자동으로 제거됩니다.

}

void ADestructableObject::BeginPlay()
{
    Super::BeginPlay();
    // 초기 NavArea_Obstacle이 확실히 적용되도록 CollisionComponent의 AreaClass를 재설정
    CollisionComponent->AreaClass = PreDestructionNavArea;

    // 내비게이션 메시 업데이트를 위해 Nav Modifier Volume처럼 작용할 수 있도록 컴포넌트 설정
    CollisionComponent->SetCanEverAffectNavigation(true);
}

