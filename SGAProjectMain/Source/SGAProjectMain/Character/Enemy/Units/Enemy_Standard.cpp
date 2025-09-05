// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Standard.h"
#include "Components/CapsuleComponent.h"

AEnemy_Standard::AEnemy_Standard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	_claw_L = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Claw_L"));
	_claw_R = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Claw_R"));

	_claw_L->SetupAttachment(GetMesh());
	_claw_R->SetupAttachment(GetMesh());

}

bool AEnemy_Standard::AttackMelee()
{

	

	return true;
}

void AEnemy_Standard::ActivateClaw_L()
{
	_claw_L->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemy_Standard::ActivateClaw_R()
{
	_claw_R->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AEnemy_Standard::DeactivateClaw_L()
{
	_claw_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy_Standard::DeactivateClaw_R()
{
	_claw_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
