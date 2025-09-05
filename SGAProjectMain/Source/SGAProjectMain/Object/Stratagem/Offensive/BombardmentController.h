// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BombardmentController.generated.h"

UCLASS()
class SGAPROJECTMAIN_API ABombardmentController : public AActor
{
	GENERATED_BODY()
	
public:
	ABombardmentController();

protected:
	virtual void BeginPlay() override;

	// 웨이브/포탄 스폰 루프
	void SpawnNextWave();
	void SpawnNextShell();

	// 한 발 스폰
	void SpawnOneShellAt(const FVector& target);

	// 랜덤 산포 타깃 샘플링(피격 중심 기준)
	FVector SampleTargetLocation() const;


protected:
	// 떨어뜨릴 액터(포탄 또는 드롭 포드)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	TSubclassOf<AActor> _projectileClass;

	// 한 웨이브당 투하 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	int32 _shellsPerWave = 8;

	// 총 웨이브 수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	int32 _waveCount = 3;

	// 웨이브 간 간격(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	float _waveInterval = 0.6f;

	// 같은 웨이브 내 포탄 간 간격(초) 최소 0.01f는 세팅되어 있어야한다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	float _intraWaveInterval = 0.08f; 

	// 산포 반경(센티미터)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	float _scatterRadius = 1200.0f;

	// 투하 시작 높이(Z). 현재 투하 원점은 (0,0,0) 이므로 (0,0,dropHeight)에서 시작
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	float _dropHeight = 1500.0f;

	// 낙하 속도(ProjectileMovement가 있을 때 적용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	float _dropSpeed = 8000.0f;

	// 랜덤 시드(0이면 시간 기반)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	int32 _seed = 0;

	// 피격 중심(신호기/스트라타젬 위치). BeginPlay에서 컨트롤러 스폰 위치로 설정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	FVector _impactCenter = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stratagem/Bombard")
	AActor* _owner = nullptr;

	int32 _waveIdx = 0;
	int32 _shellIdxInWave = 0;

	FTimerHandle _waveTimer;
	FTimerHandle _intraWaveTimer;

	mutable FRandomStream _rng;
};
