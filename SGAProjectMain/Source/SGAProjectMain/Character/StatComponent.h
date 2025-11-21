

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitDataTable.h"
#include "StatComponent.generated.h"


enum class EDamageType : uint8;

// 사망 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
// 체력 변화 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FHPChanged, float);

// 부위 파괴 이벤트 델리게이트 (어느 부위인지 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartDestroyed);
// 부위 복구 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPartRestored);


USTRUCT(BlueprintType)
struct FUnitPartStat
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerName;
	// 파트별 스탯

	//파트의 현재체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _curHP = 100;
	//파트의 최대체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _partHP = 100;
	//파트의 장갑수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 _partAV = 0;

	// 파트의 내구성: 이 비율만큼 내구피해로 받음 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partDurability = 0.f;

	//파트의 영향력: 파트의 입은피해의 이 비율만큼 코어체력에서 감소
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partInfluence = 1.f;

	//파트의 폭발저항. 1이면 폭발피해를 전혀받지않음.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float _partExplosionImmunity = 0.f;

	FOnPartDestroyed _onPartDestroyed;

	FOnPartRestored _onPartRestored;

	FUnitPartStat(const FUnitPartLayerData& data)
	{
		_partHP = data._partHP;
		_curHP = data._partHP;
		_partAV = data._partAV;
		_partDurability = data._partDurability;
		_partInfluence = data._partInfluence;
		_partExplosionImmunity = data._partExplosionImmunity;
	}
	FUnitPartStat()
	{

	}

};

USTRUCT(BlueprintType)
struct FUnitPartStatArrayWrapper
{
	GENERATED_BODY()

public:
	// TArray<struct FUnitPartStat> 대신 사용할 멤버 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	TArray<struct FUnitPartStat> PartStats;
};



/*
	모든부위가 별도의 체력을 가질 이유가 없다. 초기화를 받지 못한 부위라면 바로 코어로 직행시키자


*/



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SGAPROJECTMAIN_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatComponent();
	void InitData(const struct FProcessedUnitData* data);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
public:	



	virtual void Reset();

	bool IsDead();

	float GetDefaultSpeed() { return _defaultMovementSpeed; }

	float GetBattleSpeed() { return _battleMovementSpeed; }
	virtual float GetCurStateSpeed();
	void ChangeSpeed(float speed);

	// 포인트 데미지(부위별) 핸들러 : takeDamage에 커스텀 이벤트 방식으로 가기때문에 일단 주석처리
	//UFUNCTION()
	//void HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
	//	UPrimitiveComponent* HitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);

	void ChangeHp(FUnitPartStat* part,float Amount);

	UFUNCTION(BlueprintCallable, Category = "Game/Stat")
	void ChangeCoreHp( float Amount);

	UFUNCTION(BlueprintCallable, Category = "Game/Stat")
	void StartRegen();
	
	FUnitPartStat* GetCoreStat() ;
	FUnitPartStat* GetPartStat(EBodyPart part);
	FUnitPartStat* GetPartStat(FName partName);

	


	TMap<EBodyPart, FUnitPartStatArrayWrapper>* GetPartDatas() { return &_partDatas; }


	FUnitPartStatArrayWrapper* GetPartData(EBodyPart part);
	// 사망 알림
	UPROPERTY(BlueprintAssignable, Category = "Game/Stat")
	FOnDeath OnDeath;

	FHPChanged _coreHpChanged;
	
	void ProcessDamage(FUnitPartStat* part, const struct FCDamageEvent* damageEvent, EDamageType damageType);
private:
	// 실제 HP 차감 및 이벤트 브로드캐스트

protected:

	
	//평소에 헬다이버의 기본걸음속도 혹은 유닛들의 정찰중 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _defaultMovementSpeed = 300.0f;

	//헬다이버의 스프린트중의 속도 혹은 유닛들의 전투 혹은 경계중 속도.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	float _battleMovementSpeed = 500.0f;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game/Stat")
	class ACharacterBase* _owner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Stat")
	TMap<EBodyPart,FUnitPartStatArrayWrapper> _partDatas;

};