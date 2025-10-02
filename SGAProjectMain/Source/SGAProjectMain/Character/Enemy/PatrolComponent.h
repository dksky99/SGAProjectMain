// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "PatrolComponent.generated.h"


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
	int32 _index;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float _distance;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	bool _isReverse;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float _acceptanceRadius;	//변동이 생길떄 허용범위

	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class ACPatrolPath* _path;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* patrolCurve;

	FTimeline patrolTimeline;


};
