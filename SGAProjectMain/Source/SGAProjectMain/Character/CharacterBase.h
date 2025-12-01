// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/Targetable.h"

#include "Perception/AIPerceptionTypes.h"	//감각 구분 제어, 감각 자극 관리
#include "GenericTeamAgentInterface.h"

#include "../SGAProjectMain.h"

#include "../Object/CDamageType.h"

#include "CharacterBase.generated.h"





DECLARE_DELEGATE(FReservedFunctionDelegate);

UCLASS()
class SGAPROJECTMAIN_API ACharacterBase : public ACharacter, public ITargetable, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase(const FObjectInitializer& ObjectInitializer);

	void PostInitializeComponents() override;

	static const TMap<FName, EBodyPart>& GetPartTagMap();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void InitUnit();

	static const TMap<FName, EBodyPart> PartTagMap;

	struct FCDamageEvent AttackDataToDamageEvent(class UUnitAttackDataAsset* attackData);

	//각 파트에 파괴시 호출될 델리게이트를 추가하는 메소드
	virtual void PartInit();

	//즉사 함수다. 부위파괴시 즉사한다면 이함수를 호출하여 코어hp를 0으로 만들것이다.
	UFUNCTION()
	void Critical();
	//부위파괴시 사망해야하지만 몇초정도 유예기간을 받는 함수.추가체력을 받고 몇초에걸쳐 피해를 입다 사망하게된다.
	virtual void TimeLimit() {}



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void UpDown(float value);
	void RightLeft(float value);

	void MakeSound(float loudness, FString soundName);


	float MyVertical() { return _vertical; }
	float MyHorizontal() { return _horizontal; }
	float MyDeltaAngle() { return _deltaAngle; }
	float MyYaw() { return _yaw; }
	float MyPitch() { return _pitch; }

	FText GetCharacterName() { return _name; }

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { TeamId = NewTeamId; }
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

	virtual void Landed(const FHitResult& Hit) override;

	bool _isTurnLeft = false;
	bool _isTurnRight = false;


	void KnockDownRecovery();

	//virtual float TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;
	//고스트 액터를 뽑으니 도저히 자연스럽지 못하다 포즈 복사도 제대로안되고있고 액터생성부분도 부자연스럽다
	//그냥 메시를 래그돌로 드랍하고 사망후 1분동안 캐릭터를 완전히 무력화해놓고 1분뒤 슬그머니 레벨에서 지운 후 복구해서 써먹는 것으로 가자
	//캐릭터의 순환은 스폰-> 사망->래그돌->래그돌에서 회복,풀로 복귀->상태 복구 후 스폰
	void CharacterToRagdoll();
	virtual void KnockDown(float time=3.f);
	virtual void RecoverFromKnockDown();
	virtual void Dead();
	virtual void RecoverFromDead();
	virtual void UnitUnable();
	virtual void UnitRecoverFromUnable();



	virtual void UnitDeactivate();
	virtual void UnitActivate();
	virtual FVector GetTargetLoc() { return FVector(); }


	// ITargetable을(를) 통해 상속됨
	virtual bool GetTargetLook(FVector& loc,FVector& dir)  override;



	virtual void Spawn();
	void SpawnProcessFinish();
	bool IsReadyToSpawn();
	void ReadyToSpawn();

	virtual void SpawnGhost();

	virtual void ResetUnit();

	class UStatComponent* GetStatComponent() ;
	class UCharacterStateComponent* GetStateComponent();
	UFUNCTION()
	virtual void OnPartDestroyed_Handler(EBodyPart part); // 상속받아서 각 부위별 부위파괴 구현

	UFUNCTION()
	virtual void RestoreAllParts(); // 모든 파손부위 복구 (파손체크 x)

	UFUNCTION()
	virtual void OnDeath_Handler(); // 상속받아서 죽었을 시 동작 구현

	// ITargetable을(를) 통해 상속됨
	bool IsTargetable() const override;
	// ITargetable을(를) 통해 상속됨
	FTransform GetTargetTransform() const override;


	//근접공격 기능은 우선 블루프린트에서 충돌체들을 메시에 부착하고 밀리콜리더 맵에 이름과 콜리전을 적어서 추가하는것을 시작으로 전부 추가 후 아래 함수를 호출해 초기화해준다.
	UFUNCTION(BlueprintCallable)
	virtual void InitMeleeColliders();

	//충돌시 호출될 함수
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	//랜덤으로 가지고있는 공격 데이터중하나를 골라서 호출
	virtual bool AttackMelee();
	virtual void ActionEnd();
	//공격이 실시되기전 공격데이터에서 이 공격에 사용하는 콜리더들을 리스트에 추가
	virtual void SetMeleeColisions(class UUnitAttackDataAsset* data);
	//공격모션이 끝난 후 사용했던 콜리더들을 리스트에서 해제
	virtual void ReleaseMeleeColision();
	//리스트에 있는 콜리더들 활성화, 비활성화
	virtual void ActivateMeleeColision();
	virtual void DeactivateMeleeColision();

	//데미지이벤트에 기록된 컴포넌트로부터 어느 부위인지 뽑아내는 함수
	EBodyPart GetHittedPart( const struct FCDamageEvent * DamageEvent);
	//뽑아진 부위에서 캐릭터마다 판정이 다를 수있다 만약 특별한 파트 판정을 갖는유닛들은 이것을 오버라이드.
	//기본적으로는 제일 첫번째 레이어를 가져오도록 해놨다. 판정이 필요하다면 switch문과 enum을 활용해 
	virtual struct FUnitPartStat* GetHittedPartStat(EBodyPart part, const UPrimitiveComponent* OverlappedComponent=nullptr, FVector hitLoc=FVector::ZeroVector);

	//부위의 복구를 할떄 사용. 파괴되어 효과를 발동한게 있다면 복구할떄 이걸 오버라이드해서 복구.
	virtual void RestoreParts() {}

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// 약한 비틀거림. 헬다이버라면 잠시 조준점이 튀고 적이라면 맞는순간 이동속도가 초기화된다.
	virtual void WeakStagger(float time);

	// 강한 비틀거림. 헬다이버라면 넉다운이되고 밀치기 수치만큼 튕겨나간다. 적이라면 몽타주가 캔슬된다.
	virtual void StrongStagger(float time);

	void KnockBack(FVector dir);

	//한번의 OnOff로 한 액터가 여러번의 타격을 방지.
	bool CheckHitted(AActor* target);
	void AddHitted(AActor* target);
	void ClearHitted();

	virtual void TakeHitted(FVector hitPoint,float hitPower=0.1);
	void PlayHitReaction(float time = 0.5f);

	void HitRecovery();

	class UUnitAttackDataAsset* GetCurAttackData() { return _curAttackData; }


	virtual float GetCurStateMoveSpeed() { return 0.0f; }

	// 특정 파트가 어느정도의 AV를 갖는지 반환. 총알 쪽에서 사용하려고 만들었습니다
	int32 GetPartArmorValue(class UPrimitiveComponent* ColComp);

public:
	FReservedFunctionDelegate _reservedFunction;
	
protected:
	//=================================
	//근접 공격 관련
	//=================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Melee", meta = (AllowPrivateAccess = "true"))
	TMap< FName, UShapeComponent*> _meleeColliders;
	UPROPERTY()
	TArray<class UShapeComponent*> _activateColliders;


	UPROPERTY()
	class AGunBulletBase* _activateProjectile=nullptr;

	UPROPERTY()
	class UUnitAttackDataAsset* _curAttackData=nullptr;

	UPROPERTY()
	TArray<class AActor*> _hitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	TArray<class UUnitAttackDataAsset*> _meleeAttackDatas;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game/Stat")
	class UStatComponent* _statComponent;

	static const FName StatComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/AIPerception", meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionStimuliSourceComponent* _stimuliSourceComp;

	float _vertical = 0;
	float _horizontal = 0;

	float _deltaAngle = 0.0f;

	float _yaw=0.0f;
	float _pitch = 0.0f;
	bool _isViewTurnCenter=false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/State", meta = (AllowPrivateAccess = "true"))
	class UCharacterStateComponent* _stateComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/State", meta = (AllowPrivateAccess = "true"))
	FName _unitID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	FText _name;

	FTimerHandle _knockDownTimerHandle;

	

	FGenericTeamId TeamId;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _spawnMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _reinforcementMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _deadMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* _hitReactionMontage;

	FTimerHandle _respawnTimer;

	bool _isReadyToSpawn = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game/Squad", meta = (AllowPrivateAccess = "true"))
	float _respawnCoolDown;






};
