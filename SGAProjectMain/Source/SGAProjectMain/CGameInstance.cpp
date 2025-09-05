// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "Gun/GunDataTable.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UCGameInstance::UCGameInstance()
{

	//포인터 초기화
	_bgmCues.Empty();
	_bgmComponent = nullptr;


}

void UCGameInstance::Init()
{
	Super::Init();

	//언리얼의 호출 순서 : 전체 앱->(게임 인스턴스 시작) -> 세션 ->(게임 인스턴스 끝)-> 세션 로딩 끝-> 최상 레벨(지속레벨)-> 레벨 -> 실제 게임
	//게임인스턴스의 Init은 전체 앱과 세션 사이에 호출됨.

	//호출 흐름중 Init을 통해 컴포넌트 생성 함수나 엔진기능을 호추랗면 기능 취소 발생. 중요한 액터나 게임모드에서 하는게 좋지 않을까	
	
	//월드생성시 브금을 틀수있도록 델리게이트 바인드 :성능면에서 이슈가 있을수 있으니 게임 모드에서 하는걸 생각해보자
	//FWorldDelegates::OnPostWorldCreation.AddUObject(this, &UCGameInstance::C_OnWorldCreated);

}

void UCGameInstance::C_OnWorldCreated(UWorld* world)
{
	PlayMusic(0);

}

FGunData UCGameInstance::GetGunDataFromTable(int32 id)
{
	FString rowName = FString::FromInt(id);
	auto row = _gunTable->FindRow<FGunData>(*rowName, TEXT(""));
	return *row;
}

void UCGameInstance::AddEarnedSample(const FSampleBundle& earnedSample)
{
	_savedSample.AddSample(earnedSample);
}

void UCGameInstance::PlayMusic(int32 index)
{


	if (_bgmComponent && _bgmComponent->IsPlaying())
	{
		_bgmComponent->Stop();
	}

	if (_bgmCues.IsValidIndex(index)&& _bgmCues[index])
	{
		USoundCue* cue = _bgmCues[index];

		_bgmComponent = UGameplayStatics::SpawnSound2D(GetWorld(), cue);

		//_bgmComponent->Play();

		//SpawnSound : bgm 배경음악 등에 유리
		//PlaySound : 단발성 소리에 유리 :  총성, 발걸음 소리 등.

	}
}
