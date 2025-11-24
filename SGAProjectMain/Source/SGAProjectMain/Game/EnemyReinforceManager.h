// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyReinforceManager.generated.h"

UCLASS()
class SGAPROJECTMAIN_API AEnemyReinforceManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyReinforceManager();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//초기에 증원부대를 풀링.
	virtual void Init();

	class ASquadSpawner* GetExtraSpawner();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//여분의 스쿼드를 증원받은곳에 호출
	class AEnemySquad* GetExtraCallableSquad(FVector callPoint,AActor* target=nullptr);

	void SetSpawner(FVector callPoint, class AEnemySquad* squad, class ASquadSpawner* spawner);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game/Units", meta = (AllowPrivateAccess = "true"))
	TMap< TSubclassOf<class AEnemySquad>, int >_squadList;



	UPROPERTY()
	TArray<class AEnemySquad*> _squadPool;


	UPROPERTY()
	TArray<class AReinforceSquadSpawner*> _spawnerPool;

};
