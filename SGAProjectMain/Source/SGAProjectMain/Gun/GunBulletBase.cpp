// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBulletBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../Object/Explosive/ExplosionComponent.h"

// Sets default values
AGunBulletBase::AGunBulletBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    _collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    _collisionComp->SetNotifyRigidBodyCollision(true); // OnHit 이벤트에 필요
    _collisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    _collisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
    RootComponent = _collisionComp;

    // 이동 컴포넌트
    _projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    _projectileMovement->InitialSpeed = _bulletData._initialSpeed;
    _projectileMovement->MaxSpeed = _bulletData._initialSpeed;
    _projectileMovement->bRotationFollowsVelocity = true;
    _projectileMovement->SetUpdatedComponent(_collisionComp);
    //_projectileMovement->ProjectileGravityScale = 0.f;

    // 폭발 컴포넌트
    _explosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
}

// Called when the game starts or when spawned
void AGunBulletBase::BeginPlay()
{
	Super::BeginPlay();

    _collisionComp->OnComponentBeginOverlap.AddDynamic(this, &AGunBulletBase::OnBulletOverlap);

    if (GetOwner())
        _collisionComp->IgnoreActorWhenMoving(GetOwner(), true);
    if (GetInstigator())
        _collisionComp->IgnoreActorWhenMoving(GetInstigator(), true);

    _baseSpeed = _bulletData._initialSpeed;


}

// Called every frame
void AGunBulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    _moveDistance += _projectileMovement->Velocity.Size() * DeltaTime;
    float speedLoss = _bulletData._initialSpeed * GetSpeedMultiplier(_moveDistance); // 이동 거리 기반 감속된 속도

    // 기본 속도에서 감속량만큼 빼기
    float _curSpeed = _baseSpeed - speedLoss * DeltaTime;

    if (_curSpeed <= 0.1f) // 예: 속도 10 이하라면 제거
    {
        if (_bulletData._type == EBulletType::Standard)
            Destroy();
    }
    else
    {
        FVector newVelocity = _projectileMovement->Velocity.GetSafeNormal() * _curSpeed;
        _projectileMovement->Velocity = newVelocity;
    }
}

void AGunBulletBase::OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (_isExploded) // 폭발했다면 OnHit 발생 안함
        return;

    if (OtherActor && OtherActor != this)
    {
        if (_hitComponents.Contains(OtherComp))
            return; // 같은 부위를 두 번 공격하지 않음

        float finalDamage = _bulletData._baseDamage * (_projectileMovement->Velocity.Size() / _bulletData._initialSpeed); // 속도에 비례하는 최종 데미지
        FVector shotDirection = _projectileMovement->Velocity.GetSafeNormal(); // 데미지 방향

        UGameplayStatics::ApplyPointDamage(
            OtherActor,                     // 데미지를 받을 액터
            finalDamage,                    // 적용할 기본 데미지 값
            shotDirection,                  // 데미지가 들어온 방향 벡터
            SweepResult,                    // 충돌 정보(FHitResult)
            GetInstigatorController(),      // 데미지를 유발한 컨트롤러
            this,                           // 데미지 발생 주체 액터
            UDamageType::StaticClass()      // 사용할 데미지 타입 클래스
        );

        UE_LOG(LogTemp, Warning, TEXT("DamageAmount: %f"), finalDamage);

        _hitComponents.Add(OtherComp); // 공격한 부위 저장 -> 중복 방지
        _baseSpeed *= 0.75f;

        if (_bulletData._type == EBulletType::Explosive)
        {
            _isExploded = true;
            Explode();
            Destroy();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Bullet Hit!"));
}

void AGunBulletBase::Explode()
{
    _explosionComponent->Explode();

    //UGameplayStatics::ApplyRadialDamageWithFalloff(
    //    this,
    //    _bulletData._explosionDamage,   // 중심 폭발 피해
    //    0.f,                            // 최소 피해
    //    GetActorLocation(),             // 폭발 위치
    //    _bulletData._innerRadius,       // 최대 피해 거리
    //    _bulletData._outerRadius,       // 최소 피해 거리
    //    1.0f,                           // 거리에 비례하여 피해 감소
    //    UDamageType::StaticClass(),     
    //    TArray<AActor*>(),              // 무시 액터 목록
    //    this,
    //    GetInstigatorController(),
    //    ECC_Visibility
    //);

    //// 디버깅용
    //FColor drawColor = FColor::Red;
    //DrawDebugSphere(GetWorld(), GetActorLocation(), _bulletData._innerRadius, 10, drawColor, false, 1.0f);
    //DrawDebugSphere(GetWorld(), GetActorLocation(), _bulletData._outerRadius, 10, drawColor, false, 1.0f);
}

float AGunBulletBase::GetSpeedMultiplier(float distance)
{
    if (distance <= 2500.f) // 25m까지
        return FMath::Lerp(0.0f, _bulletData._falloff25, distance / 25.0f);
    else if (distance <= 5000.f) // 50m까지
        return FMath::Lerp(_bulletData._falloff25, _bulletData._falloff50, (distance - 25.0f) / 25.0f);
    else if (distance <= 10000.f) // 100m까지
        return FMath::Lerp(_bulletData._falloff50, _bulletData._falloff100, (distance - 50.0f) / 50.0f);
    else
    {
        // 100m 이후부터는 50~100m 구간의 감속 기울기 사용
        float perMeterFalloff = (_bulletData._falloff100 - _bulletData._falloff50) / 50.0f;
        return perMeterFalloff * (distance - 10000.f);
    }
}

void AGunBulletBase::InitializeProjectile()
{

}

