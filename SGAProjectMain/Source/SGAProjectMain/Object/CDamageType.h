// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Engine/DamageEvents.h"
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

UENUM(BlueprintType)
enum class EDotType : uint8
{
    None,
    Acid,
    Fire,
    Gas,
    Bleeding
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
    bool IsExplosionDamage= false; // 관통력 

    UPROPERTY()
    float DotWeight= 0.f; // 도트피해가 있을시 대상에게 가중시킬 가중치
    // --- 공격의 '속성' ---
    UPROPERTY()
    UPrimitiveComponent* ColComp = nullptr;
};
/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API UCDamageType : public UDamageType
{
	GENERATED_BODY()

public:
    EDamageType _damageType = EDamageType::Default;
    EDotType _dotType = EDotType::None;


};
