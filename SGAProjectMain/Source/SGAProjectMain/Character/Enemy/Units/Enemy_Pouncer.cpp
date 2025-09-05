// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Pouncer.h"
#include "Components/CapsuleComponent.h"


AEnemy_Pouncer::AEnemy_Pouncer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	_claw_L = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Claw_L"));
	_claw_R = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Claw_R"));

	_claw_L->SetupAttachment(GetMesh());
	_claw_R->SetupAttachment(GetMesh());

}
