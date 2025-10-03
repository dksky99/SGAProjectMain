// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStateComponent.generated.h"

/*
	상태이상 :
	입력이 가해지면 중첩은 불가하고 지속을 초기화하는것으로 한다. 1틱당 0.25초

	화상 : 피해를 초당 100 													: 지속: 3초   틱 25
	가스 : 피해를 초당 25,						25퍼센트 이속감소			: 지속: 6초   틱 6
	산성(방울)  : 첫틱에 3,이후엔 피해없음		25퍼센트 이속감소			: 지속: 4초   틱 0
	산성(스트림): 첫타에 100 이후틱부턴 20      25퍼센트 이속감소			: 지속: 0.5초 틱 5
	출혈 : 매틱마다 1씩 회복제를 맞지 않으면 영구지속						: 영구        틱 1
	가시 : 지형지물중 캐릭터와 중첩시 피해를 입히는 구조물, 중첩에 벗어날때까지 :지속 : 덤불에서 벗어날때까지 ,틱 :1
	강경직 : 크게 흔들리며 1.5초간 행동불능	. 약경직은 그냥 없는것으로 치거나 고려해봐야할듯
	기절 : 기절판정을 내는 장비로인해 기절상태. 행동불가, 

	지속피해 : 화상, 가스, 산성, 출혈
	슬로우 : 가스,산성,
	행동불가: 강경직, 기절

*/

UENUM(BlueprintType)
enum class EAbnormality : uint8
{
	Fire=0,
	Burn =1,
	Gas,
	AcidBubble,
	AcidStream,
	bleeding ,
	Thornbush ,
	StrongStagger, 
	Shock ,

	Max
};
UENUM()
enum class EAbnormalityState : uint32
{
	None = 0 UMETA(Hidden),
	Fire = 1<<0,
	Burn = 1<<1,
	Gas =1<<2,
	AcidBubble = 1 << 3,
	AcidStream = 1 << 4,
	bleeding = 1 << 5,
	Thornbush = 1 << 6,
	StrongStagger = 1 << 7,
	Shock = 1 << 8,

	Max
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterStateComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddAbnormality(EAbnormality abnormality);
	void SubAbnormality(EAbnormality abnormality);
	void SubAbnormality(EAbnormalityState abnormality);

	bool CheckAbnormality(EAbnormality abnormality);
	bool CheckAbnormality(uint32 abnormality);

	bool IsMoving();

	bool ActionBegin();
	void ActionEnd() { _isActing = false; }

	bool IsActing() { return _isActing; }

	
	bool IsUnable();
	bool IsSlow();
	void ActiveSlow();
	void DeactiveSlow();
	void CalcAbnormalityTime(float deltaTime);

	int CalcActivates(uint32 type,float deltaTime);

	virtual void Reset();

protected:

	class ACharacterBase* _owner;

	FTimerHandle _dotTickTimer;


	UPROPERTY()
	uint32 _activeAbnormalities;

	UPROPERTY()
	TMap<EAbnormalityState, float> _remainTimes;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool _isActing=false;
		

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool _isUnable = false;
};
