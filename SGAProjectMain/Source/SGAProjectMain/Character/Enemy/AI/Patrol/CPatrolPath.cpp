// Fill out your copyright notice in the Description page of Project Settings.


#include "CPatrolPath.h"
#include "Components/SplineComponent.h"
#include "Components/TextRenderComponent.h"


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

