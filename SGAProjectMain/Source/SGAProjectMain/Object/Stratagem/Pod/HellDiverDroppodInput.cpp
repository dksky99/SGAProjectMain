// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverDroppodInput.h"
#include "HellDiverDropPod.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

// Sets default values
AHellDiverDroppodInput::AHellDiverDroppodInput()
{
	PrimaryActorTick.bCanEverTick = true;

	// 입력은 컨트롤러가 Possess한 뒤 SetupPlayerInputComponent에서 바인딩합니다
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

// Called when the game starts or when spawned
void AHellDiverDroppodInput::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHellDiverDroppodInput::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHellDiverDroppodInput::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 포제스 시 입력 활성화 및 IMC 등록
	if (APlayerController* pc = Cast<APlayerController>(NewController))
	{
		// 이 폰에 대한 입력을 활성화
		EnableInput(pc);

		// Enhanced Input 사용 시, 로컬 플레이어 서브시스템에 IMC를 등록해야 액션(IA_Move)이 유효해진다
		if (_IMC_Default)
		{
			if (ULocalPlayer* LP = pc->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					// 우선순위 1로 IMC 등록
					Subsystem->AddMappingContext(_IMC_Default, 1);
				}
			}
		}
	}
}

void AHellDiverDroppodInput::UnPossessed()
{
	Super::UnPossessed();

	// 입력 비활성화 후 자기 정리
	// 드랍포드가 착지 후 헬다이버에게 컨트롤러를 넘기면
	// 이 입력 폰은 더 이상 필요하지 않다
	DisableInput(nullptr);
	Destroy();
}

// Called to bind functionality to input
void AHellDiverDroppodInput::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Enhanced Input 바인딩
	// IA_Move는 Axis2D(Vector2D) 타입의 액션이어야 하며,
	// IMC에서 W/S/A/D, 화살표, 게임패드 왼 스틱 2D축 등을 매핑해 둔다
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 액션이 에셋에 지정되어 있어야 한다
		if (_IA_Move)
		{
			// 키가 눌려 있는 동안 지속적으로 호출된다(값이 변할 때마다 전달)
			EIC->BindAction(_IA_Move, ETriggerEvent::Triggered, this, &AHellDiverDroppodInput::OnMove2D);

			// 입력이 끝났을 때(키를 놓았을 때) 호출되어 0.0으로 정지시킨다
			EIC->BindAction(_IA_Move, ETriggerEvent::Completed, this, &AHellDiverDroppodInput::OnMove2D_Released);
		}
	}
}

void AHellDiverDroppodInput::SetHellPodActor(AHellDiverDropPod* pod)
{
	_pod = pod;
}

void AHellDiverDroppodInput::OnMove2D(const FInputActionValue& Value)
{
	// 드랍포드가 아직 연결되지 않았다면 무시
	if (!_pod.IsValid())
		return;

	// 액션의 2D 입력 값을 꺼낸다
	const FVector2D v = Value.Get<FVector2D>();

	// 데드존 처리(너무 작은 입력은 0.0으로 간주)
	const float axisX = (FMath::Abs(v.X) < _deadZone) ? 0.0f : v.X;
	const float axisY = (FMath::Abs(v.Y) < _deadZone) ? 0.0f : v.Y;

	// 컨트롤러의 Yaw만 추출해서 전/우 방향을 정한다
	FRotator yawOnly = FRotator(0.0f, 0.0f, 0.0f);
	if (AController* c = GetController())
	{
		const FRotator ctrlRot = c->GetControlRotation();
		yawOnly = FRotator(0.0f, ctrlRot.Yaw, 0.0f);
	}

	// 누적/가속 없이 즉시 반응하도록 드랍포드에 전달
	_pod->ApplyHorizontalInput(axisX, axisY, yawOnly);
}

void AHellDiverDroppodInput::OnMove2D_Released(const FInputActionValue& Value)
{
	if (!_pod.IsValid())
		return;

	// 컨트롤러의 Yaw만 추출
	FRotator yawOnly = FRotator(0.0f, 0.0f, 0.0f);
	if (AController* c = GetController())
	{
		const FRotator ctrlRot = c->GetControlRotation();
		yawOnly = FRotator(0.0f, ctrlRot.Yaw, 0.0f);
	}

	// 즉시 정지 입력 전송
	_pod->ApplyHorizontalInput(0.0f, 0.0f, yawOnly);
}