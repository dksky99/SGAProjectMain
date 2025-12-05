// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "PatrolComponent.generated.h"

/*
	정찰의 로직.
	1.패트롤 태스크가 실행. 
	2. 패트롤컴포넌트로부터 다음 좌표를 얻어옴
	- 패트롤컴포넌트는 패트롤패스의 스플라인으로부터 다음 경유지의 좌표를 가져옴.





*/


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPatrolComponent();

	bool GetMoveTo(FVector& OutLocation, float& OutAcceptanceRadius);

	void UpdateNextIndex(); //distance를 사용할 경우, 곡선의 형태를 직접 따서 벡터+타임라인으로 쓰는 경우가 많아서

	void SetPatrolPath(class ACPatrolPath* path);

	bool HasPath() { return _path != nullptr; }

protected:

	

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnTimeLineUpdate(float value);

	UFUNCTION()
	void OnTimelineFinished();

	UFUNCTION()
	void StartTimeline();

protected:
	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	int32 _index=1;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float _distance;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	bool _isReverse=false;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float _acceptanceRadius=500.f;	//변동이 생길떄 허용범위

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class ACPatrolPath* _path;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* patrolCurve;

	FTimeline patrolTimeline;


};
