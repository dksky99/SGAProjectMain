// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PakourComponent.generated.h"


UENUM(BlueprintType)
enum class EVaultType_C :uint8
{
	OneHandVault,
	TwoHandVault,
	FrontFlip,
	Vault,
	LowMantle,
	HighMantle

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UPakourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPakourComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ReadyPakour(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void ActiveColNMove(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void DeactiveColNMove();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void TriggerPakour();

	bool DetectWall(FVector& hitLocation, FRotator& normal);
	void ScanWall(FVector detectLocation, FRotator reverseNormal);
	void MeasureWall();
	void Initialize();
	void TryPakour();
	UFUNCTION(BlueprintCallable, Category = "My Events")
	void TryVault(EVaultType_C type);

	class UAnimMontage* OneHandVault();
	class UAnimMontage* TwoHandVault();
	class UAnimMontage* FrontFlip()   ;
	class UAnimMontage* Vault()	   ;
	class UAnimMontage* LowMantle()   ;
	class UAnimMontage* HighMantle()  ;
	

	FOnMontageEnded _montageEnded;
	FOnMontageBlendingOutStarted _montageBlendOutStarted;

private:
	//벽 바로앞. 캐릭터와 벽이 만나는 지점
	FHitResult _facedWallHitResult;
	//캐릭터와 가장 가까운 벽의 윗부분. 손을 올려놓을 장소
	FHitResult _firstTopHitResult;
	//캐릭터와 가장 먼 벽의 윗부분. 벽의 두께를 알수 있게된다. 
	FHitResult _lastTopHitResult;
	FHitResult _endOfWallHitResult;
	//벽 너머에 착지할 지점.
	FHitResult _vaultLandingHitResult;
	//벽의 정면 로테이션.
	FRotator _wallRotation;

	float _wallHeight;

	UPROPERTY(BlueprintReadOnly, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	class AHellDiver* _ownerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* _movement;

	UPROPERTY(BlueprintReadOnly, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* _mesh;

	UPROPERTY(BlueprintReadOnly, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* _capsule;

	UPROPERTY(BlueprintReadOnly, Category = "Cache", meta = (AllowPrivateAccess = "true"))
	class UMotionWarpingComponent* _motionWarp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _oneHandVault;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _twoHandVault;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _frontFlip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _vault;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _lowMantle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* _highMantle;
	bool _canPakour = true;
};
