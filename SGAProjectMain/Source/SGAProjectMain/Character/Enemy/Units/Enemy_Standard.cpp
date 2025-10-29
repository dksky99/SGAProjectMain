// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Standard.h"
#include "Components/ShapeComponent.h"
#include "Components/CapsuleComponent.h"
#include "../../CharacterStateComponent.h"
#include "../../CharacterAnimInstance.h"
#include "../../../Data/UnitAttackDataAsset.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../../Helper/AIActingHelperLibrary.h"
#include "../../../MainGameMode.h"
#include "../../../Game/EnemyReinforceManager.h"
AEnemy_Standard::AEnemy_Standard(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	

}


bool AEnemy_Standard::CheckAbleTryNear(AActor* target)
{
	if (target == nullptr)
		return false;
	return true;
}

bool AEnemy_Standard::CheckAbleTryMiddle(AActor* target)
{
	if (target == nullptr)
		return false;
	if (_hasReinforceAuthority == false)
		return false;
	return false;
}

bool AEnemy_Standard::CheckAbleTryFar(AActor* target)
{
	if (target == nullptr)
		return false;
	return true;
}

bool AEnemy_Standard::TryNear(AActor* target)
{
	if (CheckAbleTryNear(target) == false)
		return false;
	if (AttackMelee())
		return true;


	return false;
}

bool AEnemy_Standard::TryMiddle(AActor* target)
{
	if (CheckAbleTryMiddle(target) == false)
		return false;
	//if (TryCalling(target))
	//	return true;
	//
	return false;
}

bool AEnemy_Standard::TryFar(AActor* target)
{
	if (CheckAbleTryFar(target) == false)
		return false;
	if (TryBurrow(target))
		return true;

	return false;
}

bool AEnemy_Standard::TryCalling(AActor* target)
{
	if (target == nullptr)
		return false;
	if (_hasReinforceAuthority == false)
		return false;

	UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (_callReinforce_Animation == nullptr)
		return false;
	if (anim == nullptr)
		return false;

	if (_stateComp->ActionBegin() == false)
		return false;
	if (_reservedFunction.IsBound())
		_reservedFunction.Unbind();
	_reservedFunction.BindUObject(this, &AEnemy_Standard::CallingReinforce);
	const float Duration = anim->PlayAnimMontage(_callReinforce_Animation);
	_hasReinforceAuthority = false;
	return true;
}

void AEnemy_Standard::CallingReinforce()
{

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(this);
	if (!World) return;

	AMainGameMode* GM = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GM)return;
	if (!GM->GetEnemyReinforceManager()) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC) return;

	ACharacterBase* MyChar = Cast<ACharacterBase>(PC->GetPawn());
	if (!MyChar)return;


	GM->GetEnemyReinforceManager()->GetExtraCallableSquad(MyChar->GetActorLocation());
}


bool AEnemy_Standard::TryBurrow(AActor* target)
{

	FVector pos = target->GetActorLocation();

	//NavMesh 찾기
	auto naviSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	if (naviSystem->IsValidLowLevel() == false)
		return false;



	//반환받을 랜덤한 위치.
	FNavLocation randLocation;
	//일정 반경안의 랜덤한 지점을 가져오는 함수
	if (naviSystem->GetRandomPointInNavigableRadius(pos, 300, randLocation))
	{

		_burrowOutLoc = randLocation;
		UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
		if (_burrowIn_Animation == nullptr)
			return false;
		if (anim == nullptr)
			return false;

		if (_stateComp->ActionBegin() == false)
			return false;
		if (_reservedFunction.IsBound())
			_reservedFunction.Unbind();
		_reservedFunction.BindUObject(this, &AEnemy_Standard::BurrowIn);
		const float Duration = anim->PlayAnimMontage(_burrowIn_Animation);
		UE_LOG(LogTemp, Display, TEXT("Try Burrow"));
		return(true);
			
	}
	return false;
}

void  AEnemy_Standard::BurrowIn()
{
	//애님 몽타주 이후 사라진 후 타겟 위치로 이동,특정 시간 후 애님몽타주를 실행하면서 모습을 드러냄.

	UE_LOG(LogTemp, Display, TEXT("Burrow In"));

	// 3. 중력/물리 끄기
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->GravityScale = 0.0f;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	GetWorld()->GetTimerManager().SetTimer(_burrowTimer,this,&AEnemy_Standard::BurrowOut, _burrowOutDelay, false);

}

void AEnemy_Standard::BurrowOut()
{

		UE_LOG(LogTemp, Display, TEXT("Burrow Out"));
	SetActorLocation(_burrowOutLoc);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->GravityScale = 1.0f;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	UCharacterAnimInstance* anim = Cast<UCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (_burrowOut_Animation == nullptr)
		return ;
	if (anim == nullptr)
		return ;
	//이 버로우 들어가는 몽타주에는 액션앤드가 없다 
	const float Duration = anim->PlayAnimMontage(_burrowOut_Animation);
}


