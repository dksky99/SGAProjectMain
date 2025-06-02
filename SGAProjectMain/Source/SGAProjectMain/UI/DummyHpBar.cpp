// Fill out your copyright notice in the Description page of Project Settings.


#include "DummyHpBar.h"

#include "Components/ProgressBar.h"

void UDummyHpBar::SetHp(float ratio)
{
	_curHpPB->SetPercent(ratio);
}
