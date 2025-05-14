// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Throwable.h"
#include "Stratagem.generated.h"

/**
 * 
 */
UCLASS()
class SGAPROJECTMAIN_API AStratagem : public AThrowable
{
	GENERATED_BODY()
	
public:
	void DeployStratagem();
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	bool IsSurfaceAttachable(const FHitResult& Hit);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Stratagem")
	TSubclassOf<AActor> _objectToSpawn; // ���� ����/��Ʈ�� ��

	UPROPERTY(EditAnywhere, Category = "Stratagem")
	float _deployDelay = 5.0f; // ��ô �� �� �� �ڿ� ����

	UPROPERTY(EditAnywhere, Category = "Stratagem")
	bool _isAttackStratagem = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stratagem")
	AActor* _targetActor = nullptr;

	FTimerHandle _deployTimerHandle;
};
