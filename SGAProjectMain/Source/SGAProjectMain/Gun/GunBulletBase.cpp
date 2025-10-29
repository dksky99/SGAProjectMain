// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBulletBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "../Data/GunDataTable.h"
#include "../Object/Explosive/ExplosionComponent.h"
#include "../SGAProjectMain.h"

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
    _projectileMovement->InitialSpeed = _projectileData._initialSpeed * 100.f;
    _projectileMovement->MaxSpeed = _projectileData._initialSpeed * 100.f;
    _projectileMovement->bRotationFollowsVelocity = true;
    _projectileMovement->SetUpdatedComponent(_collisionComp);
    _projectileMovement->ProjectileGravityScale = _projectileData._gravityScale * 0.01f;

    // 폭발 컴포넌트
    _explosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComponent"));
}

// Called when the game starts or when spawned
void AGunBulletBase::BeginPlay()
{
	Super::BeginPlay();

    _collisionComp->OnComponentBeginOverlap.AddDynamic(this, &AGunBulletBase::OnBulletOverlap);
    _collisionComp->OnComponentHit.AddDynamic(this, &AGunBulletBase::OnBulletHit);

    if (GetOwner())
        _collisionComp->IgnoreActorWhenMoving(GetOwner(), true);
    if (GetInstigator())
        _collisionComp->IgnoreActorWhenMoving(GetInstigator(), true);

    _prevLoc = GetActorLocation();
    _baseSpeed = _projectileMovement->InitialSpeed;
}

// Called every frame
void AGunBulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    FVector curLoc = GetActorLocation();
    _moveDistance += FVector::Dist(curLoc, _prevLoc);
    _prevLoc = curLoc;
    
    // 기본 속도에서 감속량 적용   
	float speedFalloffMultiplier = CalculateSpeedFalloffMultiplier(_moveDistance / 100.f);
	float targetSpeed = _baseSpeed * speedFalloffMultiplier;
	float curSpeed = _projectileMovement->Velocity.Size();
    UE_LOG(LogTemp, Log, TEXT("CurSpeed: %f"), curSpeed);

	if (curSpeed < 10.f)    // 속도가 거의 0에 도달했으면 파괴
    {
        if (_projectileData._type == EGunProjectileType::Explosive && !_isExploded)
        {
            _isExploded = true;
			Explode();
        }

        Destroy();
        return;
    }
    
    FVector newVelocity = _projectileMovement->Velocity.GetSafeNormal() * targetSpeed;
    _projectileMovement->Velocity = newVelocity;
    _projectileMovement->MaxSpeed = targetSpeed;
}

void AGunBulletBase::OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (_isExploded) // 폭발했다면 OnHit 발생 안함
        return;

    if (OtherActor && OtherActor != this)
    {
        if (_hitComponents.Contains(OtherComp))
            return; // 같은 부위를 두 번 공격하지 않음

        if (OtherComp->GetCollisionProfileName() != FName(TEXT("HitBox")))
            return; // 히트박스 콜리전만 공격

        float finalDamage = _projectileData._baseDamage * (_projectileMovement->Velocity.Size() / _baseSpeed); // 속도에 비례하는 최종 데미지
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

        FVector newVelocity = _projectileMovement->Velocity * (1.f - _projectileData._falloffPenetration);
        _projectileMovement->Velocity = newVelocity;
        _projectileMovement->MaxSpeed = newVelocity.Size();

        if (_projectileData._type == EGunProjectileType::Explosive)
        {
            _isExploded = true;
            Explode();
            Destroy();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Bullet Hit!"));
}

void AGunBulletBase::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherComp || _isExploded) return;

    _projectileMovement->StopMovementImmediately();
    
    if (_projectileData._type == EGunProjectileType::Explosive)
    {
        _isExploded = true;
        Explode();
    }

    Destroy();
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

float AGunBulletBase::CalculateSpeedFalloffMultiplier(float distance)
{
    float falloff = 0.0f;

    if (distance <= 25.f) // 25m까지
    {
        float alpha = distance / 25.0f;
        falloff = FMath::Lerp(0.0f, _projectileData._falloff25, alpha);
    }
    else if (distance <= 50.f) // 50m까지
    {
        float alpha = (distance - 25.0f) / 25.0f;
        falloff = FMath::Lerp(_projectileData._falloff25, _projectileData._falloff50, alpha);
    }
    else if (distance <= 100.f) // 100m까지
    {
        float alpha = (distance - 50.0f) / 50.0f;
        falloff = FMath::Lerp(_projectileData._falloff50, _projectileData._falloff100, alpha);
    }
    else
    {
        // 50~100m 구간의 감속 기울기
        float perMeterFalloff = (_projectileData._falloff100 - _projectileData._falloff50) / 50.0f;

        // 100m 이후부터는 50~100m 구간의 감속 기울기 사용
        float extraFalloff = perMeterFalloff * ((distance - 100.f));
        falloff = _projectileData._falloff100 + extraFalloff;
    }

    falloff = FMath::Clamp(falloff, 0.f, 1.f);
    return 1.0f - falloff;
}

void AGunBulletBase::InitializeProjectile(FGunProjectileData data)
{
    _projectileData = data;
    _projectileMovement->InitialSpeed = data._initialSpeed * 100.f;
    _projectileMovement->MaxSpeed = data._initialSpeed * 100.f;
    _baseSpeed = _projectileMovement->InitialSpeed;
    _projectileMovement->ProjectileGravityScale = data._gravityScale * 0.01f;
    _prevLoc = GetActorLocation();
}

