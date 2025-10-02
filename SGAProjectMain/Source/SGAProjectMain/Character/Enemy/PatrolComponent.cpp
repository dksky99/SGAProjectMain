// Fill out your copyright notice in the Description page of Project Settings.


#include "PatrolComponent.h"

#include "AI/Patrol/CPatrolPath.h"
#include "Components/SplineComponent.h"
// Sets default values for this component's properties
UPatrolComponent::UPatrolComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;	//순찰 "행동"은 틱이 필요없다
												//틱은 행동하는 주체에게 필요

	// ...
}

bool UPatrolComponent::GetMoveTo(FVector& OutLocation, float& OutAcceptanceRadius)
{

	OutLocation = FVector::ZeroVector; //일단 초기화
	OutAcceptanceRadius = _acceptanceRadius;

	if (_path == nullptr)return false;

	//경로에서 지정한 경유지에 있는 트랜스폼 정보를 가져오기
	OutLocation = _path->GetSpline()->GetLocationAtSplinePoint(_index, ESplineCoordinateSpace::World);



	return true;
}

void UPatrolComponent::UpdateNextIndex()
{

	if (_path == nullptr)return;

	int32 count = _path->GetSpline()->GetNumberOfSplinePoints();

	if (_isReverse == false)
	{
		if (_index < count - 1)
		{
			_index++;
			return;
		}

		if (_path->GetSpline()->IsClosedLoop())
		{
			_index = 0;
			return;
		}

		_index = count - 2;
		_isReverse = !_isReverse;

	}
	else
	{
		if (_index > 0)
		{
			_index--;
			return;
		}

		if (_path->GetSpline()->IsClosedLoop())
		{
			_index = count-1;
			return;
		}

		_index = 1;
		_isReverse = !_isReverse;

	}




}

void UPatrolComponent::SetPatrolPath(ACPatrolPath* path)
{
	if (path == nullptr)
	{

	}
	_path = path;
}


// Called when the game starts
void UPatrolComponent::BeginPlay()
{
	Super::BeginPlay();

	if (patrolCurve)
	{
		FOnTimelineEvent TimelineFinishedDelegate;
		FOnTimelineFloat TimelineUpdateDelegate;

		TimelineUpdateDelegate.BindUFunction(this, FName("OnTimelineUpdate"));
		TimelineFinishedDelegate.BindUFunction(this, FName("OnTimelineFinished"));


		patrolTimeline.AddInterpFloat(
			patrolCurve,
			TimelineUpdateDelegate
		);
		patrolTimeline.SetTimelineFinishedFunc(
			TimelineFinishedDelegate
		);


		float minT, maxT;

		patrolCurve->GetTimeRange(minT,maxT);

		patrolTimeline.SetTimelineLength(maxT);
	}

	// ...
	
}


// Called every frame
void UPatrolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	patrolTimeline.TickTimeline(DeltaTime);
	// ...
}

void UPatrolComponent::OnTimeLineUpdate(float value)
{
	//스플라인 경로를 받아와서 해당 경로의 그래픽상의 진행 정도를 추적해서 따라가기

	USplineComponent* spline = _path->GetSpline();


	float splineLength = spline->GetSplineLength();

	float pathDistance = splineLength * value;


	FVector location = spline->GetLocationAtDistanceAlongSpline(
		pathDistance,
		ESplineCoordinateSpace::World
	);
	FRotator rotation = spline->GetRotationAtDistanceAlongSpline(
		pathDistance,
		ESplineCoordinateSpace::World
	);
	FVector scale(1, 1, 1);
	FTransform transform;
	transform.SetLocation(location);
	transform.SetRotation(rotation.Quaternion());
	transform.SetScale3D(scale);
}

void UPatrolComponent::OnTimelineFinished()
{
	//타임라인 종료시 필요한 동작 작성
}

void UPatrolComponent::StartTimeline()
{

	patrolTimeline.PlayFromStart();
}

