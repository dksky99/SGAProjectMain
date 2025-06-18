// Fill out your copyright notice in the Description page of Project Settings.


#include "PakourComponent.h"
#include "Engine/DamageEvents.h"
#include "../../Helper/H_Vector.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"
#include "HellDiver.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimInstance.h"             // UAnimInstance, Montage_Play(), GetActiveInstanceForMontage()
#include "Animation/AnimMontage.h"              // UAnimMontage
// Sets default values for this component's properties
UPakourComponent::UPakourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPakourComponent::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
	// ...
	
}

void UPakourComponent::ReadyPakour(UAnimMontage* Montage, bool bInterrupted)
{
	_canPakour = true;
	UE_LOG(LogTemp, Display, TEXT("Call ReadyPakour"));
}

void UPakourComponent::ActiveColNMove(UAnimMontage* Montage, bool bInterrupted)
{

	_capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	_movement->SetMovementMode(EMovementMode::MOVE_Walking);
	UE_LOG(LogTemp, Display, TEXT("Call ActiveColNMove"));
}

void UPakourComponent::DeactiveColNMove()
{

	_capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_movement->SetMovementMode(EMovementMode::MOVE_Flying);
}


// Called every frame
void UPakourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPakourComponent::TriggerPakour()
{
	if (_canPakour == false)
		return;
	FVector hitLoc;
	FRotator normal;
	UE_LOG(LogTemp, Display, TEXT("TriggerPakour"));
	bool bResult=DetectWall(hitLoc, normal);
	if (bResult == false)
	{

		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Detect"));

	ScanWall(hitLoc, normal);

	UE_LOG(LogTemp, Display, TEXT("ScanFinish"));
	MeasureWall();
	UE_LOG(LogTemp, Display, TEXT("MeasureFinish"));
	TryPakour();
	UE_LOG(LogTemp, Display, TEXT("TryPakour"));
}

bool UPakourComponent::DetectWall(FVector& hitLocation, FRotator& normal)
{
	for (int i = 0; i < 8; i++)
	{
		//자신의 정면방향으로 발에서부터 위로올라가며 정면방향으로 캡슐 콜리전을 해서 벽이 있는지 탐색. 탐지를 명확히하기위해 좀더 뒤에서부터 시작.
		FVector temp = H_Vector::MoveVectorDownward(GetOwner()->GetActorLocation(), 60);
		temp = H_Vector::MoveVectorUpward(temp, i * 20);
		FVector end = temp + (GetOwner()->GetActorForwardVector() * 200.0f);
		FVector start= H_Vector::MoveVectorBackward(temp, GetOwner()->GetActorRotation(), 30);
		 //start에서 정면 200만큼 추가로.

		FHitResult hit;
		
		FCollisionQueryParams params(NAME_None, false, GetOwner());

		bool bResult=GetWorld()->SweepSingleByChannel(hit,
			start,
			end,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(10),
			params);
		//찾았으면 벽의 위치와 벽의 법선백터와 결과를 반환.
		if (bResult)
		{
			hitLocation = hit.Location;
			normal = hit.Normal.Rotation();
			return true;
		}
	}
	return false;
}

void UPakourComponent::ScanWall(FVector detectLocation, FRotator reverseNormal)
{
	for (int i = 0; i < 30; i++)
	{
		//탐지된 위치로부터 충분히 높은 위에서부터 아래로 10씩 내려가면서.
		FVector temp = H_Vector::MoveVectorUpward(detectLocation, 300);
		temp = H_Vector::MoveVectorDownward(temp, i * 10);
		//정면방향으로 100만큼 라인트레이스
		FVector start = H_Vector::MoveVectorBackward(temp, reverseNormal, 20);
		FVector end=H_Vector::MoveVectorForward(temp,reverseNormal,80);

		FHitResult hit;

		FCollisionQueryParams params(NAME_None, false, GetOwner());

		bool bResult = GetWorld()->LineTraceSingleByChannel(hit,
			start,
			end,
			ECC_Visibility,
			params);
		//찾았으면 break
		if (bResult)
		{
			_facedWallHitResult = hit;

			//UE_LOG(LogTemp, Display, TEXT("Find _facedWallHitResult"));
			break;
		}
	}
	//지금 벽의 높이 와 그 충돌위치가 나왔음.
	_wallRotation = H_Vector::ReverseNormal(_facedWallHitResult.Normal);
	for (int i = 0; i < 7; i++)
	{

		//충돌지점에서 앞으로 20단위로 위에서 아래로 스피어 트레이스를 해서 벽의 두께를 확인함.
		FVector temp=H_Vector::MoveVectorForward(_facedWallHitResult.Location,_wallRotation,i*20);
		FVector start = H_Vector::MoveVectorUpward(temp, 10);
		FVector end = H_Vector::MoveVectorDownward(start, 10);

		FHitResult hit;

		FCollisionQueryParams params(NAME_None, false, GetOwner());

		bool bResult = GetWorld()->SweepSingleByChannel(hit,
			start,
			end,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(5),
			params);
		if (i == 0)
		{
			//첫 충돌로 제일 앞의 충돌지점.
			if (bResult)
			{
				_firstTopHitResult = hit;

				//UE_LOG(LogTemp, Display, TEXT("Find _firstTopHitResult"));
			}

		}
		else
		{
			//계속 갱신되며 충돌이 되면 갱신, 안되면 반복을 종료 . 이 벽의 두께가 여기까지임을 확인.
			if (bResult)
			{
				//UE_LOG(LogTemp, Display, TEXT("Find _lastTopHitResult"));
				_lastTopHitResult = hit;
			}
			else
			{
				break;
			}
		}

	}

	{
		//벽의 마지막에서 살짝 앞으로가서 벽방향으로 스피어트레이스
		FVector start = H_Vector::MoveVectorForward(_lastTopHitResult.ImpactPoint, _wallRotation, 20);
		FVector end = _lastTopHitResult.ImpactPoint;

		FHitResult hit;

		FCollisionQueryParams params(NAME_None, false, GetOwner());

		bool bResult = GetWorld()->SweepSingleByChannel(hit,
			start,
			end,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(10),
			params);
		if (bResult==false)
		{

			//UE_LOG(LogTemp, Display, TEXT("Is not Wall"));
			return;
		}
		//이 위치가 벽의 끝임을 확인, 그위치를 저장.
		_endOfWallHitResult = hit;

		//UE_LOG(LogTemp, Display, TEXT("Find _endOfWallHitResult"));
	}
	{
		//벽에서 살짝 앞에서 밑으로 스피어트레이스. 착지지점을 만든다.
		FVector start = H_Vector::MoveVectorForward(_endOfWallHitResult.ImpactPoint, _wallRotation, 60);
		FVector end = H_Vector::MoveVectorDownward(start, 100);

		FHitResult hit;

		FCollisionQueryParams params(NAME_None, false, GetOwner());

		bool bResult = GetWorld()->SweepSingleByChannel(hit,
			start,
			end,
			FQuat::Identity,
			ECC_Visibility,
			FCollisionShape::MakeSphere(10),
			params);
		if (!bResult)
		{
			//UE_LOG(LogTemp, Display, TEXT("Not Found VaultLandingPoint"));
			return;

		}
		_vaultLandingHitResult = hit;
		//UE_LOG(LogTemp, Display, TEXT("Find _vaultLandingHitResult"));
	}
	

	
}

void UPakourComponent::MeasureWall()
{
	//벽 높이를 측정. 제일 가까운 벽의 충돌지점과 자신의 높이를 뺸 값.
	if (_firstTopHitResult.bBlockingHit && _facedWallHitResult.bBlockingHit)
	{
		if (_ownerCharacter)
		{

			_wallHeight = _firstTopHitResult.ImpactPoint.Z - _ownerCharacter->GetMesh()->GetComponentLocation().Z;
		}
	}
	else
	{
		_wallHeight = 0.0f;

	}

	//UE_LOG(LogTemp, Display, TEXT("wallHeight : %f"),_wallHeight);
}

void UPakourComponent::Initialize()
{
	_ownerCharacter = Cast<AHellDiver>(GetOwner());
	_capsule = _ownerCharacter->GetCapsuleComponent();
	_mesh = _ownerCharacter->GetMesh();
	_motionWarp = _ownerCharacter->GetMotionWarp();
	_movement = _ownerCharacter->GetCharacterMovement();
	_montageEnded.BindUObject(this, &UPakourComponent::ReadyPakour);
	_montageBlendOutStarted.BindUObject(this, &UPakourComponent::ActiveColNMove);
}

void UPakourComponent::TryPakour()
{
	if (_firstTopHitResult.bBlockingHit == false)
	{
		return;
	}
	//착지상태인지
	if (_movement->IsMovingOnGround() == false)
	{
		return;

	}
	if (_wallHeight > 300.0f)
	{
		//Too High Wall
		return;
	}
	if (_wallHeight > 130)
	{
		//Climb;
		return;
	}
	if (FMath::IsNearlyEqual(_ownerCharacter->GetVelocity().Size2D(), 0.0f, 5.0f))
	{
		//Mantle
		return;
	}
	if (_wallHeight > 100.0f)
	{
		//FrontFlip
		TryVault(EVaultType_C::FrontFlip);
		return;
	}
	if (_wallHeight > 90)
	{
		//TwoHandVault
		TryVault(EVaultType_C::TwoHandVault);

		return;

	}
	if (_wallHeight > 60)
	{
		//OneHandVault
		TryVault(EVaultType_C::OneHandVault);

		return;

	}
	else
	{

		return;
	}
}

void UPakourComponent::TryVault(EVaultType_C type)
{
	_canPakour = false;

	_capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	_movement->SetMovementMode(EMovementMode::MOVE_Flying);

	UAnimInstance* animInstance = _mesh->GetAnimInstance();
	if (!animInstance) return;

	UAnimMontage* SelectedMontage = nullptr;

	switch (type)
	{
	case EVaultType_C::OneHandVault:
		UE_LOG(LogTemp, Display, TEXT("OneHandVault Height: %f"), _wallHeight);
		SelectedMontage = _oneHandVault;
		break;

	case EVaultType_C::TwoHandVault:
	{
		UE_LOG(LogTemp, Display, TEXT("TwoHandVault Height: %f"), _wallHeight);
		UE_LOG(LogTemp, Display, TEXT("FirstTopHit: %f : %f : %f"), _firstTopHitResult.Location.X, _firstTopHitResult.Location.Y, _firstTopHitResult.Location.Z);
		UE_LOG(LogTemp, Display, TEXT("VaultLanding: %f : %f : %f"), _vaultLandingHitResult.Location.X, _vaultLandingHitResult.Location.Y, _vaultLandingHitResult.Location.Z);
		UE_LOG(LogTemp, Display, TEXT("Actor Rotation: %f : %f : %f"), GetOwner()->GetActorRotation().Pitch, GetOwner()->GetActorRotation().Yaw, GetOwner()->GetActorRotation().Roll);
		UE_LOG(LogTemp, Display, TEXT("wall Rotation: %f : %f : %f"), _wallRotation.Pitch, _wallRotation.Yaw, _wallRotation.Roll);
		SelectedMontage = _twoHandVault;
		FVector temp = H_Vector::MoveVectorForward(_firstTopHitResult.Location, _wallRotation, 62);
		temp = H_Vector::MoveVectorDownward(temp, 46.5);
		FRotator rot = _wallRotation;
		rot.Roll = 0;
		_motionWarp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("VaultStart"),temp, rot);
		temp = _vaultLandingHitResult.Location;
		_motionWarp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("VaultEnd"),temp, rot);

	}
		break;

	case EVaultType_C::FrontFlip:
	{
		UE_LOG(LogTemp, Display, TEXT("FrontFlip Height: %f"), _wallHeight);
		UE_LOG(LogTemp, Display, TEXT("FirstTopHit: %f : %f : %f"), _firstTopHitResult.Location.X, _firstTopHitResult.Location.Y, _firstTopHitResult.Location.Z);
		UE_LOG(LogTemp, Display, TEXT("VaultLanding: %f : %f : %f"), _vaultLandingHitResult.Location.X, _vaultLandingHitResult.Location.Y, _vaultLandingHitResult.Location.Z);
		UE_LOG(LogTemp, Display, TEXT("Actor Rotation: %f : %f : %f"), GetOwner()->GetActorRotation().Pitch, GetOwner()->GetActorRotation().Yaw, GetOwner()->GetActorRotation().Roll);
		UE_LOG(LogTemp, Display, TEXT("wall Rotation: %f : %f : %f"),_wallRotation.Pitch, _wallRotation.Yaw, _wallRotation.Roll);
		SelectedMontage = _frontFlip;
		FVector temp = H_Vector::MoveVectorBackward(_firstTopHitResult.Location, _wallRotation, 100);
		FRotator rot = _wallRotation;
		rot.Roll = 0;
		_motionWarp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("VaultStart"), temp, rot);
		FVector temp2 = H_Vector::MoveVectorRight(_vaultLandingHitResult.Location, _wallRotation, 5);
		_motionWarp->AddOrUpdateWarpTargetFromLocationAndRotation(TEXT("VaultEnd"), temp2, rot);
		break;

	}

	default:
		return;
	}

	if (!SelectedMontage) return;

	// 애니메이션 재생
	const float Duration = animInstance->Montage_Play(SelectedMontage);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to play montage: %s"), *SelectedMontage->GetName());
		return;
	}

	// 재생 후 인스턴스 가져오기
	if (FAnimMontageInstance* MontageInstance = animInstance->GetActiveInstanceForMontage(SelectedMontage))
	{
		// 델리게이트 중복 방지
		MontageInstance->OnMontageEnded.Unbind();
		MontageInstance->OnMontageBlendingOutStarted.Unbind();

		// 델리게이트 바인딩
		MontageInstance->OnMontageEnded.BindUObject(this, &UPakourComponent::ReadyPakour);
		MontageInstance->OnMontageBlendingOutStarted.BindUObject(this, &UPakourComponent::ActiveColNMove);

		UE_LOG(LogTemp, Display, TEXT("Delegates bound for %s"), *SelectedMontage->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get MontageInstance for %s"), *SelectedMontage->GetName());
	}
}


