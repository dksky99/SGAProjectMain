// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverDropPod.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "../../../Character/PlayerCharacter.h"

AHellDiverDropPod::AHellDiverDropPod()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스프링암 생성 및 루트에 부착
	_springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	_springArm->SetupAttachment(RootComponent);
	_springArm->TargetArmLength = 500.0f;            // 기본 거리(후에 BP에서 조정)
	_springArm->bUsePawnControlRotation = false;      // 컨트롤러 회전 비사용
	_springArm->bDoCollisionTest = false;             // 카메라 충돌테스트 끔(연출용)
	_springArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); // 기본 내려보기 각도

	// 카메라 생성 및 스프링암 소켓에 부착
	_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	_camera->SetupAttachment(_springArm, USpringArmComponent::SocketName);
	_camera->bUsePawnControlRotation = false;  // 컨트롤러 회전 비사용
	_camera->bAutoActivate = true;             // 이 액터를 ViewTarget으로 주면 즉시 활성
}

void AHellDiverDropPod::BeginPlay()
{
	Super::BeginPlay();

	_desiredVelXY = FVector::ZeroVector;
	_raiseElapsed = 0.0f;
}

void AHellDiverDropPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 공중일 때는 입력 기반 수평 속도 갱신
	if (!_isGrounded)
	{
		UpdateVelocityImmediate();
		return;
	}

	// 착지 후에는 플레이어 상승 연출(지하→지상)
	if (_player && _raiseElapsed < _raiseDuration)
	{
		UpdateRaisePlayer(DeltaTime);
	}
}

void AHellDiverDropPod::SpawnInternalActor(const FVector& SpawnLocation)
{
	if (!_player) return;            // 매니저에서 SetPreSpawnedPlayer로 주입 필수

	// 지면 아래 위치부터 시작(끼임 방지 위해 충돌 Off)
	const FVector startBelow = SpawnLocation - FVector(0, 0, _spawnDepth);

	_player->ResetUnit();
	_player->SetActorLocation(startBelow, false, nullptr, ETeleportType::TeleportPhysics);

	if (UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(_player->GetRootComponent()))
	{
		rootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}


	_raiseElapsed = 0.0f;
}

void AHellDiverDropPod::UpdateRaisePlayer(const float dt)
{
	if (!_player) return;

	_raiseElapsed += dt;
	const float t = FMath::Clamp(_raiseElapsed / _raiseDuration, 0.0f, 1.0f);

	const FVector landing = GetActorLocation();
	const FVector start = landing - FVector(0, 0, _spawnDepth);
	const FVector target = landing + FVector(0, 0, 2.0f); // 살짝 지상 위
	const FVector pos = FMath::Lerp(start, target, t);

	_player->SetActorLocation(pos, false, nullptr, ETeleportType::TeleportPhysics);

	if (t >= 1.0f)
	{
		FinishRaiseAndPossess();
	}
}

void AHellDiverDropPod::FinishRaiseAndPossess()
{
	if (!_player)
		return;

	// 충돌 활성화
	if (UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(_player->GetRootComponent()))
	{
		rootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// 컨트롤러 인계: Owner 우선(매니저가 드랍포드 Owner를 컨트롤러로 설정하여 스폰)
	if (AController* c = Cast<AController>(GetOwner()))
	{
		c->Possess(_player);
	}
	else if (AController* c2 = GetInstigatorController())
	{
		c2->Possess(_player);
	}

	// 플레이어 카메라를 TPS로 확정 전환
	if (APlayerCharacter* player = Cast<APlayerCharacter>(_player))
	{
		if (APlayerController* playerController = Cast<APlayerController>(player->GetController()))
		{
			SmoothBlendToPlayerCamera(playerController);
		}
	}

	// 보간 종료 마커 및 비용 절감
	_raiseElapsed = _raiseDuration;
	SetActorTickEnabled(false);
}

void AHellDiverDropPod::ApplyHorizontalInput(const float axisX, const float axisY, const FRotator& controlYawOnly)
{
	// 착지 후에는 수평 속도 제거
	if (_isGrounded)
	{
		_desiredVelXY = FVector::ZeroVector;
		return;
	}

	// 컨트롤러 Yaw 기준 전/우 벡터
	const FVector forward = FRotationMatrix(controlYawOnly).GetUnitAxis(EAxis::X);
	const FVector right = FRotationMatrix(controlYawOnly).GetUnitAxis(EAxis::Y);

	// 누적/가속 없이 즉시 반응: 입력 벡터를 정규화하여 최대 속도 곱
	const FVector dir = forward * axisY + right * axisX;

	// 수평 성분만 유지하고 1.0f로 캡
	const FVector horizontal = FVector(dir.X, dir.Y, 0.0f).GetClampedToMaxSize(1.0f);

	_desiredVelXY = horizontal * _maxHorizontalSpeed;
}

void AHellDiverDropPod::ActivateOverlookCamera(AController* controller, const float blendTime)
{
	// 드랍포드에 BP 카메라가 존재한다면 이 액터를 ViewTarget으로 전환한다
		// BP에서 _cameraBoom/_overlookCamera를 추가하지 않았다면 그냥 넘어간다
	if (APlayerController* pc = Cast<APlayerController>(controller))
	{
		if (_camera != nullptr || _springArm!= nullptr)
		{
			// 드랍포드를 ViewTarget으로 주면 BP 카메라를 통해 보게 된다
			pc->SetViewTargetWithBlend(this, blendTime);
		}
	}
}

void AHellDiverDropPod::UpdateVelocityImmediate()
{
	if (!_projectile)
		return;

	// 안전 가드: 착지 후에는 중력/가속 주입 금지
	if (_isGrounded)
	{
		_projectile->StopMovementImmediately();
		return;
	}

	const FVector newVel(_desiredVelXY.X, _desiredVelXY.Y, -_fallSpeed);
	_projectile->Velocity = newVel;
}

void AHellDiverDropPod::SmoothBlendToPlayerCamera(APlayerController* PC)
{
	if (!PC || !_player) return;

	// 블렌드 파라미터
	const float  BlendTime = 0.85f;
	const EViewTargetBlendFunction BlendFunc = VTBlend_EaseInOut;
	const float  BlendExp = 2.0f;

	// 플레이어가 자체 로직으로 TPS 카메라 활성화
	_player->SetTPSView();

	// 플레이어의 현재 카메라 ChildActor로 '우리가 원하는 값'으로 재블렌드(덮어쓰기)
	if (UChildActorComponent* CurCam = _player->GetCurCamera())
	{
		if (AActor* CamActor = CurCam->GetChildActor())
		{
			PC->SetViewTargetWithBlend(CamActor, BlendTime, BlendFunc, BlendExp);
		}
	}
}
