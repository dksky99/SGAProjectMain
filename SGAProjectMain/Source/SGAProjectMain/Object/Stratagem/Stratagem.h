// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Throwable.h"
#include "Stratagem.generated.h"

// 스트라타젬 종류
UENUM(BlueprintType)
enum class EStratagemType : uint8
{
	Offensive	UMETA(DisplayName = "Offensive"),
	Defensive	UMETA(DisplayName = "Defensive"),
	Supply		UMETA(DisplayName = "Supply")
};

UCLASS()
class SGAPROJECTMAIN_API AStratagem : public AThrowable
{
	GENERATED_BODY()

public:
	AStratagem();

	virtual void Tick(float DeltaTime) override;

	// 스트라타젬 배치 후 실제 기능 수행 (폭격, 드랍포드 등)
	void DeployStratagem();

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	// 입력 시퀀스 (UI용)
	UFUNCTION(BlueprintPure)
	const TArray<FKey>& GetInputSequence() const { return _inputSequence; }

	// UI용ETA(남은 시간)
	float GetImpactRemain() const { return _impactRemain; }

	// ETA 계산이 끝났는지 여부
	bool IsEtaReady() const { return _isEtaReady; }

	// 외부(플레이어)에서 호출하는 UI 표시/숨김 함수
	void ShowEtaAtScreenPosition(const FVector2D& screenPosition, int32 etaSec, class APlayerController* ownerPC);
	void HideEta();

	const FName GetStgName() const { return _name; }
	class UTexture2D* GetStgIcon() const { return _icon; }
	EStratagemType GetStgType() const { return _type; }
	float GetDeployDelay() const { return _deployDelay; }


protected:
	// 특정 표면에 부착 가능한지 확인
	bool IsSurfaceAttachable(const FHitResult& Hit);

	// 비공격형: 전체 수명 계산 (부착 → 투하 딜레이 → 낙하)
	float ComputeNonAttackTotalLifetime(const FVector& targetLocation) const;

	// 비공격형: 드랍포드 실제 스폰 + 낙하 시작
	void SpawnNonAttackDropPod(const FVector& targetLocation, const FActorSpawnParameters& sp);

	// 공격형: 폭격 컨트롤러 스폰 + 타이밍 설정
	void SetupOffensiveStratagem(const FVector& targetLocation, const FActorSpawnParameters& sp);
	
	virtual void DestroySelf() override;

protected:
	// 실제 폭격/센트리/드랍포드 등
	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	TSubclassOf<AActor> _objectToSpawn;

	// ETA 위젯 클래스 (스크린 공간)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	TSubclassOf<class UStratagemEtaWidget> _etaWidgetClass;

	// 투척 후 몇 초 뒤에 동작 시작 (폭격 시작 지연, 드랍포드 투하 지연 등)
	UPROPERTY(EditAnywhere, Category = "Game/Stratagem")
	float _deployDelay = 5.0f;

	// 도착까지 남은 시간 (로그/UI용 카운트다운)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float _impactRemain = 0.0f;

	// 투하 시작 높이(Z)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float _dropHeight = 1500.0f;

	// 투하 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float _dropSpeed = 8000.0f;

	// 공격형 전용: 폭격 구간(첫 탄 이후 → 마지막 탄까지) 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	float _bombardRemain = 0.0f;

	// 공격형 여부
	UPROPERTY(EditAnywhere, Category = "Game/Stratagem")
	bool _isAttackStratagem = false;

	// 입력 시퀀스
	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	TArray<FKey> _inputSequence;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	FName _name;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	EStratagemType _type;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	class UTexture2D* _icon;

	// 추후 타깃 액터가 필요할 때용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	AActor* _targetActor = nullptr;

	// DeployStratagem 지연용
	FTimerHandle _deployTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Game/Stratagem")
	UAnimMontage* _animMontage;

	// 생성된 ETA 위젯 인스턴스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	class UStratagemEtaWidget* _etaWidget = nullptr;

	// ETA 계산이 완료되어 UI를 띄울 수 있는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem")
	bool _isEtaReady = false;
};
