// Fill out your copyright notice in the Description page of Project Settings.


#include "HellDiverDroppodInput.h"

// Sets default values
AHellDiverDroppodInput::AHellDiverDroppodInput()
{
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	PrimaryActorTick.bCanEverTick = true;
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
	PushInputToHellPod();
}

// Called to bind functionality to input
void AHellDiverDroppodInput::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 프로젝트의 입력 이름으로 변경하십시오.
	PlayerInputComponent->BindAxis(TEXT("MoveX"), this, &AHellDiverDroppodInput::OnMoveX);
	PlayerInputComponent->BindAxis(TEXT("MoveY"), this, &AHellDiverDroppodInput::OnMoveY);

}

void AHellDiverDroppodInput::OnMoveX(float axis)
{
	_axisX = (FMath::Abs(axis) < 0.01f) ? 0.0f : axis;
}

void AHellDiverDroppodInput::OnMoveY(float axis)
{
	_axisY = (FMath::Abs(axis) < 0.01f) ? 0.0f : axis;
}

void AHellDiverDroppodInput::PushInputToHellPod()
{
	if (!_hellPodActor.IsValid()) { return; }

	APlayerController* pc = Cast<APlayerController>(GetController());
	if (!pc) { return; }

	// 컨트롤러의 Yaw만 전달합니다.
	const FRotator controlRot = pc->GetControlRotation();
	const FRotator yawOnly = FRotator(0.0f, controlRot.Yaw, 0.0f);

	// 헬포드에 아래 시그니처의 함수를 준비해 둡니다:
	// UFUNCTION(BlueprintCallable) void ApplyHorizontalInput(const float axisX, const float axisY, const FRotator& controlYawOnly);
	UFunction* func = _hellPodActor->FindFunction(TEXT("ApplyHorizontalInput"));
	if (func)
	{
		struct { float X; float Y; FRotator Yaw; } params{ _axisX, _axisY, yawOnly };
		_hellPodActor->ProcessEvent(func, &params);
	}
}

