// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HellDiverDroppodInput.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AHellDiverDroppodInput : public APawn
{
	GENERATED_BODY()

public:
	AHellDiverDroppodInput();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 컨트롤러 포제스/언포제스 시 입력 활성/정리
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// 입력 바인딩
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 드랍포드 참조를 설정합니다(매니저가 드랍포드를 스폰한 직후 호출)
	UFUNCTION(BlueprintCallable, Category = "Game/DropPod/Input")
	void SetHellPodActor(class AHellDiverDropPod* pod);

protected:
	// Enhanced Input 핸들러
	void OnMove2D(const struct FInputActionValue& Value);
	void OnMove2D_Released(const struct FInputActionValue& Value);

protected:
	// 제어 대상 드랍포드
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/DropPod/Input", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<class AHellDiverDropPod> _pod;

	// 입력 데드존(너무 작은 입력 무시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod/Input", meta = (AllowPrivateAccess = "true"))
	float _deadZone = 0.01f;

	// Enhanced Input IMC/IA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod/Input", meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* _IMC_Default = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/DropPod/Input", meta = (AllowPrivateAccess = "true"))
	class UInputAction* _IA_Move = nullptr;
};
