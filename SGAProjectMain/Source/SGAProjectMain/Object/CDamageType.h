// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
#include "AbnormalityTable.h"
#include "CDamageType.generated.h"



UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Default,
	Acid,
	Fire,
	Gas,
    Bleeding,
    Arc

};



USTRUCT(BlueprintType)
struct FCDamageEvent : public FPointDamageEvent
{
    GENERATED_BODY()

    // --- 이 구조체의 고유 ID ---
    // FDamageEvent를 상속받을 때 식별을 위해 필요합니다.
    static const int32 ClassID = 1111;
    virtual int32 GetTypeID() const override { return  FCDamageEvent::ClassID; }
    

    // (기본 생성자)
    FCDamageEvent() : FPointDamageEvent() {}
    
    // 이 이벤트를 발생시킨 DamageType 클래스를 쉽게 가져오기 위한 생성자
    FCDamageEvent(TSubclassOf<class UDamageType> InDamageTypeClass): FPointDamageEvent()
    {
        DamageTypeClass = InDamageTypeClass;
    }

    //virtual FDamageEvent* GetCopy() const override
    //{
    //    // 현재 객체를 복사하여 새로운 객체를 힙에 할당합니다.
    //    return new FCDamageEvent(*this);
    //}

    // --- 공격마다 변하는 핵심 '수치'들 ---
    UPROPERTY()
    int32 BaseDamage = 0; // 일반 피해

    UPROPERTY()
    int32 DurabilityDamage = 0; // 내구 피해

    UPROPERTY()
    int32 DemolitionDamage = 0; // 철거 피해

    UPROPERTY()
    int32 PenetrationLevel = 0; // 관통력 

    UPROPERTY()
    int32 Stagger = 0; // 비틀거림 유발. 헬다이버는 30이넘으면 날라간다.

    UPROPERTY()
    int32 PushForce = 0; // 밀치기 위력. 만약 헬다이버가 비틀거림유발이 30이넘어서 넉다운이될때 이수치만큼의 위력으로 날라가게될것.

    UPROPERTY()
    bool IsExplosionDamage= false; // 폭발피해인지 : 폭발저항력떄문에 

    UPROPERTY()
    UPrimitiveComponent* ColComp;
};
/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCDamageType : public UDamageType
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    EDamageType _damageType = EDamageType::Default;
    UPROPERTY(EditAnywhere,BlueprintReadWrite)
    EAbnormality _abnormalityType = EAbnormality::Max;


};
