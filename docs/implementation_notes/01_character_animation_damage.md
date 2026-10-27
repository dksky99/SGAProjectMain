# 1. 캐릭터·입력·애니메이션·상태·부위 피해 구현 설명

## 1.1 이 장을 읽는 기준과 전체 연결

이 장은 2026년 10월 10일 현재 작업 트리의 C++ 구현을 설명한다. 기존 커밋의 코드만이 아니라 수정 중인 CharacterBase, CharacterAnimInstance, HellDiver 및 미추적 AnimLayer·MovementComponent 파일도 함께 읽었다. 목적은 구현을 머릿속에 다시 연결하는 것이다. 따라서 이름만 보고 완성된 기능으로 가정하지 않고, 실제 호출되는 경로, 선언은 있지만 비어 있는 확장 지점, 주석 처리된 구형 경로를 구분한다. Blueprint 클래스의 부모, AnimGraph의 노드 연결, 몽타주의 Notify 배치, DataTable의 실제 행 값은 이 장에서 직접 확인하지 않았다. 아래 설명의 “C++에서 제공한다”와 “실제 에셋에서 연결되어 작동한다”는 서로 다른 수준의 확인이다.

소스 기준 디렉터리는 D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/Source/SGAProjectMain이다. 이후 Character/CharacterBase.cpp:136처럼 표기한 위치는 이 기준 아래의 파일과 한 줄 번호를 의미한다. 줄 번호는 설명 작성 시점의 working tree를 기준으로 하므로 후속 수정 후에는 함수 이름으로 함께 검색하는 것이 좋다.

전체 연결은 입력에서 시작하면 쉽게 이해된다. MainPlayerController가 Enhanced Input Mapping Context를 등록하고, PlayerCharacter가 InputAction을 행동 함수에 연결한다. PlayerCharacter는 화면·상호작용 상태에 따라 입력을 걸러낸 뒤 HellDiver의 행동을 호출한다. HellDiverStateComponent가 행동 가능 여부와 자세·장비·생명 상태를 보관하고, HellDiver가 속도·캡슐·무기·몽타주를 실제로 변경한다. CharacterAnimInstance는 캐릭터 데이터를 읽어 애니메이션용 값으로 만들고, CharacterAnimLayerInstance는 그 값에 맞는 시퀀스와 재생 시간을 선택한다. 공격의 유효 시점은 AnimNotify가 다시 캐릭터에 알려 준다. 최종 피해는 CharacterBase의 TakeDamage에서 부위를 선택한 뒤 StatComponent에서 계산되며, 부위 파괴 이벤트가 적별 특수 반응과 사망으로 이어진다.

읽는 순서는 CharacterBase.h와 HellDiver.h로 소유 관계를 잡고, PlayerCharacter::SetupPlayerInputComponent와 Move를 읽은 뒤 StateComponent를 따라가는 방식이 좋다. 그다음 AnimInstance의 NativeUpdateAnimation, AnimLayer의 Start/Stop/Pivot 함수, CharacterBase::TakeDamage와 StatComponent::ProcessDamage, 마지막으로 Dead/ResetUnit을 읽으면 입력부터 재사용까지 하나의 흐름으로 연결된다.

## 1.2 캐릭터 계층과 생성·초기화 순서

실제 게임용 C++ 계층은 ACharacter → ACharacterBase → AHellDiver → APlayerCharacter이다. 적은 ACharacterBase → AEnemy → 각 Enemy_... 클래스로 내려간다. ACharacterBase는 Unreal의 ACharacter 이동·캡슐·SkeletalMesh 기반 위에 공통 스탯, 상태, 근접 공격, 피격, 래그돌, 타깃 인터페이스를 얹는다. ITargetable은 적 AI가 상대를 타깃으로 다루는 계약이고, IGenericTeamAgentInterface는 팀 구분의 계약이다. 근거는 Character/CharacterBase.h:26, Character/HellDiver/HellDiver.h:18, Character/PlayerCharacter.h:24이다.

ACharacterBase 생성자는 이름이 StatComponent인 UStatComponent를 만든다. AHellDiver는 ObjectInitializer.SetDefaultSubobjectClass<UHellDiverStatComponent>(StatComponentName)으로 같은 기본 서브오브젝트를 플레이어 전용 스탯 타입으로 교체한다. 따라서 공통 코드가 쓰는 _statComponent와 헬다이버 코드가 쓰는 _statComp는 서로 다른 체력 컨테이너가 아니라 같은 서브오브젝트를 다른 타입으로 바라보는 포인터다. 상태도 AHellDiver가 UHellDiverStateComponent를 생성하고 부모의 _stateComp에 연결한다. 이 이중 이름을 이해하지 못하면 HP와 이동 속도 저장소가 두 벌이라고 오해하기 쉽다. HellDiver.cpp:38의 생성자에는 인벤토리, StimPack, Stratagem, Pakour, MotionWarping, 투척 궤적 Spline도 함께 만들어진다.

유닛 수치의 준비는 GameInstance 단계에서 시작된다. UCGameInstance::Init(CGameInstance.cpp:30)는 CachingUnitDataFromTable을 호출하여 유닛 클래스별 FProcessedUnitData를 만든다. CharacterBase::PostInitializeComponents(:82)는 InitUnit(:136)을 호출하고, 현재 액터의 GetClass()를 키로 캐시를 찾는다. 찾은 부위 데이터와 일반·전투 이동 속도는 StatComponent::InitData로 복사되고, 상태 이상 저항 데이터는 StateComponent::InitData로 전달된다. 클래스가 정확히 캐시에 없으면 부모 클래스 데이터를 자동 검색하는 처리는 이 경로에 없다. 새 BP 유닛을 만들 때 데이터 행의 _unitClass와 실제 생성 클래스가 맞는지 확인해야 하는 이유다.

BeginPlay에서는 데이터 이후의 런타임 연결이 이루어진다. CharacterBase::BeginPlay(:96)는 PartInit을 통해 코어 파괴와 사망을 연결하고, 자세 변화 delegate를 애니메이션 인터페이스에 연결한다. 지원하는 AnimInstance가 있으면 _cachedAnimInterface를 저장하고 기본 AnimLayer를 링크한다. AHellDiver::BeginPlay(:61)는 현재 자세의 충돌 형태를 적용하고 자세 변화에 SetCollisionState를 추가 구독한다. APlayerCharacter::PostInitializeComponents(:108)는 HUD 객체들을 만들고, BeginPlay(:132)는 카메라 초기 선택, 아이템 감지 overlap, HUD delegate와 viewport 연결을 수행한다. 부모의 초기화가 먼저 끝나야 자식의 상태·장비·UI 연결이 같은 데이터를 바라볼 수 있다.

프로젝트에 ASGAProjectMainCharacter, TP_WeaponComponent, TP_PickUpComponent도 남아 있다. ASGAProjectMainCharacter는 ACharacter를 직접 상속하는 Unreal 1인칭 템플릿 계열이므로 위 게임 캐릭터 계층의 부모가 아니다. 이 템플릿 파일을 수정해도 HellDiver의 이동이나 피해 처리가 바뀌지 않는다. 실제 맵이 어떤 Pawn BP를 생성하는지는 GameMode 설정과 에셋을 함께 봐야 한다.

## 1.3 입력은 어디서 걸러지고 카메라는 어떻게 목표점을 만드는가

MainPlayerController::BeginPlay(Controller/MainPlayerController.cpp:25)는 LocalPlayer의 EnhancedInputLocalPlayerSubsystem에 _inputMappingContext를 우선순위 0으로 추가한다. 실제 키와 InputAction의 관계는 이 에셋에 있고, 행동 함수를 선택하는 코드는 PlayerCharacter::SetupPlayerInputComponent(Character/PlayerCharacter.cpp:234)에 있다. 이동은 Started/Triggered/Completed를 각각 MoveStart/Move/MoveFinish로 나눈다. 마우스 좌·우 버튼도 눌림·유지·해제의 세 단계다. 재장전은 누름과 해제로 나누어 짧게 누르면 Reload, 일정 시간 유지하면 총 설정 UI로 들어가는 방식이다. 웅크리기·엎드리기·회피·근접 공격은 주로 Started에 연결한다. JumpAction은 이 클래스에서 Jump를 직접 부르는 대신 TryPakour로 연결된다.

핵심은 입력 함수가 단순 forwarding이 아니라 현재 모드를 해석한다는 점이다. Move(:521)는 스트라타젬 입력, 터미널 상호작용, 공중 이동, 행동 불가, 회피, 정지 재장전이면 이동을 거부한다. StartFiring(:650)은 지도 모드라면 먼저 핑을 시도하고, 상호작용 중이면 사격하지 않으며, 달리는 중이면 달리기를 끝낸다. 이어 장비 타입이 총인지 수류탄인지 장치인지에 따라 다른 동작을 선택한다. 그러므로 같은 마우스 클릭도 화면 상태와 장비 상태의 조합에 따라 의미가 바뀐다. 입력 변경 시 PlayerCharacter만 보는 것이 아니라 EActionState와 EWeaponType 사용처를 같이 읽어야 한다.

카메라 구성은 캐릭터에 붙은 카메라 컴포넌트 하나가 아니다. PlayerCharacter 생성자(:64)는 CameraRoot, TPS/확대 TPS/FPS용 SpringArm 세 개, CameraContainActor를 생성하는 ChildActorComponent 세 개를 만든다. TPS 계열은 CameraRoot에, FPS SpringArm은 Mesh의 head에 붙는다. SetDefaultVIew(:1038)는 컨트롤러 회전과 SpringArm 상속 설정을 정한다. SetTPSView/SetTPSZoomView/SetFPSView(:1112 이후)는 ECharacterViewType을 바꾸고 ChangeViewCamera(:1369)로 들어간다. 이 함수는 원하는 ChildActor를 ViewTarget으로 블렌딩하고 다른 카메라를 비활성화한다. 카메라 에셋 참조가 없으면 선택 함수가 일찍 반환하므로 BP 기본값도 중요하다.

실제 조준 목표의 월드 좌표는 GetCenterLoc(:1493)에서 계산된다. PlayerController가 있으면 화면 중앙을 DeprojectScreenPositionToWorld로 월드 레이로 바꾸고, 10000 Unreal 단위 길이의 Visibility trace를 한다. 맞으면 ImpactPoint, 맞지 않으면 레이 끝점을 목표로 삼는다. 컨트롤러가 없는 경우 현재 카메라 컴포넌트의 위치·방향으로 fallback한다. 이 값은 PlayerCharacter::Tick(:197)과 CalcFocusingLoc(:1704)을 통해 _focusingLoc에 저장된다. 공통 CharacterBase는 앞쪽 200 단위 지점을 기본 목표로 쓰지만 플레이어는 화면 중앙으로 대체한다.

이 목표점은 몸과 무기를 어디로 향하게 할지 결정하는 입력 데이터다. 발사체 자체는 무기의 Muzzle 소켓에서 출발하므로 화면 레이와 총구 레이는 시작점이 다르다. 가까운 벽 옆에서 조준점과 탄착이 어긋나는 현상을 이해하려면 카메라 trace, 몸의 aim 보정, 총구의 실제 transform을 차례로 확인해야 한다. PlayerCharacter의 Focusing/CalcYaw/CalcPitch와 HellDiverAnimInstance의 구형 aim 계산도 남아 있으므로 어느 계산이 AnimGraph에 연결되었는지는 BP 확인 대상이다.

## 1.4 이동·자세·스태미나·회피는 서로 다른 층에서 처리된다

Move는 IsFocusing 결과에 따라 FocusMove(:1157) 또는 DefaultMove(:1177)를 선택한다. FocusMove는 Controller yaw에서 앞·오른쪽 축을 구하고 입력 Y/X를 각각 AddMovementInput에 전달한다. 몸 방향을 조준 쪽에 유지하면서 전후좌우로 움직이는 strafing 경로다. DefaultMove도 카메라 yaw를 기준으로 이동 방향을 만들지만 bOrientRotationToMovement를 켜서 몸이 이동 방향으로 돌아가게 한다. 이때 _vertical/_horizontal에는 액터 로컬 방향을 기록한다. 이 필드들은 물리 위치를 직접 바꾸는 값이 아니라 애니메이션과 회피 방향 계산에 쓰는 입력 기록이다. MoveFinish(:571)는 두 값을 0으로 만들고 필요하면 ViewTurnBack(:1592)으로 시선 복귀 회전을 준비한다.

자세 전환은 입력 → 상태 승인 → 물리 설정의 순서다. TryCrouch/TryProne이 현재 자세를 보고 시작 또는 해제를 선택하고, AHellDiver::StartCrouch/StartProne(:485/:500)가 StateComponent의 Start...를 먼저 호출한다. 승인되면 Standing/Sprinting/Crouching/Proning(:553 이후)에서 충돌 형태와 MaxWalkSpeed를 바꾼다. 상태 변화 delegate에 연결된 SetCollisionState(:1241)도 캡슐 변경을 수행한다. 웅크리기 시작은 ACharacter의 Crouch를 호출하는 반면 종료는 자체 Standing 경로를 사용한다. Unreal 기본 crouch 상태와 자체 ECharacterState를 같은 개념으로 단정하지 않는 것이 좋다.

충돌·카메라 설정은 UCollisionCameraDataAsset 세 개(_standingStance/_crouchingStance/_proningStance)를 기준으로 한다. HellDiver의 SetStandingCollisionCamera(:1271) 등은 캡슐 반지름·반높이와 Mesh의 상대 Z를 바꾼다. PlayerCharacter override(:1601/:1624/:1644)는 부모 호출 후 CameraRoot를 자세별 _cameraOffset으로 0.2초 동안 이동시킨다. 따라서 자세 데이터의 반높이를 수정하면 충돌뿐 아니라 메시 원점과 카메라 높이도 함께 검증해야 한다. 통합 이름의 SetCollisionCamera(:1265)는 현재 비어 있으며 실제 경로는 자세별 함수다.

플레이어 속도는 UHellDiverStatComponent가 별도 필드로 보관한다. 기본값은 걷기 500, 달리기 700, 웅크리기 300, 엎드리기 200이고 스태미나는 300이다(HellDiverStatComponent.h). 부모 UStatComponent의 DataTable 기반 일반/전투 속도와 자식의 _defaultSpeed는 다른 필드다. GetDefaultSpeed도 부모 함수가 virtual이 아닌 상태에서 자식이 같은 이름으로 제공하므로 호출 포인터의 타입을 확인해야 한다. “테이블 속도를 바꿨는데 플레이어 속도가 그대로”인 경우 이 저장소 차이가 원인 후보다.

AHellDiver::Tick(:92)은 Sprinting이면 deltaTime×50만큼 스태미나를 소비하고 0이면 FinishSprint한다. 그 외에는 재생 대기 시간이 아니면 deltaTime×70을 회복한다. FinishSprint(:472 부근)는 서 있는 자세로 바꾸고 2초 timer 동안 재생을 막는다. 스태미나 delegate는 현재/최대의 비율과 회복 여부를 보내며 PlayerCharacter가 이를 StaminaBar에 연결한다. 수치의 의미는 HP와 달리 화면 비율이다.

Rolling(:514)은 일반적인 구르기 root motion만으로 이동하는 구현이 아니다. CanJump와 상태를 검사한 뒤 Jump, StartProne을 수행하고 입력 기록으로 도약 방향을 만든다. 입력이 없으면 액터 정면으로, 입력이 있으면 액터의 앞/오른쪽 축 조합으로 방향을 만든 뒤 회전시키고 Velocity에 전방 500을 더한다. 결과적으로 공중 도약 후 엎드리기로 이어지는 dive에 가깝다. 착지에서 _isRolling 해제를 처리하므로 입력뿐 아니라 Landed까지 함께 읽어야 한다. TryPakour 입력은 Standing/Sprinting/Crouching에서 벽 넘기를 시도하고, 이미 엎드린 경우에는 일어나도록 한다.

## 1.5 상태 저장소: 자세 enum, 입력 모드, 생명 상태와 잠금

ECharacterState는 Standing, Crouching, Proning, Sprinting, Knockdown을 나타낸다(CharacterStateComponent.h:39). EActionState는 None, Stratagem, ViewMap, InterActing이며 이동 자세와 독립적으로 입력의 의미를 바꾼다. EWeaponType은 None/Gun/Grenade/StratagemDevice이고, ELifeState는 Alive/Downed/Dead/Respawning이다(HellDiverStateComponent.h:9 이후). 이들은 하나의 거대한 상태 enum을 대신하는 여러 축이다. 예를 들어 “엎드린 채 총을 들고 지도 보기”는 자세·장비·행동 모드의 조합으로 표현된다.

추가 bool은 _isFiring, _isReloading, _isRolling, _isAiming, _isCookingGrenade, _isWeaponChange이고 부모에 _isActing/_isMotionChange가 있다. IsActionable(HellDiverStateComponent.cpp:276)은 Alive, Knockdown 아님, 강한 행동 불가 상태 이상 없음의 조건이다. IsFocusing(:227)은 행동 가능한 상태에서 행동 모드 None, 재장전/무기 교체/공통 action 없음인지 확인하고, 사격 또는 조준이면 true, 평범하게 이동 중이면 false, 서서 가만히 있는 경우에도 true로 판정한다. 따라서 Focusing은 우클릭 bool의 동의어가 아니라 상체와 시선 보정을 적용할 수 있는 상황 판정이다.

공통 ActionBegin(CharacterStateComponent.cpp:206)은 이미 행동 중이면 false를 반환하고, 아니면 avoidance를 끈 후 _isActing을 true로 만든다. 근접 공격, 투척, 일부 적 행동, 스폰 몽타주가 이 잠금을 공유한다. ActionEnd(CharacterBase.cpp:804)는 근접 충돌을 해제하고 _isActing을 false로 돌리며 _reservedFunction을 해제하고 진행 중 몽타주를 중단한다. 즉 ActionEnd는 bool 하나를 내리는 API보다 넓은 정리 작업이다. 변경할 때 공격 충돌·예약 콜백·몽타주 세 가지 영향이 같이 생긴다.

자세 전환의 TryMotionChange와 무기 전환의 TryWeaponChange(HellDiverStateComponent.cpp:356/:366)는 현재 항상 false를 반환하며 기존 잠금 코드는 주석 상태다. StartSprint 등의 호출부는 true면 전환 거부로 해석하므로 현재 이 잠금은 사실상 통과한다. IsMovable(:292)은 마지막에도 false를 반환하고 C++ 호출 경로를 찾지 못했다. UHellDiverState UObject의 Move/Look/Fire 등도 비어 있다. 따라서 별도 state-pattern 객체가 실제 행동을 주도한다고 설명하면 안 된다. 현재 동작의 중심은 StateComponent와 캐릭터 함수들이다.

모션 종료를 StateComponent로 전달하려는 _motionFinished → RecieveMotionFinish 연결은 CharacterBase.cpp:129에 있다. 대상 함수의 헤더 선언(:129)에 UFUNCTION이 없고, 소스 전체에서 _motionFinished.Broadcast 호출도 찾지 못했다. FinishStateTransition(CharacterAnimInstance.cpp:438)은 비어 있다. 이 경로는 현재 미완성 연결이다. ActionState/WeaponState/LifeState delegate도 선언되어 있으나 단순 setter는 필드 대입만 한다. 외부에서 이벤트를 구독했다고 값 변경마다 통지된다고 가정하면 안 된다.

## 1.6 상태 이상은 활성 비트와 누적치·시간 Map을 함께 사용한다

EAbnormality는 Fire/Burn/Gas/AcidBubble/AcidStream/bleeding/Thornbush/LightStagger/StrongStagger/Shock의 순서 인덱스다(Object/AbnormalityTable.h). EAbnormalityState는 같은 항목을 1<<인덱스로 표현하는 uint32 비트 enum이다. StateComponent의 _activeAbnormalities는 실제 발현한 상태, _activeAbnormalitiesWeight는 감소 중인 누적치를 가진 상태다. _remainTimes와 _remainWeights는 각 비트에 연결된 남은 시간과 누적치다. 두 비트셋을 나눈 이유는 효과가 아직 발현하지 않았어도 누적치 감소는 관리해야 하기 때문이다.

AddAbnormality(CharacterStateComponent.cpp:59)는 GameInstance에서 FProcessedAbnormalityDefinitionData를 얻고, 해당 항목 누적치에 _stateWeight를 더해 최대 저항치까지 Clamp한다. Fire/Gas/Shock 계열은 FUnitAbnormalResistData의 min/max 임계치를 사용한다. 누적치가 min보다 엄격하게 커지면 지속시간을 _remainTimes에 넣고 활성 비트를 세운다. TickComponent(:48)는 누적치를 먼저 감소시킨 다음 지속시간을 감소시킨다. 비트 순회는 최하위 1비트를 꺼내며 진행하고, CalcActivates(:311)는 prev×4와 cur×4의 정수 구간이 바뀌면 피해를 한 번 적용한다. 설계 의도는 0.25초마다 한 번인 지속 피해다. 긴 프레임에서 여러 구간을 넘어도 한 번만 호출하는 현재 구현의 성질도 기억해야 한다.

지속 피해는 별도의 HP 감소 공식을 쓰지 않는다. CalcDamage(:337)가 캐시된 FCDamageEvent를 얻어 _owner->TakeDamage(0,*event,nullptr,nullptr)로 다시 일반 부위 피해 파이프라인에 들어간다. 그러므로 상태 이상도 관통력·내구도·코어 선택의 영향을 받는다. UCDamageType의 _abnormalityType이 Max가 아니면 TakeDamage에서 AddAbnormality를 호출하므로 지속 피해 이벤트의 DamageType 설계에 따라 재적용도 일어날 수 있다. 데이터의 상태 부여와 tick 피해의 의미를 함께 확인해야 한다.

IsSlow(:228)는 산성 두 종류, 가스, 덤불, 약한 stagger 중 하나가 활성인지 검사한다. IsUnable(:222)은 Shock 또는 StrongStagger 비트다. ActiveSlow는 현재 MaxWalkSpeed를 ChangeSpeed에 전달하고, ChangeSpeed가 slow라면 0.75를 곱한다. ActiveUnable는 몽타주와 이동을 중지하는 UnitUnable로 이어지고, PlayerCharacter override(:1682)는 컨트롤러 입력도 비활성화한다. 복구는 UnitRecoverFromUnable(:1693)에서 입력을 다시 켠다. CheckStagger(:366)는 강한 임계치를 넘으면 StrongStagger와 KnockBack을, 약한 임계치를 넘으면 WeakStagger도 수행한다. 두 if가 독립적이므로 강한 충격은 약한 충격 경로까지 들어갈 수 있다.

현재 동작을 해석할 때는 세 가지 구현 상태를 알고 있어야 한다. AddAbnormality의 Tick 켜기 조건(:103)은 반대로 작성되어 있고 HellDiverStateComponent 생성자는 bCanEverTick=false이다. 지속시간의 FindOrAdd(:115)는 기존 키의 값을 갱신하지 않는다. 또한 prevSlow를 비트 변경 후에 읽고 활성 때마다 감속을 재적용하므로 실제 속도가 누적 감소할 수 있다. 이 부분들은 이 장 작성 중 고치지 않았으며, 설계 의도와 코드의 현재 결과를 분리하여 설명하는 이유다.

## 1.7 AnimInstance는 데이터 허브, AnimLayer는 시퀀스 선택기다

UCharacterAnimInstance는 캐릭터의 주 AnimInstance이며 IAnimInstanceInterface를 구현한다. NativeInitializeAnimation(:20)은 Pawn을 캐시하고 NativeUpdateAnimation(:29)은 캐릭터 속도, 낙하, 공통 행동 플래그, 컨트롤러·액터 회전, 가속도, 현재/이전 위치, 시선 목표 등을 읽는다. _targetPos_World를 Mesh transform으로 역변환한 _targetPos_Mesh도 여기서 만든다. _incommingState는 캐릭터 상태 변화 통지를 통해 들어오고, GetCharacterState(:288)가 이전/현재 상태 및 _bIsStateChanged를 만든다. 공중 여부와 Z velocity 부호로 Jumping/Falling을 나누고, 중력과 Z 속도로 apex까지의 시간 값을 계산한다.

NativeThreadSafeUpdateAnimation(:81)은 저장된 속도·가속도·위치·회전에서 평면 속도, 방향, RootYawOffset 등을 계산한다. 구조상 게임 스레드에서 실제 캐릭터 정보를 모으고 worker에서 애니메이션용 계산을 하려는 분리다. 다만 현재 GetRotationData(:193)는 GetAimBone을 통해 실제 Mesh bone transform에 접근하며, AnimLayer에서도 MovementComponent의 마찰값을 직접 읽는 부분이 있다. “ThreadSafe 이름이 붙었으니 전부 안전하다”는 가정은 피해야 한다. 캐시 값만 계산하는 함수와 UObject/물리 데이터를 읽거나 쓰는 함수를 따로 확인해야 한다.

IAnimInstanceInterface(Interface/AnimInstanceInterface.h)는 RecieveGroundDistance와 RecieveCharacterStateChange 두 함수의 순수 C++ 계약이다. CharacterBase는 지원 AnimInstance를 TScriptInterface로 캐시하고, 낙하 중 바닥까지 거리를 계산해 전달한다. CalcGroundDistance(:201)는 캡슐 하단에서 아래 300 단위로 반지름 5 sphere sweep을 하고 hit distance를 보낸다. 이 값은 AnimLayer 착지 distance matching의 입력이다. 지원 인터페이스만 BP에서 표시한다고 임의의 BP 전용 구현을 호출하는 UFUNCTION 계약은 아니며, 현재 BeginPlay도 UCharacterAnimInstance로 먼저 cast한다.

UCharacterAnimLayerInstance는 그 주 인스턴스의 값들을 바탕으로 자세·방향별 시퀀스를 선택한다. NativeInitializeAnimation은 SetBaseAnim(:21)으로 OwningComponent의 AnimInstance를 UCharacterAnimInstance에 cast해 _baseAnimInst로 보관한다. GetBaseAnim(:30)은 캐시가 없으면 다시 구한다. CharacterBase::ChangeAnimLayer(:255)는 Mesh->LinkAnimClassLayers를 호출한다. AHellDiver::TryChangeLayer(:1143)는 슬롯 0/2에서 rifle, 1에서 pistol, 3에서 default layer 클래스를 선택한다. 즉 무기별 움직임·자세 표현을 주 상태 로직과 분리하려는 구조다. 실제 Linked Anim Layer interface와 노드가 BP에서 맞게 구성되었는지는 아직 확인하지 않았다.

UHellDiverAnimLayerInstance와 UHelldiverMovementComponent는 현재 상속 선언만 있고 cpp에도 별도 동작이 없다. 특히 HellDiver 생성자에는 CharacterMovement 서브오브젝트를 UHelldiverMovementComponent로 교체하는 코드가 없다. 따라서 커스텀 MovementComponent에 구현된 새로운 이동 시스템이 이미 주 경로라는 설명은 맞지 않는다. 현재 이동은 기존 UCharacterMovementComponent를 조작하는 캐릭터 함수가 주도하고, 새 AnimLayer 코드가 그 결과를 읽는 작업 단계다.

## 1.8 방향·거리 매칭·제자리 회전의 애니메이션 계산

ELocomotionDirections는 Forward/Backward/Right/Left다. UpdateOrientationData(CharacterAnimInstance.cpp:222)는 velocity와 acceleration을 액터 회전에 대해 CalculateDirection으로 각도로 만들고, CalculateLocomotionDirection(:237)에서 네 방향으로 분류한다. 기본 전방 범위는 -50~50도, 후방은 ±130도 바깥이며 현재 방향을 유지하는 dead zone 20도가 추가된다. 이 완충 범위는 입력 각도가 경계 근처에서 흔들릴 때 시퀀스를 매 프레임 바꾸지 않기 위한 hysteresis다. 속도 방향과 가속 방향을 따로 보관하므로 현재 이동과 앞으로 의도한 이동을 구분할 수 있다.

FDirectionalAnimations(CharacterAnimLayerInstance.h:12)는 네 방향 UAnimSequenceBase 포인터를 한 세트로 묶는다. 걷기/달리기/웅크리기/엎드리기의 idle, cycle, start, stop, pivot 및 dodge 세트가 필드로 분리되어 있다. IdleOnUpdate(:39)와 CycleOnUpdate(:70)는 자세에 맞는 세트에서 방향별 애니메이션을 골라 SequencePlayer에 넣고 0.2초 inertial blending을 사용한다. Prone idle과 dodge는 GetProneDirection으로 몸과 조준의 상대 yaw를 네 방향으로 나눈 결과를 사용한다. 이 자료형의 네 포인터는 현재 명시적으로 초기화되지 않았고 과거 로그의 struct validation 오류와 연결된다. 에셋을 새로 연결할 때 기본 null과 필수 시퀀스 누락을 함께 검증해야 한다.

Start/Stop/Pivot는 SequenceEvaluator를 주로 쓴다. SetupStartAnims(:196)는 시작 시퀀스를 선택하고 explicit time을 0으로 설정한다. UpdateStartAnims(:233)는 이동 방향이 바뀌면 시퀀스와 시간을 다시 맞추고, _deltaLocation을 Distance curve와 비교하여 AdvanceTimeByDistanceMatching으로 시간을 진행한다. 같은 속도의 재생률을 고정하는 대신 실제 이동 거리에 발 동작을 맞추려는 방식이다. _deltaLocation은 두 월드 위치 차이의 길이이므로 이동 거리 데이터의 수집 시점도 중요하다.

UpdateStopAnims(:171)는 현재 평면 velocity와 CharacterMovement의 braking 설정으로 예상 정지 거리를 계산하고 DistanceMatchToTarget을 사용한다. Pivot(:321)는 velocity와 acceleration의 내적이 음수면 반대 방향 전환 중이라고 보고 예상 pivot 지점을 이용한다. Jump/Fall/Land(:455)는 _groundDistance로 타깃 시간을 맞추고, DodgeStart/End는 distance matching, DodgeLoop는 SequencePlayer로 반복 구간을 재생한다. 자세 전환 SetupStanceTransitionAnims(:550)는 _lastFrameState와 _incommingState 조합으로 stand↔crouch↔prone 시퀀스를 선택한다. 이를 고칠 때는 C++ 선택표뿐 아니라 Distance curve, evaluator/player 노드 타입, 상태 진입 callback 연결을 같이 읽어야 한다.

제자리 회전은 RootYawOffset과 IsTurning/root_rotation_Z curve를 함께 사용한다. UpdateRootYawOffset(:335)은 Accumulate면 yaw 차이를 누적하고 BlendOut이면 보간해 0으로 돌린다. UpdateIdleState(:356)는 accumulate 모드를 설정하고 ProcessTurnYawCurve(:362)를 호출한다. curve의 프레임 차이만큼 offset을 보정하고 마지막에는 Pawn을 회전시키는 코드가 :397/:415에 있다. 두 함수는 BlueprintThreadSafe로 선언되어 있는데 액터 회전은 물리 transform 변경으로 이어진다. 2026년 5월 20일 과거 로그의 스레드 ensure는 이 경로를 직접 지목했다. 구현 의도는 시각적인 허리 offset을 회전 애니메이션과 동기화하는 것이지만 실제 Pawn 적용은 게임 스레드로 옮길 대상이다. 과거 로그는 오늘 재현 결과가 아니다.

## 1.9 헬다이버 전용 상체 조준과 무기 좌표계

UHellDiverAnimInstance는 공통 AnimInstance를 상속하고 IHelldiverAnimInterface를 구현한다. NativeUpdateAnimation(HellDiverAnimInstance.cpp:33)은 먼저 부모의 데이터를 갱신한 후 자세/행동/장비/생명 상태, 사격·재장전·회피·집중 bool, 현재 무기, 총구 transform, 오른손 기준 왼손 grip transform 등을 추가 수집한다. CheckEquipChange(:227)는 장비 인덱스가 달라진 프레임에 _changeWeapon을 true로 한다. IsUsingLeftHand(:181)는 살아 있고 action mode가 없으며 공통 행동 중이 아니고 총을 들었을 때 왼손 IK 사용을 허용한다. IsUsingFocusing은 무기 교체·재장전·불안정 상체 상태 등을 추가로 배제한다.

무기 부착과 IK는 서로 다른 좌표계가 연결되는 곳이다. HellDiver::GetLeftHandSocketTransform(:1019)는 무기의 LeftGrip 월드 transform을 캐릭터 hand_r bone space로 변환하고 위치에 작은 보정값을 더한다. 무기를 쥔 오른손 기준으로 왼손이 어디에 있어야 하는지 AnimGraph에 전달하려는 값이다. GunBase::CalculateMuzzleToHand_R는 hand_r에 대한 무기 socket의 상대 transform과 무기 root에 대한 Muzzle transform을 합쳐 IHelldiverAnimInterface::SetWeaponOffset으로 전달한다. 이 때문에 hand_r, cc_weaponbone_r_socket, Muzzle, LeftGrip 소켓의 이름과 실제 축이 맞아야 한다. 다른 스켈레톤으로 리타깃할 때 C++ 로직이 같아도 보정 결과는 달라질 수 있다.

AimFocus(:137)는 _useFocusing이면 캐릭터의 Focusing 회전 결과와 DeltaTime을 이용해 _focusRotate를 누적하고 _focusAlpha를 0~1로 증가시킨다. 비활성 상태에서는 회전과 alpha를 원상태로 보간한다. 반면 GetAimOffset(:248)은 actor yaw 변화와 CalcYaw/CalcPitch, 총구 방향 추가 보정으로 _finalYaw/_finalPitch를 만드는 구형 함수이고 NativeUpdateAnimation의 호출이 주석 처리되어 있다. GetCurrentMoveNode를 읽어 문자열 Move/Look 상태를 추적하는 경로도 현재 호출이 주석이다. 기존 MoveChangeFinish/LookChangeFinish delegate 연결 또한 HellDiver::BeginPlay에서 주석 처리되어 있다. 이 신구 경로를 모두 현재 활성화된 시스템처럼 읽으면 상태 잠금과 조준 계산을 혼동하게 된다.

IHelldiverAnimInterface의 SetWeaponOffset은 실제 구현이 있어 _weaponOffset과 _bUseAim을 저장한다. SetLeftHandLocation(HellDiverAnimInstance.cpp:452)은 현재 false만 반환한다. 따라서 파쿠르가 왼손 목표를 계산하는 코드와 손 IK가 실제로 연결되었다고 단정할 수 없다. 인터페이스 선언, 이를 호출하는 코드, 값을 받는 구현, BP의 IK node를 네 단계로 확인하는 것이 변경 영향 분석에 필요하다.

## 1.10 파쿠르: 탐색 결과를 몽타주와 MotionWarping으로 바꾸는 과정

UPakourComponent는 벽 넘기 판단과 실행을 분리한 컴포넌트다. BeginPlay에서 Initialize로 캐릭터, 캡슐, Mesh, Movement, MotionWarping 포인터를 캐시한다. TriggerPakour(PakourComponent.cpp:70)는 _canPakour를 검사하고 DetectWall → ScanWall → MeasureWall → TryPakour 순서로 처리한다. DetectWall(:89)은 캐릭터 앞의 여러 높이에 sphere sweep을 하여 벽을 찾는다. ScanWall(:125)은 벽 표면, 첫 상단·마지막 상단, 벽 끝, 반대편 착지점에 해당하는 HitResult를 만든다. MeasureWall(:259)은 상단 hit와 Mesh 위치의 Z 차이로 높이를 계산한다. 여기서 기준은 캡슐 중심이 아니라 Mesh 위치이므로 자세별 Mesh offset 변경과 연결된다.

TryPakour(:289 부근)는 지면에 있고 첫 상단 hit가 존재해야 진행한다. 높이가 180을 넘으면 거부, 150을 넘으면 HighMantle, MaxStepHeight보다 낮으면 일반 보행에 맡긴다. 그 외에는 벽 상단 두 점 사이 거리로 두께를 재고 30 이하면 Vault, 더 두꺼우면 LowMantle을 선택한다. 여러 OneHandVault/TwoHandVault/FrontFlip 함수가 있지만 이 자동 분류 경로가 직접 선택하는 타입은 Vault/LowMantle/HighMantle이다. 다른 종류는 확장 가능한 선택지로 이해하는 편이 정확하다.

TryVault(:341)는 _canPakour=false, MovementMode=Flying, 캡슐 collision off, StateComponent::StartPakour로 공통 action 잠금을 설정한 후 선택 몽타주를 재생한다. Flying은 이 구간에 일반 걷기·중력 이동이 간섭하지 않게 하려는 설정이고, 실제 공간 이동은 root motion과 MotionWarping target에 맞춘 몽타주가 담당하는 구조다. Vault/LowMantle/HighMantle 함수는 VaultEnd라는 warp target을 계산하며 일부 변형은 VaultStart도 설정한다. 몽타주 안의 MotionWarping Notify가 이 이름을 참조해야 실제 목적지 보정이 이루어진다. 그 에셋 내부 연결은 미확인이다.

OnMontageBlendingOutStarted는 ActiveColNMove(:45)로 연결되어 Walking과 캡슐 collision을 먼저 복구한다. OnMontageEnded는 ReadyPakour(:38)로 연결되어 _isActing과 _canPakour를 복구한다. 이동·충돌 복구와 다음 파쿠르 허용을 다른 시점에 나누었다는 점이 핵심이다. 실패 경로에서는 잠근 뒤 anim instance나 montage가 없어 반환할 수 있고, ScanWall은 이전 HitResult를 초기화하지 않는 경우가 있다. 실제 작업을 이어갈 때는 이 탐색/실행 계약과 성공·중단·실패 세 가지 정리 경로를 함께 확인해야 한다.

UpdateHandIKLocation(:518)은 왼손 주변에서 벽과 상단을 sphere trace하여 손 위치를 구하는 보조 함수다. 현재 끝에서는 handLocation 로컬 변수 계산까지만 하고 캐릭터/AnimInstance에 전달하지 않는다. TickComponent도 비어 있으므로 자동 매 프레임 호출이 이루어진다고 설명할 수 없다. 코드에 존재하는 계산과 실제 구동되는 애니메이션 기능을 구분할 대표적인 부분이다.

## 1.11 근접 공격·예약 함수·AnimNotify의 왕복 연결

UUnitAttackDataAsset(Data/UnitAttackDataAsset.h)은 공격 Motion 몽타주와 Attack/DurabilityAttack/DemolitionAttack, PenetrationLevel, Stagger, PushForce, ActiveColliders 이름 목록, DamageType 등을 하나로 묶는다. 공통 CharacterBase::AttackMelee(:782)는 공격 자료 목록에서 하나를 골라 ActionBegin으로 행동 잠금을 얻고 몽타주를 재생한 뒤 _curAttackData와 활성화할 충돌 목록을 정한다. HellDiver::AttackMelee(:418)는 무기가 총이면 양손 공격 자료, 그 외면 한손 공격 자료를 선택한다. Interval/bCanMove/Effect 필드도 자료형에는 있지만 이 공통 함수에서 전부 소비되는 것은 아니다.

_meleeColliders는 이름→UShapeComponent의 Map이고 _activateColliders는 이번 공격에 사용할 목록이다. SetMeleeColisions(:832)는 DataAsset의 ActiveColliders 이름으로 컴포넌트를 찾아 목록에 넣는다. InitMeleeColliders(:733)는 collision을 끈 상태로 설정하고 Pawn overlap에 OnWeaponOverlap을 연결한다. 실제 검출 구간은 UANS_MeleeCollision::NotifyBegin(AnimNotify/ANS_MeleeCollision.cpp:12)과 NotifyEnd(:26)가 열고 닫는다. 이 NotifyState가 몽타주의 어느 구간에 배치되었는지는 에셋 확인 대상이다.

OnWeaponOverlap(CharacterBase.cpp:746)은 자신과 이미 맞은 액터를 제외하고 AttackDataToDamageEvent(:158)로 커스텀 피해를 만든 뒤 상대의 TakeDamage를 호출한다. _hitted 배열은 한 공격 창에서 여러 충돌체가 같은 상대를 중복 때리는 것을 막는다. DeactivateMeleeColision은 collision을 끄면서 이 목록을 비우므로 다음 창에서는 다시 맞을 수 있다. 이 구조를 바꾸면 “한 몽타주에 여러 타격 창”과 “한 창의 중복 검출” 정책을 같이 검토해야 한다.

UAN_CallFunction(AnimNotify/AN_CallFunction.cpp:14)은 CharacterBase::_reservedFunction을 실행하는 범용 단일 시점 Notify다. 투척에서는 Throwing(HellDiver.cpp:976)이 OnThrowReleased를 예약하고, Notify가 실제 손에서 물체를 놓는 시점을 결정한다. 적의 증원 호출·잠복 같은 동작도 같은 슬롯에 다른 함수를 예약한다. UAN_AttackEnd(AnimNotify/AN_AttackEnd.cpp:14)는 ActionEnd를 호출하여 행동 잠금·충돌·예약 함수·몽타주를 정리한다. 따라서 몽타주의 이벤트와 gameplay 처리 시점을 맞추는 방식이다. UANS_CallFunctionInterval은 begin/end/tick이 모두 비어 있어 반복 호출 기능의 뼈대만 존재한다.

한 캐릭터당 _reservedFunction은 하나이므로 동시에 서로 다른 행동이 이를 예약하면 나중 bind가 이전 의미를 덮어쓴다. ActionBegin은 이 충돌을 제한하려는 공통 장치다. 공격 수정 시 읽는 순서는 DataAsset → AttackMelee → SetMeleeColisions → NotifyState → OnWeaponOverlap → TakeDamage → AttackEnd가 적절하다.

## 1.12 유닛·부위 데이터와 장갑·내구도 피해 계산

FUnitData(Character/UnitDataTable.h:125)는 유닛 이름/클래스/속도/저항과 _PartIDs Map을 가진다. _PartIDs의 키는 EBodyPart, 값은 부위 정의 테이블의 행 이름이다. FPartDefinitionRow는 Layers 배열을 가지며 각 FUnitPartLayerData에는 LayerName, 최대 HP, AV, Durability, Influence, ExplosionImmunity가 있다. FUnitPartStat(StatComponent.h:25)은 이 수치에 _curHP와 파괴/복구 delegate를 추가한 런타임 형태다. 배열을 struct wrapper로 감싸고 Map에 넣은 형태는 부위마다 레이어 수가 달라도 표현하기 위한 것이다.

CachingUnitDataFromTable(CGameInstance.cpp:73)은 UnitTable의 각 행을 읽고 _PartIDs를 따라 PartDefinitionTable을 조회한다. 각 Layers 원소를 FUnitPartStat로 바꾸고 HP를 최대치로 채운 뒤 _processedUnitDataMap에 유닛 클래스 키로 저장한다. InitUnit이 이 캐시를 복사하므로 같은 적 클래스 두 인스턴스의 현재 HP는 서로 독립적이다. 런타임 _onPartDestroyed 연결은 캐시 단계가 아니라 인스턴스의 PartInit에서 붙는다. 새 유닛이나 레이어를 추가할 때는 DataTable 한 곳만이 아니라 클래스 키, 부위 이름, 필요한 배열 길이를 함께 맞춰야 한다.

피격 부위는 CharacterBase::PartTagMap(:45)에 정의된 Core/Head/Torso/Tail/LeftArm/RightArm/LeftLeg/RightLeg/LeftClaw/RightClaw 태그로 구분한다. GetHittedPart(:869)는 FCDamageEvent::ColComp의 ComponentTags를 검사하고 첫 유효 부위를 고르며 없으면 Core다. GetHittedPartStat(:903)는 기본적으로 해당 부위의 첫 레이어를 반환한다. 같은 EBodyPart 안의 레이어 선택은 일부 적의 override에 맡긴다. GetPartArmorValue(:1068)는 관통 판정용 AV를 얻지만 현재는 항상 첫 레이어를 읽는다. 피해 대상 레이어 선택과 관통 전 AV 조회가 같은 규칙인지 확인할 필요가 있다.

FCDamageEvent(Object/CDamageType.h:28)는 FPointDamageEvent를 상속하지만 별도 ClassID 1111을 사용한다. 기본/내구/철거 피해, 관통력, stagger, push, 폭발 여부, 피격 컴포넌트를 함께 전달한다. UCDamageType은 Default/Acid/Fire/Gas/Bleeding/Arc라는 속성과 부여할 상태 이상을 제공한다. CharacterBase::TakeDamage(:922)는 먼저 AI damage perception 이벤트를 알린 뒤 커스텀 ClassID이면 상태 이상·stagger를 처리하고, 태그로 부위와 실제 레이어를 선택하여 StatComponent::ProcessDamage에 넘긴다. 폭발 면역 1인 부위에 폭발이면 대상 부위를 코어로 바꾸는 특별 분기도 있다. DamageAmount 인자보다 커스텀 이벤트 내부 값이 실제 계산의 중심이다.

ProcessDamage(StatComponent.cpp:204)의 계산은 다음 순서다. AV와 AP가 같으면 장갑 배율 0.65, AV가 AP보다 크면 0, AP가 더 크면 1이다. 폭발이면 여기에 (1−ExplosionImmunity)를 곱한다. 유효 피해는 이 배율에 [Durability×DurabilityDamage + (1−Durability)×BaseDamage]를 곱한 값이다. 즉 Durability는 추가 HP나 피해 감소율이 아니라 두 피해 종류를 섞는 가중치다. 예를 들어 Durability 0.3, BaseDamage 100, DurabilityDamage 20이면 장갑 적용 전 76이며 장갑과 관통이 같으면 49.4다. 이는 코드를 설명하기 위한 수치 예시이며 실제 무기 데이터 값은 아니다.

이 피해를 부위 HP에서 빼고, 피해×Influence를 코어에서도 뺀다(:236–238). Influence는 부위 피해가 전체 생명에 얼마나 전해지는가이다. 부위와 코어가 같은 포인터인 경우 현재 두 차감이 같은 HP에 적용된다. _curHP는 int32이므로 float 피해를 적용하면 저장 과정의 정수 변환도 발생한다. ChangeHp(:110)는 0~최대 HP로 Clamp하고 0 이하면 파괴 이벤트를 호출한다. EDamageType 인자는 현재 이 함수의 추가 속성 배율 계산에 쓰이지 않는다. “산성이라 별도 추가 피해 배율이 있다”와 같은 설명은 현재 코드에서 근거를 찾지 못했다.

발사체 AGunBulletBase의 CalculateHitOutcome(Gun/GunBulletBase.cpp:281)은 velocity의 반대 방향과 표면 normal의 각도로 AP를 직접/완만/큰/극단 입사각 값 중 선택하고 관통 횟수를 뺀다. AP가 AV보다 2 이상 높으면 관통 지속, 1 높으면 완전 관통, 같으면 부분 관통, 낮으면 도탄이다. ProcessHitOutcome(:303)은 관통 지속 시 속도를 낮추고, 도탄 시 반사 방향에 0.6을 곱하며, 관통 종료 상태면 탄을 파괴한다. 이 결과 판정과 StatComponent의 피해 배율은 서로 다른 역할이다. HitscanDamageComponent는 표준 ApplyPointDamage를 사용하는 반면 CharacterBase는 커스텀 이벤트 경로에 HP 계산이 있어 두 피해 경로의 통합은 후속 확인 대상이다.

## 1.13 부위 파괴는 데이터에서 이벤트로, 이벤트에서 적별 반응으로 이어진다

공통 PartInit(CharacterBase.cpp:175)은 Core 첫 레이어의 _onPartDestroyed를 OnDeath_Handler에 연결한다. 파괴 이벤트 자체는 부위 enum을 전달하지 않으므로 어떤 부위가 깨졌는지는 “어느 FUnitPartStat에 어떤 함수가 bind되었는가”로 결정한다. Critical(:193)은 코어 최대 HP만큼을 감소시켜 사망으로 이어지는 공통 즉사 반응이다. 이름이 비슷한 OnPartDestroyed_Handler(:676)는 부위별 빈 분기와 로그만 있으며 현재의 주요 부위 파괴 경로는 아니다. RestoreAllParts(:703)도 비어 있다.

Standard 적은 PartInit(Enemy/Units/Enemy_Standard.cpp:218)에서 머리와 사지의 첫 레이어 파괴를 Critical에 연결한다. Hunter는 머리에, Spitter는 산성 주머니 파괴 콜백과 여러 즉사 부위에 연결한다. Warrior(Enemy_Warrior.cpp:57)는 머리 파괴 후 즉사 대신 Warrior_Bleeding을 호출한다. 이는 _isBleeding을 세우고 코어 HP를 _additiveHP로 바꾼 후 1초 간격 CalcBleedingDamage를 반복하여 결국 죽게 하는 특수 반응이다. 머리를 잃어도 잠깐 행동을 지속하는 적을 “모델만 다른 적”이 아닌 이벤트 기반 변형으로 구현한 부분이다.

Spewer의 GetHittedPartStat(Enemy_Spewer.cpp:12)은 Head에서 피격 Z가 콜라이더 중심 위면 첫 레이어, 아래면 두 번째 레이어를 선택한다. Titan(:26)은 Torso 위/아래, Tail 위/아래, 사지의 외부 장갑 생존 여부로 여러 레이어를 선택한다. Tail 아래쪽의 두 번째 레이어가 파괴되면 세 번째 내부 레이어를 선택하고, 사지는 첫 레이어 HP가 0이면 두 번째로 넘어간다. 이것은 모든 부위에 자동으로 적용되는 범용 레이어 시스템이 아니라 적별 선택 함수가 구현하는 규칙이다. 배열 순서를 바꾸면 피해 대상이 바뀌므로 레이어 정의와 override를 함께 읽어야 한다.

Titan의 AcidbagDestroyed는 산성 공격 timer를 지우고 _isSpitReady=false로 만든다. Titan_Bleeding은 코어 HP를 바꾸지만 Warrior와 같은 주기 피해 루프는 이 함수에 없다. 파괴 이벤트와 복구 이벤트가 자료형에 모두 존재한다고 모든 적의 파괴 모델 교체·부위 재생까지 완성되었다고 볼 수 없다. 새 부위 반응을 추가할 때는 HP=0이 되는 순간의 이벤트, 같은 부위에 후속 피해가 들어오는 경우, pooled reset 시 원래 기능 복구를 순서대로 확인해야 한다.

## 1.14 피격 물리·낙하·래그돌에서 위치의 기준이 바뀐다

약한 stagger는 WeakStagger(CharacterBase.cpp:986)에서 이동을 즉시 멈추고 LightStagger를 부여한다. 강한 stagger(:993)는 ActionEnd로 행동을 끊고 이동을 멈춘 뒤 StrongStagger를 부여한다. 헬다이버 override(:1137)는 이에 KnockDown을 추가한다. KnockBack(CharacterBase.cpp:1003)은 받은 방향에 계수 10을 곱해 LaunchCharacter를 호출한다. PlayHitReaction(:1043)은 action을 새로 잠그고 hit reaction montage를 재생하며 timer로 HitRecovery를 호출한다. 단순 피격 애니메이션과 물리 knockdown은 별도 경로다.

CharacterToRagdoll(:377)은 Movement를 끄고 Mesh를 캡슐에서 분리해 새 RootComponent로 설정한다. 캡슐 collision은 꺼지고 모든 mesh body의 물리 simulation이 켜진다. 캡슐은 다시 Mesh 자식이 되어 상대 위치와 absolute rotation을 설정한다. 또한 PhysicsAsset의 ConstraintSetup 기본 linear limit를 free로 바꾼다. 따라서 이 구간의 액터 위치는 평소의 캡슐 중심과 동일한 의미가 아니며, 공유 PhysicsAsset 기본 설정을 변경하는 영향도 따로 검토할 대상이다.

RecoverFromKnockDown(:442)은 pelvis 월드 위치를 기준으로 새 캡슐 위치를 정한다. 물리 simulation을 끄고 캡슐을 루트로 복원하며, pelvis 위치에 캡슐 반높이를 더해 teleport하고, Mesh를 캡슐 아래에 다시 붙인다. collision과 MOVE_Walking, 애니메이션 업데이트를 다시 켠다. 헬다이버 override(:1074)는 상태를 Proning으로 바꾸고 그 자세의 collision·속도를 적용한다. 파일에 KnockDownRecovery(:348)라는 다른 복구 함수도 남아 있으므로 이름이 비슷한 함수를 혼동하지 말고 timer가 가리키는 RecoverFromKnockDown을 기준으로 읽어야 한다.

Landed(HellDiver.cpp:825)는 Z 속도와 현재 자세에 따라 회피 종료, HardLanding montage, 낙하 damage/stagger를 처리한다. 기본 수치는 200 단위 낙하 속도마다 damage 10, stagger 5이며 서 있거나 달리는 자세 외에는 stagger를 3배로 한다. FCDamageEvent를 만들어 자신의 TakeDamage에 넣기 때문에 착지 피해도 부위·코어·상태 이상 파이프라인을 공유한다. 부모 Landed(:329)의 일부 분기는 주석뿐이고 SoftLanding/HardLanding의 종료 함수 FinishLanding은 현재 실질 동작이 비어 있다. 회피 도약의 결과를 이해하려면 Rolling → 실제 Movement velocity → Landed → 상태 종료 순서를 함께 봐야 한다.

## 1.15 회복·사망·증원·풀 재사용의 계약

UseStimPack(HellDiver.cpp:454)은 IsActionable을 확인하고 UStimPackComponent에 맡긴다. StimPackComponent::UseStimPack(:36)은 개수를 1 줄이고 RestoreAllParts를 호출한 뒤 StartOrExtendRegen(:92)을 수행한다. 기본 값은 시작 2개/최대 4개, 회복 지속 4초, 간격 0.05초, 한 tick당 각 레이어 HP +10이다. 이미 회복 timer가 있으면 남은 tick 예산에 새 4초분을 추가한다. HandleRegen(:68)은 모든 부위·레이어를 참조로 순회하고 ChangeHp를 호출한다. 이름은 코어 회복처럼 읽힐 수 있지만 실제 구현은 전체 레이어 회복이며, 별도 파괴 기능 복구인 RestoreAllParts는 공통 구현이 비어 있다.

사망 여부의 기준은 UStatComponent::IsDead(:57)의 Core 첫 레이어 HP다. 부위 Map이 비었거나 Core 배열의 첫 원소가 없으면 사망으로 취급한다. 다만 Map에 다른 부위만 있고 Core 키 자체가 없는 경우에는 무검증 Map 접근이므로 정상적인 fallback으로 보장되지 않는다. HP 0의 파괴 이벤트가 OnDeath_Handler → virtual Dead를 호출한다. CharacterBase::Dead(:493)는 ready-to-spawn을 false로 하고 래그돌로 만든다. AEnemy::Dead(Enemy.cpp:254 부근)는 컨트롤러를 떼고 60초 후 RecoverFromDead를 예약한다. AHellDiver::Dead(:1082)는 모든 총, 배낭, 샘플을 떨어뜨리고 컨트롤러를 ReinforceManager queue로 넘긴 뒤 5초 후 AfterDead를 예약한다. PlayerCharacter의 UnPossessed는 AI perception source도 해제하므로 소유권 변경이 타깃 인식과 연결된다.

AfterDead(:1114)는 ReinforceHelldiver를 호출하여 다른 pooled player를 드롭포드에 태우고, 자기 시체는 추가 60초 뒤 RecoverFromDead로 준비 상태가 된다. RecoverFromDead(CharacterBase.cpp:509)는 래그돌에서 복구한 뒤 UnitDeactivate로 숨기고 collision/tick을 끄며 위치를 원점으로 옮긴다. 헬다이버 override(:1130)는 LifeState를 Alive로 바꾸고 재사용 가능 플래그를 세운다. 따라서 “죽은 플레이어가 5초 뒤 같은 인스턴스로 다시 살아난다”가 아니라, 컨트롤러는 새 준비된 Pawn으로 이동하고 기존 시체 Pawn은 뒤늦게 풀에 돌아오는 구조다.

AHelldiverReinforceManager는 초기 pool을 SpawnActor로 만든 뒤 UnitDeactivate한다. AcquirePooledDiver는 컨트롤러가 없고 IsReadyToSpawn인 Pawn을 찾는다. ReinforceHelldiver는 컨트롤러 queue에서 하나를 꺼내 pool을 얻고 드롭포드·입력 Pawn을 생성한다. 생성 실패 시 컨트롤러를 queue로 반환한다. 성공하면 controller가 입력 Pawn을 possess하고 pooled diver를 pod에 SetPreSpawnedPlayer로 전달한다. pod가 착지 단계에서 최종 플레이어로 연결하는 부분은 드롭포드 시스템 장에서 이어 읽으면 된다.

ResetUnit(CharacterBase.cpp:566)의 공통 계약은 StatComponent::Reset, StateComponent::Reset 후 collision/tick/visibility 활성화다. Spawn(:633)은 ResetUnit을 호출하고 스폰 montage가 있으면 action을 잠그고 _reservedFunction에 SpawnProcessFinish를 예약한다. 현재 SpawnProcessFinish는 비어 있고 공통 state reset은 상태 이상 map/bit를 지우지만 모든 행동 플래그·생명 상태·무기·timer까지 초기화하는 통합 루틴은 아니다. StatComponent::StartRegen(:141)은 두 range-for가 복사본을 수정하여 실제 HP 초기화가 반영되지 않는다. 이는 설계 설명과 별개로 현재 pooling 흐름을 읽을 때 반드시 기억할 실행상의 차이다.

HP 표시 이벤트도 변경 경로별로 다르다. ChangeCoreHp(:129)는 _coreHpChanged를 발행하지만 일반 ProcessDamage와 StimPack은 ChangeHp를 직접 호출한다. PlayerCharacter::InitWeapon(:1864) 및 Enemy::BeginPlay는 해당 HP delegate를 HUD에 연결한다. HP를 직접 쓰거나 ChangeHp만 사용하면 화면 이벤트가 생략되는 현재 성질이 있다. 회복·피격·재사용 수정은 실제 수치와 화면 상태를 같이 확인해야 한다.

## 1.16 흐름을 다시 떠올리기 위한 구체적인 읽기 시나리오

“달리다가 조준하여 쏜다”를 따라갈 때는 InputAction의 Started/Triggered/Completed → PlayerCharacter::StartAiming/StartFiring → FinishSprint → ECharacterState 변경 → collision·속도 변경 → Gun의 aiming/firing flag → HellDiverAnimInstance 수집 → AnimLayer의 시퀀스 선택 순서로 읽는다. 카메라 중앙의 목표와 총구 실제 transform이 별개이므로 마지막으로 Gun의 GetMuzzleLoc/FireShot까지 이어서 본다. 어느 한 bool만 바꿔도 모션이 맞지 않으면 다른 축의 상태와 AnimGraph 전환 조건을 확인한다.

“적의 머리를 맞힌다”는 GunBulletBase overlap → 피격 collider의 태그 → 입사각 AP/AV 결과 → FCDamageEvent → CharacterBase::TakeDamage → 적 override의 레이어 선택 → StatComponent::ProcessDamage → 부위 HP와 코어 HP 변경 → PartInit에서 bind한 특수 반응 순서다. 머리 파괴가 즉사인 적과 Warrior처럼 추가 행동 시간으로 바뀌는 적은 마지막 callback이 다르다. 데이터의 부위 레이어를 읽고 해당 적의 GetHittedPartStat/PartInit을 이어 읽으면 외형보다 행동 규칙이 잘 보인다.

“벽을 넘다 피격되거나 몽타주가 중단된다”는 입력 → 파쿠르 탐색 → 이동/충돌 잠금 → warp target 설정 → montage notify와 blend/end callback → ActionEnd/UnitUnable → 이동·충돌·허용 플래그 복구를 본다. 몽타주 시작 성공만으로 완료가 보장되는 구조가 아니므로 종료·중단·실패 경로를 같은 비중으로 읽는다. Animation montage, PhysicsAsset, collision profile, named sockets/warp targets가 C++와 만나는 연결 지점이다.

“죽고 증원된다”는 Core HP=0 → delegate → Dead → 컨트롤러 queue → AfterDead → 준비된 pool Pawn과 pod → possess 변경 → 기존 Pawn의 RecoverFromDead/UnitDeactivate 순서다. 이후 ResetUnit이 어떤 필드를 실제로 되돌리는지를 별도로 읽는다. 임시 시체 Actor를 만드는 SpawnGhost는 공통 함수가 존재하지만 모든 사망 경로에서 자동 호출되는 것은 아니다.

이 장을 기준으로 에디터에서 확인할 연결은 실제 Player/Enemy BP 부모, Mesh AnimClass, Linked Anim Layer 클래스와 인터페이스, 자세별 collision/camera DataAsset, InputAction/MappingContext, UnitTable의 클래스 키와 PartDefinition 레이어 수, hitbox 태그, MotionWarping의 VaultStart/VaultEnd, 공격/투척/종료 Notify 배치, Distance/IsTurning/root_rotation_Z curve이다. 코드에 빈 함수가 있는 항목은 에셋이 기능을 보충하는지 확인하기 전까지 미완성 확장 지점으로 남겨 두어야 한다. 이 설명서는 새 기능을 가정하여 재설계한 문서가 아니라 현재 구현을 따라가는 지도다.
