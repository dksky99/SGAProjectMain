// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AIActingHelperLibrary.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum class EAimStatus : uint8
{
    // 완벽히 조준이 끝난 상태
    Aligned UMETA(DisplayName = "Aligned"),
    // 좌우(Yaw)는 맞았지만, 상하(Pitch) 조준이 필요한 상태
    NeedsPitchCorrection UMETA(DisplayName = "Needs Pitch Correction"),
    // 조준이 완전히 빗나간 상태
    Misaligned UMETA(DisplayName = "Misaligned")
};

UCLASS()
class SGAPROJECTMAIN_API UAIActingHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    
     // 목표 지점과 발사 속도를 기반으로 포물선 운동의 발사 방향 벡터를 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "AI|Trajectory")
    static bool CalculateLaunchDirection(   // @return 목표에 도달할 수 있는 경로가 존재하면 true를, 그렇지 않으면 false를 반환합니다.
        FVector& OutLaunchDirection,        // @param  OutLaunchDirection - 계산된 발사 방향 벡터 (단위 벡터)가 저장될 변수입니다.
        const FVector& StartLocation,       // @param  StartLocation - 발사체의 시작 위치입니다.
        const FVector& TargetLocation,      // @param  TargetLocation - 발사체의 목표 위치입니다.
        float LaunchSpeed,                  // @param  LaunchSpeed - 발사체의 초기 속력 (힘)입니다.
        bool bPreferHighArc = false,        // @param  bPreferHighArc - true일 경우 두 개의 가능한 경로 중 높은 포물선을, false일 경우 낮은 포물선을 선택합니다.
        float GravityMagnitude = 980.0f     // @param  GravityMagnitude - 중력의 크기입니다. 기본값은 지구 중력(980 cm/s^2)입니다.
    );



   

    /**
    // * 캐릭터의 좌우(Yaw) 조준이 맞았는지, 아니면 상하(Pitch) 조준만 필요한지 상세히 확인합니다.
     */
    UFUNCTION(BlueprintPure, Category = "AI|Aiming", meta = (DefaultToSelf = "Character"))
    static EAimStatus CheckAimAndSuggestCorrection(     // * @return 현재 조준 상태를 EAimStatus 열거형으로 반환합니다.
        const AActor* Character,                        // * @param Character - 확인할 캐릭터 액터입니다. 캐릭터의 정면과 위쪽 방향을 기준으로 삼습니다.
        const FVector& TargetLocation,                  // * @param TargetLocation - 목표 지점의 위치입니다.
        float YawAngleTolerance = 10.0f                 // * @param YawAngleTolerance - 좌우(Yaw) 조준의 허용 각도 (Degree 단위) 입니다.
    );


    UFUNCTION(BlueprintPure, Category = "AI|Aiming", meta = (DefaultToSelf = "Character"))
    static bool IsFacingTarget_WithAngle(       // return 상대의 시선혹은 정면벡터가 자신과 얼마나 일치하는지 확인. 
        const FVector& OwnerLoc,        // OwnerLoc.  
        const FVector& TargetLoc ,      // TargetLoc  :대상의 시선의 시작점. 머리일수 있고 총구일수 있고 그냥 캐릭터의 루트 컴포넌트의 위치일 수 있다.
        const FVector& TargetLook,      // TargetLook  :대상의 시선 방향.
        float& OutputAngle,
        float AngleTolerance = 15.0f     // 대상의 시선의 중심에서 얼마만큼 벗어나있어야 보고있지 않는것으로 치는지
        );
    UFUNCTION(BlueprintPure, Category = "AI|Aiming", meta = (DefaultToSelf = "Character"))
    static bool IsFacingTarget(       // return 상대의 시선혹은 정면벡터가 자신과 얼마나 일치하는지 확인. 
        const FVector& OwnerLoc,        // OwnerLoc.  
        const FVector& TargetLoc,      // TargetLoc  :대상의 시선의 시작점. 머리일수 있고 총구일수 있고 그냥 캐릭터의 루트 컴포넌트의 위치일 수 있다.
        const FVector& TargetLook,      // TargetLook  :대상의 시선 방향.
        float AngleTolerance = 15.0f     // 대상의 시선의 중심에서 얼마만큼 벗어나있어야 보고있지 않는것으로 치는지
    );
    UFUNCTION(BlueprintPure, Category = "MyMath|Helpers", meta = (DisplayName = "Calculate Spread Direction"))
    static FVector CalculateSpreadDirection(const FVector& Dir, float ErrorAngleDeg);
};
