// Fill out your copyright notice in the Description page of Project Settings.


#include "CPatrolPath.h"
#include "Components/SplineComponent.h"
#include "Components/TextRenderComponent.h"
#include "NavigationSystem.h"

// Sets default values
ACPatrolPath::ACPatrolPath()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; //틱 없음 (그냥 지형지물)
											// 경로 갱신이 일어나도 외부에서 데이터를 주는 경우
											// = 함수 연산으로 경로가 갱신되는 유형이면 false
											// 유일한 예외는 "스스로가 학습을 하는 경우"

	_spline = CreateDefaultSubobject<USplineComponent>("Spline path");

	_spline->SetupAttachment(RootComponent);
}

void ACPatrolPath::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
	//생성시 이벤트 =지연 연산

	_spline->SetClosedLoop(_isLoop);




}

FVector ACPatrolPath::GetStartPosition()
{
    return FVector();
}

// Called when the game starts or when spawned
void ACPatrolPath::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACPatrolPath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACPatrolPath::MakeSpline(FVector start, FVector end)
{
	USplineComponent* Spline = GetSpline();
	if (!Spline) return;

	// A. 기존 스플라인 포인트를 모두 제거합니다.
	Spline->ClearSplinePoints(true);

	UWorld* World = GetWorld();

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem)
	{
		// NavSystem이 없으면, 스플라인 액터의 위치를 시작점으로 설정하고 종료합니다.
		SetActorLocation(start);
		return;
	}

	// C. 내비게이션 경로 찾기
	FPathFindingQuery Query;
	// 경로를 찾을 에이전트 클래스 (일반적으로 ADefaultNavigationData)를 지정합니다.
	ANavigationData* NavData = Cast<ANavigationData>(NavSystem->GetNavDataForActor(*this));

	// FNavAgentProperties AgentProps; // 필요시 에이전트 속성 설정

	Query.StartLocation = start;
	Query.EndLocation = end;
	Query.NavData = NavData;
	// Query.AgentProperties = AgentProps; // 필요시 사용

	FPathFindingResult Result = NavSystem->FindPathSync(Query, EPathFindingMode::Regular);

	// 경로를 찾지 못했거나 유효하지 않은 경우
	if (!Result.Path.IsValid() || Result.Result != ENavigationQueryResult::Success)
	{
		// 네비게이션 실패 시, 직선 경로로 대체하거나 오류 처리합니다.
		SetActorLocation(start);
		Spline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, true);
		FVector LocalEnd = end - start;
		Spline->AddSplinePoint(LocalEnd, ESplineCoordinateSpace::Local, true);
		return;
	}

	FNavigationPath* NavPath = Result.Path.Get();

	// 시작 위치를 스플라인 액터의 위치로 설정 (로컬 좌표계의 기준)
	SetActorLocation(start);


    // E. 경로상의 포인트를 스플라인에 추가
    auto PathPoints = NavPath->GetPathPoints();
    if (PathPoints.Num() < 2) return; // 시작, 끝점 포함 최소 2개 필요

    const float DesiredInterval = 500.0f; // 원하는 중간점 간격 (예: 500cm)
    float CurrentDistance = 0.0f;
    FVector LastAddedPoint = PathPoints[0]; // 시작점 (Point 0)

    // 시작점을 스플라인의 첫 번째 점으로 추가 (액터 위치이므로 로컬은 FVector::ZeroVector)
    Spline->AddSplinePoint(FVector::ZeroVector, ESplineCoordinateSpace::Local, true);


    // 내비게이션 경로의 모든 구간을 순회하며 일정한 간격으로 점을 추출합니다.
    for (int32 i = 0; i < PathPoints.Num() - 1; ++i)
    {
        const FVector SegmentStart = PathPoints[i];
        const FVector SegmentEnd = PathPoints[i + 1];
        const float SegmentLength = FVector::Dist(SegmentStart, SegmentEnd);

        // 현재 세그먼트를 순회하며 필요한 점들을 추가합니다.
        float CurrentSegmentOffset = 0.0f;
        while (CurrentSegmentOffset < SegmentLength)
        {
            // 다음 목표 거리 (LastAddedPoint에서 DesiredInterval만큼 떨어진 지점)
            float DistanceToTravel = DesiredInterval - CurrentDistance;

            // 현재 세그먼트 내에서 이동 가능한 거리
            if (DistanceToTravel < (SegmentLength - CurrentSegmentOffset))
            {
                // 다음 점이 세그먼트 내에 있다면 추가
                FVector Direction = (SegmentEnd - SegmentStart).GetSafeNormal();
                FVector NewPoint = SegmentStart + Direction * (CurrentSegmentOffset + DistanceToTravel);

                // 로컬 좌표로 변환하여 스플라인에 추가
                FVector LocalPoint = NewPoint - start;
                Spline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, true);

                LastAddedPoint = NewPoint;
                CurrentDistance = 0.0f; // 다음 간격 측정 시작
                CurrentSegmentOffset += DistanceToTravel;
            }
            else
            {
                // 다음 점이 세그먼트를 넘어간다면, 남은 세그먼트 길이를 CurrentDistance에 더하고 다음 세그먼트로 이동
                CurrentDistance += (SegmentLength - CurrentSegmentOffset);
                CurrentSegmentOffset = SegmentLength; // 루프 종료
            }
        }
    }

    // F. 마지막 종점(End)이 포함되었는지 확인하고 추가 (항상 내비게이션 종점과 일치)
    // 마지막 점은 항상 추가되어야 합니다.
    FVector LastPoint = PathPoints.Last();
    FVector LocalLastPoint = LastPoint - start;

    // 만약 이미 마지막 점이 추가된 것과 매우 가깝다면 건너뛸 수 있습니다.
    if (FVector::Dist(LastAddedPoint, LastPoint) > 10.0f) // 10cm 이상 떨어져 있다면
    {
        Spline->AddSplinePoint(LocalLastPoint, ESplineCoordinateSpace::Local, true);
    }

    // G. 패트롤 속성 설정 (스쿼드 사이클은 루프가 아님)
    Spline->SetClosedLoop(false);

}

