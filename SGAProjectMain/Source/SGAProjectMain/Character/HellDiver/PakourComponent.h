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
	FrontFlip
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

	FOnMontageEnded _montageEnded;
	FOnMontageBlendingOutStarted _montageBlendOutStarted;

private:
	FHitResult _facedWallHitResult;
	FHitResult _firstTopHitResult;
	FHitResult _lastTopHitResult;
	FHitResult _endOfWallHitResult;
	FHitResult _vaultLandingHitResult;
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

	bool _canPakour = true;
};
