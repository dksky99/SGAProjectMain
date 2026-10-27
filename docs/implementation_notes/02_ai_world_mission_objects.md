# AI, 적 분대, 내비게이션과 월드 임무 오브젝트 구현 설명

이 문서는 기존 구현을 다시 이해하고 수정 범위를 판단하기 위한 시스템 설명서이다. 현재 작업 트리의 C++ 소스, 프로젝트 설정, 콘텐츠 파일 목록, 과거 실행 로그를 읽어 작성했다. 게임 실행, 빌드, 에디터에서의 Blueprint·Behavior Tree·맵 내부 확인은 이 문서 작성 과정에서 하지 않았다. 따라서 C++ 함수의 동작과 호출 연결은 설명할 수 있지만, 각 BP의 실제 기본값, BT 노드 배치, 몽타주의 Notify 위치, 맵에서의 액터 배치와 실제 플레이 결과까지 확인한 것은 아니다.

소스 근거의 기준 디렉터리는 D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/Source/SGAProjectMain이다. 이하의 “파일:줄” 표기는 이 디렉터리 아래의 파일을 가리킨다. 설정과 콘텐츠는 D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain 아래를 기준으로 별도로 표기한다. 줄번호는 문서 작성 시점의 작업 트리 기준이다.

## 1. 시스템 전체를 읽는 관점

적 AI는 한 클래스가 감지, 이동, 공격, 소환을 모두 처리하는 구조가 아니다. AEnemyController가 외부 자극과 현재 타겟을 관리하고, 적 액터 AEnemy는 이동 설정·애니메이션·종별 행동을 제공한다. 적이 소유한 UBehaviorControlComponent는 두 객체의 상태를 Blackboard에 연결한다. Behavior Tree Service는 지금 어떤 상태가 맞는지 반복해서 판단하고, Task는 그 판단에 맞는 행동을 시작하거나 이동 목표를 만든다. AEnemySquad는 여러 적을 묶어 재사용 가능한 풀을 구성하고, ASquadSpawner는 그 풀에서 적을 골라 월드에 내보낸다. 상위 매니저는 어느 분대와 스포너를 사용할지를 결정한다.

이를 플레이 상황에 대입하면 “플레이어 발소리 발생 → 적 컨트롤러의 청각 경계치 증가 → Service가 Weak_Alert 또는 Strong_Alert 기록 → BT가 목표 위치로 이동하는 가지 실행 → 시각으로 플레이어 확인 → InBattle 전환 → 거리와 조준 방향에 따라 행동 선택”이라는 흐름이 된다. 여기서 Service가 Blackboard 값을 썼다는 사실과 실제 BT가 그 값을 어떻게 이용하는지는 구분해야 한다. 소스는 상태 공급과 행동 노드를 제공하지만, BT_EnemyHammer·BT_EnemyAnvil 에셋의 실제 가지 순서는 아직 확인하지 않았다.

월드 임무 오브젝트는 이 AI 흐름과 다른 이벤트 축을 가진다. 플레이어가 AInteractable을 선택하여 Interact를 호출하고, 단말은 플레이어의 입력 상태를 InterActing으로 전환한다. 작업 객체가 입력 조합 완료 이벤트를 발송하면 단말을 포함한 송신탑 또는 탈출 비콘이 반응한다. 송신탑·공장 같은 목표는 AMainGameMode에 목표 ID를 전달하고, GameMode가 임무 완료와 탈출 가능 여부를 관리한다. 탈출기는 착륙·해치 개방·이륙 애니메이션 Notify를 통해 탑승과 전투 종료 시점을 연결한다.

| 층 | 핵심 객체 | 맡는 책임 | 다음 층으로 전달하는 것 |
| --- | --- | --- | --- |
| 개별 적의 감각 | AEnemyController | 시각·청각·피해 감지, 팀 판정, 현재 타겟·마지막 감지 위치 | 타겟 액터, 경계치, 감지 위치 |
| 판단 정보 연결 | UBehaviorControlComponent | Pawn·Controller 정보를 Blackboard 키로 읽고 쓰기 | UnitState, BattleState, Target 등 |
| BT 판단·실행 | Enemy Service와 Task | 전체 경계 상태, 거리 구간, 행동 시작·종료, 순찰 목표 | 상태 값, 목표 위치, 행동 실행 요청 |
| 적의 몸과 능력 | AEnemy 및 Units 파생 클래스 | 이동 모드, 몽타주, 근접·점프·산성 공격·방어·회피 | 실제 이동·피격·애니메이션 |
| 병력 재사용 | AEnemySquad | 적 생성, 보관, 분대 명령, 소환 가능 여부 | 준비된 적 하나, 분대 전체의 명령 |
| 등장 방식 | ASquadSpawner와 파생 클래스 | 위치 선택, AI 재빙의, 시간 간격을 둔 소환 | 월드에 활성화된 적 |
| 전장 정책 | 증원·순찰·주둔 매니저 | 호출 제한, 순찰 경로 배치, 공장 파괴 집계 | 분대·스포너 배정, 목표 완료 |
| 임무 진행 | 단말·송신탑·비콘·탈출기·GameMode | 상호작용, 입력 완료, 목표 판정, 탈출과 결과 화면 | 완료 이벤트, 탑승 인원, 임무 결과 |

대표 근거: Controller/EnemyController.cpp:71, Character/Enemy/BehaviorControlComponent.cpp:45, Character/Enemy/EnemySquad.cpp:47, Character/Enemy/Squad/SquadSpawner.cpp:46, MainGameMode.cpp:75.

## 2. 적의 생성, AIController 연결과 재사용 생명주기

AEnemy는 ACharacterBase를 상속한다. 공통 캐릭터에서 이미 스탯·상태·피격·근접 공격·재사용 기능을 제공하고, AEnemy 생성자는 상태 컴포넌트, 화면 공간 체력 표시 WidgetComponent, 순찰 컴포넌트, Navigation Invoker, 행동 제어 컴포넌트를 준비한다. 체력 위젯은 BeginPlay에서 UDummyHpBar로 캐스팅하고 스탯 컴포넌트의 코어 체력 변경 이벤트에 연결한다. 적마다 체력 위젯이 있다는 구조는 C++로 확인되지만, 각 BP에 지정한 WidgetClass와 실제 표시 여부는 별도 확인 대상이다.

적이 컨트롤러에 빙의되면 AEnemy::PossessedBy가 AEnemyController를 _controller에 저장하고 Navigation Invoker를 활성화한다. 이 _controller는 현재 빙의 여부와 별개의 캐시이다. 현재 빙의 관계는 GetController()로 조회하고, GetCachedController()는 풀에 돌아간 이후에도 다시 연결할 수 있도록 보관한 포인터를 반환한다. CombineController는 새 컨트롤러를 생성하는 함수가 아니라, 저장된 컨트롤러의 Possess(this)를 호출하는 함수이다. 이 구분은 풀에서 적을 꺼낼 때 특히 중요하다.

AEnemyController::OnPossess는 Pawn을 AEnemy로 캐스팅한 뒤 적이 가진 BlackboardData, BehaviorTree, BehaviorControlComponent를 받아온다. 행동 제어 컴포넌트의 Init로 소유 Pawn과 Controller를 연결하고, 현재 적의 UnitState에 맞춰 감각 설정을 선택한다. 이후 PerceptionComponent를 활성화하고 Pawn의 팀 ID를 맞춘다. UseBlackboard가 성공해야 RunBehaviorTree로 넘어간다. Blackboard 또는 BT 준비가 실패하면 로그를 남긴다. 실제 어느 BT와 Blackboard를 사용할지는 AEnemy의 _blackBoard·_behaviorTree라는 EditAnywhere 필드에 지정된 콘텐츠 값에 달려 있다.

분대 풀에 들어갈 때 적은 숨김·충돌 해제·액터 Tick 해제·원점 이동을 받고 Controller가 UnPossess된다. Controller는 OnUnPossess에서 Brain의 로직을 중단하고 PerceptionComponent를 비활성화하며 타겟·경계치를 리셋한다. 적의 UnPossessed는 Navigation Invoker도 비활성화한다. 컨트롤러 객체와 적 객체를 Destroy하는 경로가 아니므로 이후 같은 쌍을 재빙의하여 사용한다. _controller 캐시를 지우는 방식으로 바꾸면 CombineController가 실패하여 스포너가 적을 되살리는 흐름이 끊긴다.

적의 죽음도 즉시 액터 제거와 같지 않다. AEnemy::Dead는 공통 Dead의 래그돌 전환을 호출하고 현재 Controller를 떼어낸 뒤 60초 후 ACharacterBase::RecoverFromDead를 예약한다. RecoverFromDead는 래그돌 복구와 UnitDeactivate를 거쳐 _isReadyToSpawn=true로 바꾸는 흐름이다. 반대로 Spawn은 _isReadyToSpawn=false로 두고 ResetUnit으로 스탯·행동 상태·표시·충돌·Tick을 초기화한다. 스폰 몽타주가 있으면 행동 중 상태를 시작하고 예약 콜백을 준비한다. SpawnProcessFinish 자체는 현재 빈 함수이므로 “스폰 애니메이션 이후 특별한 후처리”가 구현된 것으로 설명하면 안 된다.

재사용 가능 상태는 ACharacterBase::IsReadyToSpawn이 결정한다. Controller가 붙어 있으면 무조건 false이고, Controller가 없을 때만 _isReadyToSpawn을 반환한다. 따라서 숨김 여부만 보고 풀의 준비 상태를 판단할 수 없다. 살아 있어도 Controller가 붙은 적은 소환 대상이 아니고, Controller가 없어도 죽은 직후의 적은 60초 복구 전까지 준비 상태가 아니다. 적 종류별 쿨타임·출혈·예약 콜백·이동 모드가 공통 ResetUnit에서 모두 초기화되는지도 수정 시 함께 살펴야 한다.

대표 근거: Character/Enemy/Enemy.cpp:33, :98, :144, :256, :409; Controller/EnemyController.cpp:71, :119; Character/CharacterBase.cpp:493, :509, :566, :609, :633.

## 3. 시각, 청각, 피해 감지와 경계 단계

### 3.1 감각 입력이 만들어지는 지점

Controller 생성자는 UAIPerceptionComponent와 Sight·Hearing·Damage 설정 객체를 만든다. 초기 시각 반경은 1,500cm, 시야 상실 반경은 2,000cm, 주변 시야각 설정은 160도이다. 청각 반경은 700cm이고 자극 최대 나이는 3초이다. 시각과 청각은 Enemy만 감지하도록 켜고 Friendly와 Neutral은 끈다. 팀 ID는 ETeamID::Enemy로 설정된다. 여기의 “Enemy”는 감각 설정에서 보는 상대 팀 성격이고, 프로젝트 적 클래스와 같은 편을 감지한다는 뜻이 아니다.

공통 캐릭터는 AIPerceptionStimuliSourceComponent를 이용하여 시각·청각·피해 감각의 자극 소스로 등록한다. 소리를 만드는 함수는 ACharacterBase::MakeSound로, 현재 액터 위치·loudness·자기 자신·soundName을 ReportNoiseEvent에 전달한다. C++에서 확인한 호출자 중에는 UHellDiverAnimInstance::AnimNotify_FootStep이 있으며, 이동 속도를 기본 이동 속도로 나눈 값을 loudness로 보낸다. 이 때문에 발소리 Notify 배치와 애니메이션 속도 데이터는 AI 청각에도 영향을 준다. 사격 소리까지 같은 방식으로 보고되는지는 이 문서에서 확인한 C++ 호출만으로는 단정하지 않는다.

피해 감지는 캐릭터 TakeDamage의 시작에서 ReportDamageEvent로 보고된다. 맞은 객체는 this이고, 공격자는 EventInstigator가 있으면 그 Pawn, 없으면 DamageCauser이다. 공격 위치와 피격 캐릭터 위치도 함께 전달한다. 피해의 실제 계산은 뒤의 커스텀 FCDamageEvent 처리로 이어지므로 AI의 피해 감지 입력과 체력 감소 계산은 같은 함수 안에서 출발하지만 역할은 다르다.

대표 근거: Controller/EnemyController.cpp:28; Character/CharacterBase.cpp:74, :306, :922; Character/HellDiver/HellDiverAnimInstance.cpp:86.

### 3.2 감지 업데이트의 처리 순서

OnTargetPerceptionUpdated는 OnTargetDetected에, OnPerceptionUpdated는 PerceptionUpdated에 연결된다. OnTargetDetected는 성공 감지와 감지 실패를 로그로 출력하는 역할이고, 실제 게임 상태를 바꾸는 함수는 PerceptionUpdated이다. 이 함수는 갱신된 액터마다 시각 → 청각 → 피해 순서로 CanSenseActor를 호출한다. CanSenseActor는 감지 정보의 LastSensedStimuli를 순회하고 각 자극의 SenseClass가 요청 감각과 같은지를 비교하여 해당 FAIStimulus를 돌려준다. 감지 성공 자극에 대해서만 각 HandleSensed 함수가 실행된다.

HandleSensedSight와 HandleSensedDamage는 현재 Pawn이 있고 상대가 CheckTargetable을 통과하는지 확인한 뒤 _curTarget을 그 액터로 바꾼다. 여러 상대가 같은 업데이트 배열에 들어오면 이 순서대로 나중에 처리된 유효 상대가 현재 타겟이 될 수 있다. 여기에는 점수화된 위협도나 시각보다 피해 타겟을 영구 우선하는 정책이 따로 없다. HandleSensedHearing은 상대 액터 자체를 타겟으로 저장하지 않고 자극 위치를 _lastSensedLoc에 저장한다.

CheckTargetable은 자기 자신을 제외하고 IGenericTeamAgentInterface, 팀 적대성, Enemy가 아닌지, ITargetable 구현 여부, IsTargetable 결과를 검사한다. GetTeamAttitudeTowards는 Pawn이 아닌 액터를 Neutral로 처리하고, 같은 TeamId를 가진 Pawn을 Friendly로 처리한다. 그 외 ITargetable이며 타겟 가능한 Pawn은 Hostile이 된다. 따라서 새로운 공격 대상이 일반 AActor로 구현되거나 팀 인터페이스를 제공하지 않으면 “피격 가능하다”는 사실만으로 이 적 AI의 타겟이 되지는 않는다.

ITargetable은 타겟 가능 여부, 타겟 Transform, 바라보는 위치·방향을 제공하는 계약이다. 공통 캐릭터의 IsTargetable은 Controller가 있고 스탯이 죽지 않은 상태여야 true이다. PlayerCharacter의 GetTargetLook은 캐릭터 위치와 _focusingLoc으로 향하는 벡터를 반환한다. 이것은 카메라의 원점·총구 위치를 그대로 반환하는 API가 아니며, AI의 조준 반응은 이 계산된 시선 벡터를 기준으로 한다.

대표 근거: Controller/EnemyController.cpp:164, :178, :203, :258, :274, :473, :502; Interface/Targetable.h:20; Character/CharacterBase.cpp:721; Character/PlayerCharacter.cpp:1580.

### 3.3 소리가 쌓이는 방법과 경계치가 의미하는 것

청각 입력은 먼저 마지막 들은 위치를 갱신한 다음 _isReadyToStack을 검사한다. 증가가 허용된 상태이면 이를 false로 바꾸고 0.5초 후 ReadyToStack을 호출하는 타이머를 예약한다. 실제 경계치 증가는 loudness 그대로가 아니라 _curLoudnessStack에 1을 더하는 방식이다. 감각 시스템이 소리의 loudness로 도달 범위를 결정할 수는 있지만, 이 Controller에서 누적하는 경계치는 감지 횟수에 가까운 값이다. AddAlertStack(float)라는 별도 API는 전달량을 더하지만 청각 처리 경로는 그 API를 쓰지 않는다.

비전투 상태에서 소리를 들으면 기존 15초 리셋 타이머를 지우고 다시 예약한다. 15초 동안 새로운 소리 입력이 없으면 ResetAlertStack이 누적치를 0으로 만든다. 이미 InBattle인 적은 경계치를 증가시킨 뒤 이 리셋 타이머 갱신 부분에서 빠져나온다. 분대가 RecieveTargetLoc으로 위치 명령을 보내면 경계치를 6으로 두고 _lastSensedLoc을 갱신한다. 자연 청각 자극 없이도 강한 경계 상태에 들어가 명령받은 지점을 조사하도록 만든 방식이다.

EAIAlertStep은 감각 설정을 None/Low/Medium/High로 나누는 별도 상태이다. 실제 함수 값은 None 0/0, Low 청각 500·시각 1,000, Medium 청각 700·시각 1,500, High 청각 1,000·시각 2,000cm이다. EUnitState의 Stay·Patrol은 Low, Weak_Alert는 Medium, Strong_Alert·InBattle은 High에 대응한다. 현재 C++에서는 SetAlertStep의 호출이 OnPossess에 있고, UnitState 전환마다 다시 호출하는 연결이나 감각 리스너의 설정 갱신 호출은 보이지 않는다. 그러므로 “전투 상태가 되면 현재 감지 반경이 즉시 늘어난다”는 완료된 동작으로 설명하기보다는, 그 의도를 가진 설정 함수가 있다는 것으로 이해해야 한다.

현재 타겟의 반환 경로도 기억해야 한다. GetCurTargetActor는 CheckTargetRefresh에서 ITargetable이 아니거나 타겟 불가능하거나 숨겨진 경우 타겟을 비운다. 시야 자극을 잃었다는 사실 자체로 _curTarget을 비우는 경로는 없다. 시야 밖 플레이어의 실제 액터를 계속 보유할 수 있으므로, 나중에 “마지막 본 위치 조사 후 타겟 포기”를 추가한다면 Controller, Blackboard Target, UnitState를 함께 설계해야 한다. _targets 배열과 GetNewTargetActor의 최소 거리 선택 API는 존재하지만, 현재 시각 처리 함수가 AddTargetActor로 후보를 축적하는 흐름은 아니다.

대표 근거: Controller/EnemyController.cpp:219, :318, :349, :377, :403, :413, :438, :457; Controller/EnemyController.h:110.

## 4. Blackboard 연결과 두 종류의 AI 상태

EUnitState는 전투 전후를 포함한 큰 행동 상태이다. Stay, Patrol, Weak_Alert, Strong_Alert, InBattle을 구분한다. EBattleState는 InBattle 안에서의 거리 구간을 표현하며 None, OutOfRange, Far, Middle, Near, Melee를 가진다. 같은 적이 “InBattle이면서 Middle”일 수 있으므로 두 enum은 서로 대체 관계가 아니다. 상태 하나로 합치려면 지금 BT에서 두 키를 이용하여 가지를 나누는 방식도 함께 바뀐다.

UBehaviorControlComponent는 AEnemy의 컴포넌트이고 Init에서 Owner를 AEnemy로, 그 Controller를 AEnemyController로 캐시한다. 이 컴포넌트의 함수는 단순한 자체 필드 getter·setter만이 아니다. ChangeUnitType은 Pawn의 SetUnitState를 먼저 호출하여 이동과 알림을 바꾸고, Blackboard에 enum을 기록한다. GetCurUnitType은 Pawn의 _unitState가 아니라 Blackboard의 UnitState를 읽는다. 다른 함수도 Controller의 현재 값과 Blackboard 값을 오가므로 이 컴포넌트는 “판단 정보의 접점”이라고 보는 편이 정확하다.

| 기본 키 이름 | 기록하는 내용과 자료형 | 주요 생산·소비 경로 |
| --- | --- | --- |
| UnitState | EUnitState를 enum으로 기록 | Enemy Service → ChangeUnitType → BT 분기, 분대 회수 검사 |
| BattleState | EBattleState를 enum으로 기록 | 전투 Service → 거리별 BT 분기 |
| Target | AActor 객체 | Controller 타겟을 Service가 Blackboard로 복사 |
| TargetLoc | FVector | 경계 조사 지점, 순찰 다음 위치 → 이동 Task |
| Distance | float | 전투 Service의 Pawn–Target 거리 |
| AlertThreshold | float | Controller 누적 경계치를 Service가 기록 |
| IsAbleToAct | bool | Middle/Far에서 적 종류별 조건을 검사한 결과 |
| IsTargetLookAtMe | bool | 상대 시선 방향과 적 위치 사이 각도가 기본 45도 이내인지 |
| IsTargetAimingMe | bool | 위 각도가 기본 5도 미만인지 |
| IsUnable | bool | 캐릭터 상태 컴포넌트의 행동 불가 판정 |

위 키 이름은 BehaviorControlComponent.h의 EditAnywhere FName 기본값이다. 실제 BB_Enemy 에셋에 동일 이름과 타입이 있어야 하고, BP에서 키 이름을 변경했다면 그 값이 함수가 사용하는 이름이 된다. Blackboard와 컨트롤러의 중복 상태를 구별하는 것도 중요하다. SetTargetActor는 Blackboard Target을 쓰지만 GetTargetActor는 Controller::GetCurTargetActor를 읽는다. SetTargetLoc은 Blackboard TargetLoc을 쓰지만 GetTargetLoc은 Controller의 _lastSensedLoc을 읽는다. 따라서 순찰 Task가 TargetLoc을 바꾼다고 해서 Controller의 마지막 청각 위치도 바뀌지는 않는다.

ChangeUnitType은 현재 Blackboard 상태와 새 상태가 같으면 아무 작업도 하지 않는다. 다르면 Pawn::SetUnitState를 호출한다. 적의 Stay·Patrol은 기본 최대 이동 속도를 200cm/s로 두고, Weak_Alert·Strong_Alert·InBattle은 스탯의 기본 속도를 사용한다. Patrol은 이동 방향 회전, Weak_Alert와 InBattle은 컨트롤러가 원하는 회전을 사용하도록 설정한다. InBattle 진입은 RaiseAlert도 호출한다. 따라서 UnitState는 단순 UI 값이 아니라 속도, 회전, 주변 적 전파, 몽타주까지 영향을 준다.

대표 근거: Character/Enemy/Enemy.h:10, :22, :130; Character/Enemy/BehaviorControlComponent.cpp:31, :45, :141, :152; Character/Enemy/BehaviorControlComponent.h:95; Character/Enemy/Enemy.cpp:331.

## 5. Service의 판단 순서와 Task의 행동 종료

### 5.1 전체 상태 Service

UBT_Service_Enemy::TickNode는 Owner를 EnemyController로 받아 행동 제어 컴포넌트와 Pawn을 얻는다. 먼저 Pawn의 상태 컴포넌트가 IsUnable인지 Blackboard에 기록한다. Controller의 현재 타겟이 있으면 Target을 기록하고 UnitState를 InBattle로 바꾼 뒤 종료한다. 타겟이 없는 경우에는 Target을 비우고 경계치를 기록한다. 경계치가 5 이상이면 Strong_Alert와 마지막 감지 위치, 1 이상이면 Weak_Alert와 마지막 감지 위치를 기록한다. 경계치가 없고 순찰 경로가 있으면 Patrol, 그렇지 않으면 Stay가 된다.

이 순서는 상태 우선순위이다. 타겟 존재가 소리 누적보다 우선하고, 소리 누적은 순찰 경로보다 우선한다. PatrolComponent에 경로가 남아 있어도 소리를 들은 동안은 경계 행동을 선택할 수 있다. Service가 실제 이동 명령을 직접 내리는 것은 아니므로 “경계치가 5 이상이면 곧바로 MoveTo 호출”로 읽어서는 안 된다. Service는 Blackboard를 채우고, BT 에셋의 Decorator·Selector·이동 노드가 그 다음 실행을 결정한다.

### 5.2 전투 상태 Service와 조준 반응

UBT_Service_Enemy_InBattle은 타겟이 없으면 Target을 비우고 BattleState=None으로 만든다. 타겟이 있으면 ITargetable::GetTargetLook으로 상대의 시선 위치·방향을 받고 IsFacingTarget_WithAngle을 호출한다. 도우미는 상대 위치에서 적 위치로 향하는 정규화 벡터와 상대 시선 벡터의 내적을 acos로 각도로 바꾼다. 기본 45도 이내를 IsTargetLookAtMe, 5도 미만을 IsTargetAimingMe에 기록한다. 조준 키는 실제 ADS 버튼 상태를 검사한 값이 아니라 상대 시선과 적의 위치가 얼마나 일치하는지를 근사한 결과이다.

이후 GetDistanceTo로 거리를 계산하여 Distance에 기록한다. 거리 기준은 Melee 250, Near 600, Middle 1,500, Far 3,500cm의 AEnemy 기본값이며 BP에서 바꿀 수 있다. 가장 가까운 조건부터 검사하므로 250cm 이하이면 Melee, 그 다음 600cm 이하 Near 등으로 한 구간만 선택한다. Middle에서는 CheckAbleTryMiddle(target), Far에서는 CheckAbleTryFar(target)의 결과를 IsAbleToAct에 기록한다. Melee·Near·OutOfRange 진입 시에는 이 bool을 별도로 초기화하지 않는 현재 흐름이므로, 이를 읽는 BT 가지는 거리 상태도 같이 보는 것이 중요하다.

### 5.3 거리별 행동 Task와 애니메이션 계약

Near/Middle/Far Task는 공통적으로 Controller → BehaviorControl → Enemy Pawn → 현재 Target을 얻고 Pawn의 TryNear, TryMiddle, TryFar 중 하나를 호출한다. 호출 결과가 false면 즉시 Failed, true면 InProgress이다. Task의 Tick은 상태 컴포넌트의 IsActing이 false가 될 때 FinishLatentTask(Succeeded)를 호출한다. 즉 Task는 몽타주 길이만큼 자체 타이머를 기다리는 방식이 아니라, 실제 캐릭터 행동 상태가 종료되는지를 기다린다.

개별 적의 행동 구현은 대체로 ActionBegin을 거쳐 _isActing=true로 전환하고 몽타주를 재생한다. 근접 공격의 실제 판정 시점은 ANS_MeleeCollision NotifyState가 활성·비활성화하는 충돌 구간이고, 특수 행동의 실제 실행 시점은 AN_CallFunction이 _reservedFunction을 실행하는 Notify이다. 몽타주 끝의 AN_AttackEnd는 캐릭터 ActionEnd를 호출하여 충돌 후보를 해제하고 _isActing을 false로 바꾸며 예약 콜백을 해제한다. 이 결과를 Task가 다음 Tick에 관찰하여 완료된다.

예를 들어 산성 공격은 TryMiddle → Spit에서 방향 계산과 SpitProjectile 예약 → 몽타주 → CallFunc Notify에서 투사체 등장 → AttackEnd Notify에서 행동 종료 → BT Task 성공이다. Notify를 삭제하거나 위치를 옮기면 단순한 연출만 바뀌는 것이 아니라 실제 공격 시점과 태스크 종료도 바뀐다. 반대로 PlayAnimMontage 반환값을 확인하지 않는 행동 함수도 있으므로, 몽타주가 재생되지 않아도 행동이 시작된 것처럼 대기할 가능성을 포함해 읽어야 한다. 각 에셋의 실제 Notify 배치는 미확인이다.

Unable Task는 캐릭터가 IsUnable일 때 InProgress를 시작하고 IsUnable이 false가 되면 성공한다. 현재 상태 컴포넌트의 IsUnable은 Shock와 StrongStagger에 관련된 판정이므로 “모든 행동 불가 또는 사망 상태”를 대신하는 완전한 API라고 넓혀 설명할 수 없다. 일반 상태 Service가 IsUnable 키를 공급하고, BT에서 그 키로 해당 태스크 가지를 얼마나 우선할지는 에셋을 봐야 한다.

대표 근거: Character/Enemy/AI/BT_Service_Enemy.cpp:21; Character/Enemy/AI/BT_Service_Enemy_InBattle.cpp:21; Character/Enemy/AI/InBattle/BT_Task_NearDistance.cpp:25; Character/Enemy/AI/InBattle/BT_Task_MiddleDistance.cpp:23; Character/Enemy/AI/InBattle/BT_Task_FarDistance.cpp:23; Character/Enemy/AI/BT_Task_Unable.cpp:23; AnimNotify/AN_CallFunction.cpp:13; AnimNotify/AN_AttackEnd.cpp:24; AnimNotify/ANS_MeleeCollision.cpp:23; Character/CharacterBase.cpp:804.

## 6. 이동, 순찰, 주변 전파와 우회 행동

### 6.1 순찰 경로와 다음 목표 생성

ACPatrolPath는 월드에 놓이는 스플라인 경로 액터이고 OnConstruction에서 _isLoop를 스플라인의 ClosedLoop에 적용한다. GetStartPosition은 첫 스플라인 점의 월드 위치를 반환하며 순찰 매니저가 분대 등장 위치로 사용한다. MakeSpline(start,end)는 기존 점을 비우고 동기 Nav 경로를 구한 뒤 대략 500cm 간격으로 스플라인 점을 만든다. 경로 탐색이 실패하면 시작–끝을 잇는 두 점으로 대체하고, NavSystem이 아예 없으면 시작 위치만 설정하고 종료한다. 이것은 경로 제작 보조 코드이며 순찰 매니저가 매번 MakeSpline을 호출하는 흐름은 아니다.

각 적의 UPatrolComponent는 공유 경로 객체 _path와 자기 자신의 _index·_isReverse를 가진다. GetMoveTo는 현재 인덱스의 월드 위치와 허용 반경을 출력한다. UpdateNextIndex는 루프 경로이면 끝에서 처음으로 돌아가고, 열린 경로이면 끝에서 방향을 뒤집어 왕복한다. 기본 경로 교체는 인덱스를 0으로 되돌리지만 역방향 플래그까지 초기화하는 코드는 없다. 스플라인 점이 0개 또는 1개일 때도 안전한 정상 순찰이라고 가정할 수 없다.

BT_Task_Patrol의 현재 ExecuteTask는 다음 인덱스를 먼저 갱신하고 GetMoveTo로 좌표를 얻는다. 필요하면 허용 반경 안에서 NavMesh상 무작위 지점을 구하여 목표를 조정하고, 최종 위치를 Blackboard TargetLoc에 기록한 뒤 곧바로 Succeeded를 반환한다. 따라서 이 태스크는 현재 구현상 “다음 순찰 목표를 만드는 태스크”이고 자체적으로 그 지점에 도착할 때까지 기다리지 않는다. MoveToLocation·ReceiveMoveCompleted 구독 코드는 주석 처리되어 있으며 아래의 Tick·완료 콜백은 이전 이동 방식을 남긴 코드이다. 실제 이동은 BT 에셋에 이어지는 이동 노드가 담당하는지 확인해야 한다.

### 6.2 동적 목표 이동과 스트레이프

BT_Task_MoveToDynamicTargetLoc는 Vector Blackboard 키를 받아 MoveToLocation을 요청하고, 성공 요청이면 InProgress를 반환하는 사용자 정의 이동 노드이다. 의도된 Tick 동작은 경로 완료를 확인하고, Blackboard 목표가 기존 경로 목적지와 달라지면 다시 이동 요청을 내리는 것이다. AbortTask에서는 StopMovement를 호출한다. 다만 현재 생성자는 bNotifyTick을 켜지 않아서 소스에 적힌 Tick 완료·목표 갱신 로직이 자동 실행되지 않는다. AlreadyAtGoal도 즉시 성공으로 처리하지 않고 Failed로 떨어진다. 이 설명은 현재 코드의 제공 기능과 실제 활성화 누락을 같이 기록한 것이다.

BT_Task_SurroudingMove는 타겟을 Focus로 지정하고 적의 이동 방향 회전을 끈 뒤 좌우 방향을 무작위로 고른다. Tick마다 타겟이 아직 자신을 바라보는지 검사하고, 타겟 방향과 UpVector의 외적으로 횡이동 방향을 만든다. AddMovementInput으로 움직이며 상대의 시선 각도 밖으로 벗어나면 Focus·이동·회전을 정리하고 성공한다. 이는 Nav 경로 기반 원형 궤도 이동이 아니라 현재 위치에서 직접 입력을 주는 횡이동이다. 장애물 회피와 지형에서의 안전한 궤적은 실제 실행 확인 대상이다.

시선 Decorator인 BT_Deco_IsTargetLookAtMe는 Blackboard에서 TargetActor를 읽고 같은 각도 도우미로 조건을 반환한다. FlowAbortMode=Self라는 의도는 있으나, 이 C++ Decorator가 타겟 시선 변화를 어떤 관찰 경로로 계속 재평가할지는 BT 연결을 봐야 한다. 스트레이프 Task의 방향·이전 회전 설정은 NodeMemory가 아닌 객체 멤버에 저장되어 있는데 노드 인스턴스화 설정이 없어 같은 BT를 쓰는 여러 AI 사이에서 공유될 수 있다. 직접 AbortTask 호출 후 종료 결과를 BT에 알리지 않는 경로도 존재한다. 따라서 이 노드들은 기능의 형태를 설명할 수 있는 구현이지만, 다수 적에서 안정적으로 완료되는 동작을 검증한 상태는 아니다.

### 6.3 주변 적에게 전달하는 경보

적이 InBattle로 전환하면 RaiseAlert를 호출한다. 이 함수는 자신의 주변 1,000cm를 Pawn 채널 구체 Overlap로 조회하고 AEnemy에게 RecieveAlert를 보낸다. RecieveAlert는 타겟 액터 자체를 공유하는 대신 타겟의 현재 위치를 RecieveTargetLoc에 넘긴다. 수신 적은 경계치 6과 해당 위치를 갖게 되어 강한 경계로 조사하도록 유도된다. “한 적이 발견하면 주변 적 전부가 즉시 같은 타겟을 공격한다”는 방식과 차이가 있다. 경보 후 전투 함성 몽타주가 있으면 ActionBegin과 재생도 수행한다.

전파는 분대 객체를 통해 모든 유닛에 보내는 것이 아니라 근처의 실제 Pawn 충돌 질의 결과를 대상으로 한다. 서로 다른 분대의 적도 가까우면 알림을 받을 수 있고, 풀에서 비활성화되거나 Controller가 없는 적은 수신 함수에서 빠져나온다. 몽타주를 바꾸면 AI가 공격을 시작하기 전 함성 때문에 행동 중 상태가 유지되는 시간도 바뀐다. RaiseAlert에서 RecieveAlert로 전달할 때 타겟 null 방어가 완전하지 않으므로 새로운 외부 호출을 추가할 때 현재 타겟을 보장하는 기존 Service 흐름과 함께 봐야 한다.

대표 근거: Character/Enemy/AI/Patrol/CPatrolPath.cpp:23, :35, :70; Character/Enemy/PatrolComponent.cpp:19, :39, :94; Character/Enemy/AI/Patrol/BT_Task_Patrol.cpp:17; Character/Enemy/AI/BT_Task_MoveToDynamicTargetLoc.cpp:11, :20, :52; Character/Enemy/AI/InBattle/BT_Task_SurroudingMove.cpp:26, :58; Character/Enemy/AI/InBattle/BT_Deco_IsTargetLookAtMe.cpp:27; Character/Enemy/Enemy.cpp:168, :235.

## 7. 적 종류별 차이와 공통 전투 기능의 사용

적 종류는 외형만 바꾸는 독립 클래스 목록이 아니라 세 갈래 상속 구조를 가진다. Standard → Warrior → HiveGuard/BroodCommander는 근접과 증원·땅굴 기반 계열이고, Pouncer → Hunter는 점프 공격과 조준 회피 계열이다. Spitter → Spewer → Titan은 산성 투사체와 여러 부위 레이어를 사용하는 계열이다. Charger와 Hulk는 AEnemy에서 직접 갈라진다. 따라서 부모의 TryNear 또는 PartInit을 재정의하는지에 따라 자식의 행동이 달라진다.

| 클래스 | 직접 부모 | 현재 C++에서 확인한 특징 |
| --- | --- | --- |
| AEnemy_Standard | AEnemy | Near 근접 공격, Middle 증원 호출, Far 땅굴 이동 |
| AEnemy_Warrior | Standard | Middle 행동은 false로 대체, 머리 파괴 후 출혈형 코어 체력과 주기 피해 |
| AEnemy_HiveGuard | Warrior | 상대가 자신을 조준한다고 판단하면 Middle에서 가드 몽타주와 GuardLoop |
| AEnemy_BroodCommander | Warrior | 상속 동작 사용 가능, 자체 CallWarrior는 빈 구현 |
| AEnemy_Pouncer | AEnemy | Near 근접 공격, Middle 포물선 점프 공격과 쿨타임 |
| AEnemy_Hunter | Pouncer | 부모 점프 공격에 더해 Far 구간에서 조준 반응 좌우 도약 회피 |
| AEnemy_Spitter | AEnemy | Near/Middle 산성 투사체, 코어 파괴 시 폭발 컴포넌트 실행 |
| AEnemy_Spewer | Spitter | Near를 근접 공격으로 대체, 머리 위/아래 피격 레이어, 꼬리 파괴 반응 |
| AEnemy_Titan | Spewer | 몸통·꼬리·팔다리의 다층 부위 선택, 산성 주머니 파괴 후 발사 중단 |
| AEnemy_Charger | AEnemy | 돌진용 필드와 속도 전환은 있으나 행동 시작·완료·취소는 빈 구현 |
| AHulk | AEnemy | C++ 추가 행동 없음; 별도 Hulk BP·BT·몽타주 에셋 존재 |

### 7.1 Standard, Warrior, HiveGuard

Standard의 Near 조건은 유효 타겟 존재이고 실제 행동은 공통 AttackMelee이다. Middle 조건은 MainGameMode와 EnemyReinforceManager가 존재하고 IsCallable이 true인지이다. TryCalling은 ActionBegin 후 CallingReinforce를 예약하고 호출 몽타주를 재생한다. CallingReinforce가 실행되면 GameMode의 매니저에게 요청한다. 여기서 등장 위치 callPoint는 호출한 적 위치가 아니라 PlayerController 0의 Pawn 위치이다. targetPoint는 현재 적의 타겟 위치를 사용하고, 타겟이 사라졌다면 자기 위치를 사용한다. 호출 애니메이션의 시작과 실제 증원 요청은 Notify로 분리되어 있다.

Far에서 Standard는 타겟 근처 300cm 안의 NavMesh 무작위 지점을 구하여 _burrowOutLoc을 저장한다. 땅굴 진입 몽타주에 예약한 BurrowIn은 이동을 중단·비활성화하고 중력 0, 숨김·충돌 해제·Tick 해제 후 기본 2초 타이머를 건다. BurrowOut은 저장 위치로 옮기고 Walking·중력 1·표시·충돌·Tick을 되돌리고 출현 몽타주와 기본 10초 준비 타이머를 실행한다. _burrowReady 필드는 바뀌지만 현재 TryBurrow의 진입 조건에서 검사하지 않아 그 값이 실제 재사용 제한으로 작동하는 연결은 완성되어 있지 않다.

Warrior는 Standard의 Middle 조건과 행동을 항상 false로 재정의하므로 부모의 증원 호출을 그대로 사용하지 않는다. 머리 부위 파괴 이벤트는 Warrior_Bleeding에 연결된다. 처음 실행될 때 코어 현재 체력을 _additiveHP 기본 200으로 설정하고, 기본 초당 70의 코어 피해를 예약한다. 여기의 추가 HP는 기존 체력에 더하는 연산이 아니라 대입이다. 계속 살아 있으면 1초 후 피해를 다시 예약한다. ResetUnit은 출혈 플래그를 false로 만든다. 이것은 부위 파괴 이후 몇 초 더 움직이는 전투 연출을 표현하는 코드 흐름이다.

HiveGuard는 Warrior의 Middle을 다시 재정의한다. GetIsTargetAimingMe가 true이면 StartGuard를 호출하고 가드 몽타주를 시작한다. 예약 콜백 GuardContinue는 조준 반응이 여전히 true이고 현재 활성 몽타주가 가드 몽타주일 때 GuardLoop 섹션으로 이동한다. 가드 중 실제 방어 수치 변경, 충돌 부위의 장갑 변화 또는 자세 때문에 맞지 않는지에 대한 추가 C++ 처리는 이 클래스에서 보이지 않는다. 따라서 확인된 구현은 조준 반응에 따른 가드 애니메이션 루프이며 방어 효과 전체는 BP·부위 데이터·애니메이션 상태를 확인해야 한다.

대표 근거: Character/Enemy/Units/Enemy_Standard.cpp:26, :84, :104, :136, :175; Character/Enemy/Units/Enemy_Warrior.cpp:13, :27, :33, :47; Character/Enemy/Units/Enemy_HiveGuard.cpp:16, :39, :62.

### 7.2 Pouncer, Hunter

Pouncer는 Near에서 공통 근접 공격을 사용하고 Middle에서는 타겟 존재, _isJumpReady, 전방 각도 조건을 검사한다. JumpAttack(AActor*)는 현재 적 위치에서 타겟 위치로 발사 방향을 계산하고 ActionBegin 후 준비 상태를 false로 바꾼다. 공격 DataAsset을 _curAttackData로 지정하고 근접 충돌 후보를 설정하며, 이동을 멈춘 뒤 LaunchCharacter(direction × _jumpPower)로 날린다. 기본 점프 파워는 1,000, 준비 복구 쿨타임은 5초이다. 점프 중에 실제 피해를 주는 것은 비행 자체가 아니라 몽타주 NotifyState가 활성화한 근접 충돌이다.

Pouncer에는 위치 입력형 JumpAttack(FVector)와 비행 시간 계산 함수도 따로 있다. 이 버전은 목표까지의 비행 시간을 구하여 몽타주 재생률을 길이/비행 시간으로 맞춘다. 현재 거리 Task가 호출하는 것은 타겟 액터형 TryMiddle 경로이므로 두 JumpAttack 버전이 모두 동일하게 사용 중이라고 볼 수는 없다. 포물선 도우미도 공용 AIActingHelperLibrary 버전과 이 클래스 내부 버전이 함께 있어, 점프 튜닝 시 실제 호출되는 계산 경로를 먼저 구별해야 한다.

Hunter는 Pouncer를 상속하여 Near/Middle 근접·점프 흐름을 이어받고 Far를 회피 행동으로 재정의한다. 자신을 향한 조준 판정이 true이면 TryDodge가 ActionBegin을 진행하고 타겟 방향의 좌우 외적 벡터를 만든다. 좌우를 무작위로 고른 뒤 위쪽 성분을 더해 정규화하고, _jumpPower의 절반으로 LaunchCharacter와 회피 몽타주를 실행한다. 별도의 회피 쿨타임은 이 함수에 없고, 행동 중 제한과 거리/조준 상태가 반복 진입을 좌우한다. 이름이 Hunter라고 해서 모든 거리에서 조준 회피가 발동하는 것은 아니며 실제 TryFar 가지 선택이 필요하다.

대표 근거: Character/Enemy/Units/Enemy_Pouncer.cpp:29, :74, :131, :174, :211; Character/Enemy/Units/Enemy_Pouncer.h:48; Character/Enemy/Units/Enemy_Hunter.cpp:18, :39.

### 7.3 Spitter, Spewer, Titan

Spitter는 Near/Middle에서 Spit을 사용한다. 전방에 타겟이 있는지와 발사 준비 상태를 검사하고, 입 소켓의 위치에서 타겟 위치에 55cm를 더한 지점까지 공용 포물선 계산을 수행한다. 계산에 사용하는 투사체 속도는 이 함수에서 3,500으로 고정되어 있다. 방향을 _spitDirection에 저장하고 예약 콜백을 SpitProjectile로 연결한 뒤 공격 DataAsset의 몽타주를 재생한다. 실제 SpawnActor는 예약 콜백 시점에 실행되며 Owner·Instigator를 자기 자신으로 설정한 AGunBulletBase 파생 투사체를 만든다.

SpitProjectile은 발사 후 _isSpitReady=false와 기본 3초 쿨타임을 건다. 준비 제한이 몽타주 시작이 아니라 실제 발사 시점에서 걸린다는 것이 특징이다. _errorDegree를 이용한 퍼짐 방향과 fireRotation을 계산하지만 SpawnActor에 전달하는 회전은 현재 _spitDirection.Rotation()이다. 그래서 Spewer가 설정한 15도 오차가 이 경로의 실제 발사 방향에 적용된다고 설명할 수 없다. 투사체 클래스의 실제 속도와 Spit 계산의 3,500도 별도 데이터이므로 값을 바꾸면 양쪽을 함께 확인해야 한다.

Spitter의 코어 파괴는 AcidbagDestroyed로 연결되며 _isExplode로 중복을 막고 ExplosionComponent::Explode를 호출한다. 머리와 사지 파괴는 Critical에 연결된다. Spitter::ResetUnit 구현은 Enemy_Pouncer.cpp에 정의되어 _isExplode를 리셋한다. 클래스 이름과 구현 파일 이름이 다르기 때문에 파일 이름만 검색하면 초기화 함수를 놓칠 수 있다.

Spewer는 Spitter의 Middle 산성 공격을 이어받지만 Near는 AttackMelee로 바꾼다. GetHittedPartStat은 Head 피격 위치의 Z가 해당 충돌 컴포넌트 중심보다 위인지 아래인지에 따라 PartStats[0] 또는 [1]을 고른다. PartInit은 머리 두 레이어와 몸통의 파괴를 Critical에, 꼬리 첫 레이어를 AcidbagDestroyed와 Critical 모두에 연결한다. “꼬리가 깨지면 산성 주머니가 폭발하고 치명 반응도 발생한다”는 C++ 이벤트 연결이며, 실제 어떤 Collider에 Tail 태그가 있는지는 콘텐츠를 봐야 한다.

Titan은 Spewer를 상속하고 더 많은 부위 레이어를 선택한다. 몸통은 피격 높이로 위/아래를 구분하고, 꼬리는 위쪽 레이어 또는 아직 살아 있는 주머니 레이어를 선택한 뒤 주머니가 깨졌으면 내부 레이어를 선택한다. 사지는 바깥 PartStats[0]의 HP가 남았으면 그것을, 없으면 내부 [1]을 사용한다. 산성 주머니가 파괴되면 부모의 폭발 대신 _spitTimer를 정리하고 _isSpitReady=false로 두어 산성 공격을 막는다. 사지 내부 파괴는 Titan_Bleeding으로 연결되고 코어 현재 HP를 기본 2,000으로 설정한다. Warrior와 달리 이 클래스에서 주기 출혈 피해를 예약하는 구현은 보이지 않는다.

각 부위 이벤트는 PartInit에서 데이터에 연결된다. 잘못된 레이어 수의 데이터는 단지 장갑 수치가 다르게 나오는 문제에 그치지 않고 [0]/[1]/[2] 인덱스 접근 전제에도 영향을 준다. 신규 종을 만들 때 부모 PartInit을 일부러 건너뛰고 ACharacterBase::PartInit을 바로 호출하는 기존 파생 클래스 방식도 살펴야 한다. 그렇게 하면 공통 코어 사망 연결은 유지하면서 부모가 연결한 부위 반응을 그대로 가져오지 않고 종에 맞게 새로 구성한다.

대표 근거: Character/Enemy/Units/Enemy_Spitter.cpp:86, :137, :158; Character/Enemy/Units/Enemy_Pouncer.cpp:262; Character/Enemy/Units/Enemy_Spewer.cpp:12, :31, :50; Character/Enemy/Units/Enemy_Titan.cpp:11, :26, :90, :97, :107.

### 7.4 아직 완성된 행동으로 설명할 수 없는 클래스

Charger는 돌진 공격 DataAsset, 종료·취소 몽타주, 기본 돌진 속도 1,000과 최대 회전 속도 50이라는 필드를 가진다. SetDefaultState와 SetChargingState는 이동 속도를 바꾼다. 그러나 TryNear·TryMiddle·TryFar와 조건 함수는 false이고 StartCharging·FinishCharging·CancelCharging은 빈 함수이다. BroodCommander의 CallWarrior도 빈 함수이다. Hulk는 C++에서 AEnemy 외에 추가 구현이 없지만 콘텐츠에 BP_Hulk, BP_Hulk_mk2, BT_Hulk, BB_Hulk와 여러 공격 몽타주가 있다. 이것은 에셋의 존재 증거이며 BP 내부 행동이 완성되었다는 증거는 아니다.

대표 근거: Character/Enemy/Units/Enemy_Charger.cpp:19, :44, :60; Character/Enemy/Units/Enemy_Charger.h:44; Character/Enemy/Units/Enemy_BroodCommander.cpp:13; Character/Enemy/Hulk.h:13.

## 8. 분대 풀, 명령과 스포너의 역할 분담

### 8.1 분대는 유닛 저장소와 명령 단위이다

AEnemySquad는 AActor이고 자체 루트·SpawnPoint를 가지지만 개별 적의 등장 위치를 직접 반복 생성하는 현재 중심 객체는 아니다. BeginPlay의 Init는 _unitList라는 “Enemy 클래스 → 개수” TMap을 읽어 모든 적을 원점에 AlwaysSpawn으로 만든다. 각 적에 SpawnDefaultController를 수행하고 AddToSquad로 분대 참조를 저장한 뒤 _unitPool에 넣는다. 초기 풀 구성이 끝나면 DeactivateAllUnits로 모두 숨기고 Controller를 분리한다. 분대 생성 시 병력을 미리 확보하고, 요청이 들어올 때 새 적 생성 비용을 줄이려는 설계이다.

_unitPool은 UPROPERTY 배열로 보관되고 적도 _squad를 가진다. 이 참조들은 멤버십과 재사용 관계이지, 모든 SpawnActor에 Owner가 분대나 매니저로 지정되어 부모가 파괴되면 자식이 자동 소멸하는 구조는 아니다. 현재 Init의 FActorSpawnParameters에는 Owner 설정이 없다. 매니저와 분대는 보관 배열을 통해 객체를 관리하고, 소멸보다는 비활성화와 준비 상태 전환이 일반적인 흐름이다. 월드 종료와 명시적 풀 정리 정책을 바꿀 때는 액터 Owner 개념과 논리적 소유자를 구별해야 한다.

분대의 ESquadState는 Stationed, Patrol, Search, Attack, Deactivate이다. Command_Search는 목표 위치와 Search 상태를 저장하고 모든 적의 캐시 Controller에 RecieveTargetLoc을 보낸다. Command_Attack은 타겟과 Attack 상태를 저장하고 RecieveTarget을 보낸다. 타겟이 null이면 Stationed로 대체한다. Command_Patrol은 경로를 저장하고 각 PatrolComponent에 전달하며 경로가 null이면 Stationed로 돌아간다. Command_Stationed는 순찰 경로를 제거하고 목표 위치를 보낸다. Deactivate 명령은 분대 상태만 바꾸며, 그 함수 자체가 모든 살아 있는 유닛을 숨기는 것은 아니다.

UnitSpawnAct는 이미 선택된 적이 월드에 등장할 때 분대 상태를 적용하는 접점이다. Stationed/Search이면 목표 위치를, Patrol이면 경로를, Attack이면 타겟을 전달한다. 명령은 비활성 유닛의 캐시 Controller에도 먼저 전달될 수 있고 실제 소환할 때 다시 전달된다. 이 구조 덕분에 스포너는 적 종류나 분대 임무를 자세히 몰라도 “재빙의 → 위치 설정 → UnitSpawnAct → Spawn”을 진행할 수 있다.

CheckExtraUnit은 IsReadyToSpawn이 true인 첫 유닛을 반환한다. IsActivatedSquad는 풀이 비었으면 활성으로 취급하고, 하나라도 준비되지 않은 유닛이 있으면 true이다. IsCallableSquad는 그 반대이므로 모든 유닛이 준비된 분대만 일반 호출 후보가 된다. 이는 “생존 적이 하나라도 없는 분대”가 아니라 “풀의 모든 유닛을 다시 꺼낼 수 있는 분대” 정책이다. CheckActivateUnitCount와 GetUnitFromPool에 쓰는 IsActivatedUnit은 현재 IsReadyToSpawn을 그대로 반환하여 이름과 의미가 반대이므로, 이 API는 현 코드 의미를 별도로 확인해야 한다.

분대 회수 의도는 Search/Attack 분대의 모든 현재 유닛이 Stay가 되었을 때 SquadReturn을 호출하는 것이다. 하지만 생성 시 Tick 가능 플래그를 false로 두고 명령에서 플래그만 true로 바꾸므로 Tick 등록이 자동 복구되는 흐름은 아니다. SquadReturn도 현재 Controller가 붙은 살아 있는 유닛을 건너뛰는 조건을 갖는다. 따라서 “탐색이 끝나면 분대가 자동으로 모두 풀로 돌아간다”는 완성된 실행 결과로 이해하면 안 된다. ReturnToSquad라는 별도 함수는 빈 구현이다.

대표 근거: Character/Enemy/EnemySquad.cpp:13, :47, :81, :106, :124, :142, :162, :183, :193, :243, :260, :283, :306, :341; Character/Enemy/EnemySquad.h:105.

### 8.2 스포너는 풀에서 적을 꺼내는 등장 장치이다

ASquadSpawner는 Scene Root, SpawnPoint, Navigation Invoker를 가진다. ActivateSpawner는 분대를 _squad에 저장하고 Invoker를 켠다. 기본 구현은 loc으로 액터를 옮기지 않으며 위치 이동은 Reinforce 파생 스포너에서 한다. DeactivateSpawner는 분대 참조를 비우고 Invoker를 끈다. IsActivatedSpawner는 별도 bool이 아니라 _squad가 null이 아닌지로 결정된다.

CallRemainUnit은 분대의 CheckExtraUnit으로 준비된 적을 하나 받는다. 적이 없거나 남은 소환 수가 0이면 CallFinishAction으로 종료한다. 적이 있으면 CombineController로 기존 AIController를 붙이고, GetSpawnPoint로 구한 NavMesh 지점에 이동시킨 뒤 분대 명령을 적용하고 Spawn으로 활성화한다. 이후 최소·최대 소환 간격 사이의 난수 시간으로 다음 CallRemainUnit을 예약한다. 준비된 유닛 순서에 따라 한 마리씩 등장하며, 별도의 목록으로 무작위 적 종류를 고르는 구조는 아니다.

GetSpawnPoint는 SpawnPoint의 월드 위치에서 _callRadius 안의 NavMesh상 지점을 구하고 실패하면 원래 위치를 반환한다. 기본 반경은 1,000cm이며 Factory는 0으로 바꾼다. 기본 소환 간격은 0.3~1초, 공장 파생은 4~6초이다. _maxSpawnCount·_curSpawnCount로 배치 개수 제한을 표현하려는 필드는 있지만 현재 소환 시작 시 초기화하거나 성공 때 감소하는 코드가 없어 “공장이 한 번에 정확히 4마리 생성”하는 동작은 이 설정만으로 보장되지 않는다.

AReinforceSquadSpawner는 ActivateSpawner에서 callPoint로 이동하고 부모 활성화를 거친 뒤 3초 후 SpawnUnits를 예약한다. 주석과 코드상 의도는 새 위치 주변의 Navigation Invoker가 NavMesh를 생성할 시간을 주는 것이다. 소환이 끝나면 CallFinishAction에서 DeactivateSpawner를 호출하여 이 스포너가 다음 호출에 사용 가능해진다. 3초는 실제 Nav 데이터 준비 완료를 확인한 결과가 아니라 고정 지연이며, 큰 지형과 많은 적에서 충분한지는 측정이 필요하다.

FactorySquadSpawner는 스포너이면서 파괴 가능한 전장 건물이다. 활성화 후 0.5초 간격 Tick에서 PlayerController 0의 Pawn과 거리를 재고 기본 3,000cm 안이면 SpawnUnits를 시도한다. _spawnerReady가 true일 때만 한 배치를 시작하고 바로 false로 바꾼 뒤 기본 180초 후 준비 상태를 복구한다. CallFinishAction은 빈 구현이라 배치 종료가 건물 비활성화를 뜻하지 않는다. 공장은 자기 자리에 계속 남아 가까운 플레이어가 있을 때 다음 배치를 시작하는 방식이다.

대표 근거: Character/Enemy/Squad/SquadSpawner.cpp:11, :28, :39, :46, :88; Character/Enemy/Squad/SquadSpawner.h:95; Character/Enemy/Squad/ReinforceSquadSpawner.cpp:6, :20; Character/Enemy/Squad/FactorySquadSpawner.cpp:7, :36, :48, :68.

## 9. 증원, 순찰, 주둔 매니저의 실제 연결

AMainGameMode::BeginPlay는 지정한 EnemyReinforceManager 클래스가 있으면 SpawnActor로 만든다. 순찰·주둔 매니저는 새로 생성하지 않고 월드의 각 클래스 액터를 검색하여 첫 번째를 저장한다. 즉 증원은 GameMode의 클래스 설정, 순찰·주둔은 레벨에 배치된 액터와 그 설정이 주요 진입점이다. 레벨에 같은 매니저가 여러 개 있으면 GameMode가 전부 집계하는 구현이 아니며, 매니저들의 BeginPlay는 각각 실행될 수 있다.

증원 매니저 Init는 _squadList의 클래스·개수로 분대를 생성하고 고정 5개의 AReinforceSquadSpawner를 생성하여 풀에 넣는다. TryReinforceCall은 호출 가능 여부, 비어 있는 스포너, 완전히 준비된 분대를 찾는다. 성공하면 호출권을 하나 줄이고 다음 호출을 막은 뒤 분대에 Search(targetPoint)를 보낸다. 타겟 액터까지 있으면 Attack으로 덮어쓴다. 스포너를 callPoint에 활성화하고 다음 호출 허용 기본 60초, 호출권 회복 기본 180초 타이머를 시작한다. 반환 bool은 요청을 받아 스포너를 배정했는지이며 “이미 모든 적이 등장했다”는 뜻은 아니다.

TryMissionCall은 같은 분대·스포너 풀을 쓰지만 호출권과 일반 증원 쿨타임을 검사하거나 소비하지 않는다. 임무상 반드시 필요한 등장 요청을 위한 우회 API의 형태이다. 이 API와 일반 호출 모두 소환 지연 동안 분대를 별도 예약 상태로 표시하지 않는다. IsCallableSquad가 유닛 준비만 보므로 소환 시작 전 짧은 간격으로 반복 요청하면 같은 분대를 다른 스포너가 잡을 여지가 있다. 현재 C++의 TryMissionCall 호출자는 TestCommandSubsystem의 시험 명령에서 확인되며, 실제 임무 웨이브의 자동 스케줄은 EnemyMissionEnemyCallManager에 구현되어 있지 않다.

호출권 최대 기본값은 2이다. AddCallableCount는 현재 FMath::Max(current+1,max)를 사용하므로 한 개씩 최대치까지 회복시키려는 설명과 현재 연산이 다르다. 이 함수를 이해할 때 설정 의도와 실제 증가량을 분리해야 한다. 호출권 수치와 _isCallable은 별개 조건이라 호출권이 남아도 60초 동안 일반 증원은 막힐 수 있다. 종별 조건 함수가 IsCallable을 읽는 이유는 불필요한 호출 몽타주를 시작하지 않기 위해서다.

순찰 매니저는 _squadList로 분대 풀을 만들고 분대 수만큼 Reinforce 스포너를 만든다. 5초 간격 Tick에서 _paths[i], _squadPool[i], _spawners[i]를 하나의 묶음으로 본다. 해당 분대가 호출 가능하고 스포너가 비활성 상태이면 경로 시작 위치를 구하고 Command_Patrol(path), ActivateSpawner(squad,start)를 수행한다. 경로와 분대·스포너가 배열 인덱스로 연결되므로 개수가 다르거나 순서가 바뀌면 같은 경로에 어떤 분대가 배정되는지도 바뀐다. 경로 개수보다 분대가 적으면 없는 인덱스는 건너뛴다.

주둔 매니저는 분대를 직접 Spawn하지 않고 FGarrisonData 배열에 편집기에서 지정한 Squad, Spawners, RellyPoint, objectiveID를 사용한다. InitGarrison은 집결 위치로 Stationed 명령을 보내고 연결된 공장 스포너들을 활성화하며 파괴 이벤트를 구독한다. 공장이 파괴되면 분대 포인터로 해당 데이터 묶음을 찾아 파괴 개수를 올리고, 모든 공장 수와 같아졌을 때 GameMode::OnObjectiveCleared(objectiveID)를 호출한다. 주둔지에 공장 여러 개가 같은 분대를 공급하고, 모두 파괴하면 하나의 목표로 집계하는 설계이다.

공장의 피해 경로는 FCDamageEvent일 때만 건물 전용 ProcessDamage로 이어진다. 맞은 컴포넌트가 Core 태그를 가지면 DemolitionDamage 20 이상, 다른 부위이면 40 이상으로 DestroyFactory를 호출한다. 일반 DamageAmount를 누적해 건물 체력을 깎는 방식이 아니다. DestroyFactory는 스포너를 비활성화하고 이벤트를 발송하며 액터 자체를 Destroy하지 않는다. 현재 중복 파괴 방지와 매니저 조회 실패 방어가 없어 재피격 시 문제가 될 수 있다. 공장 외형 파괴 애니메이션이나 잔해 연출은 이 C++ 흐름에서 확인한 내용과 BP 내용을 구별해야 한다.

구형 분대 구현도 남아 있다. ReinforcementSquad는 분대 자체의 CallUnit과 쿨타임을 갖지만 실제 SpawnUnit 호출이 주석 처리되어 있고, StationedSquad도 자체 공장 내구도·생성 타이머를 가지면서 스폰 호출이 주석 처리되어 있다. PatrolSquad·MissionSquad는 현재 추가 C++ 동작이 없는 파생 클래스이며 BT_Service_Squad와 EnemyMissionEnemyCallManager도 실질 동작이 비어 있다. 새 구현의 중심인 “범용 EnemySquad + 별도 Spawner + Manager”와 이전 “분대 클래스가 소환까지 담당” 흐름을 섞어 읽지 않는 것이 중요하다.

대표 근거: MainGameMode.cpp:22; Game/EnemyReinforceManager.cpp:18, :46, :76, :148, :159; Game/EnemyPatrolManager.cpp:30, :67, :101; Game/EnemyGarrisonManager.cpp:25, :41; Game/EnemyGarrisonManager.h:21; Character/Enemy/Squad/FactorySquadSpawner.cpp:91, :119, :143; Character/Enemy/Squad/ReinforcementSquad.cpp:78; Character/Enemy/Squad/StationedSquad.cpp:34; Character/Enemy/AI/Squad/BT_Service_Squad.cpp:18.

## 10. 내비게이션, 파괴 가능한 길과 랜드스케이프 자료

### 10.1 Invoker와 Dynamic NavMesh

프로젝트 설정은 RuntimeGeneration=Dynamic이며 bGenerateNavigationOnlyAroundNavigationInvokers=True이다. 이 조합은 전장 전체의 모든 Nav 타일을 항상 생성한다기보다 Invoker가 활성화된 액터 주변을 중심으로 생성하는 정책이다. 개별 AEnemy의 Invoker는 생성 반경 5,000·제거 반경 6,000cm로 설정하고 PossessedBy/UnPossessed에서 켜고 끈다. 스포너에도 Invoker가 있으며 증원 스포너는 먼저 위치를 옮기고 켠 뒤 적을 늦게 내보낸다. 따라서 스폰 지점·적 이동·Nav 생성 위치가 서로 연결되어 있다.

Config/DefaultEngine.ini에서 타일 크기는 8,000cm, 기본 CellSize는 400cm, CellHeight는 50cm, AgentRadius는 50cm, AgentHeight는 144cm, AgentMaxSlope는 80도이다. 동시 타일 생성 작업 상한은 1,024이고 고정 타일 풀 크기는 1,024이다. 이 값은 C++ 코드가 요청하는 Nav 경로 성공 가능성과 생성 작업량을 해석할 자료이다. 하지만 각 맵의 NavMesh 액터 override, NavBounds, 지형 경사, 실제 에이전트 크기, 적 BP 캡슐 크기를 보지 않았으므로 “현재 모든 적에게 정확한 설정” 또는 “현재 병목의 원인”이라고 결론 낼 수 없다.

특히 GetRandomPointInNavigableRadius가 실패할 때 원래 위치를 사용하는 스포너, 순찰 위치를 그대로 쓰는 Patrol Task, 고정 3초 지연이 함께 존재한다. 적이 없는 Nav 영역에 먼저 명령받는지, Invoker가 충분한 영역을 생성하는지, 생성 전 호출 때문에 투영이 실패하는지와 실제 경로가 막힌 것을 구별해야 한다. NavigationSystem null 방어가 없는 함수도 있어서 새 테스트 맵은 필요한 Nav 구성을 갖춘 상태인지부터 확인할 필요가 있다.

### 10.2 파괴 가능한 장애물의 경로 갱신

ADestructableObject는 Mesh와 Box Collision을 가지고 시작 체력 100을 설정한다. Box의 PreDestructionNavArea는 UNavArea_Destructable이고 이 Area의 기본 비용은 100, 표시 색은 빨강이다. OnTakeAnyDamage delegate에서 체력을 감소시키고 0 이하가 되면 DestroyObstacle을 호출한다. RequiredDestructionPower라는 설정 필드는 있지만 이 피해 누적 경로에서 검사하지 않는다.

DestroyObstacle은 중복을 막는 bIsDestroyed를 먼저 세우고 메쉬를 숨기며 메쉬·박스 충돌을 끈다. 박스 AreaClass를 PostDestructionNavArea로 바꾸고 UpdateNavigationBounds와 UpdateComponentTransforms를 호출하여 길의 변경을 알린다. 액터 자체를 제거하는 구현은 아니다. 이것은 “적이 벽을 판단하여 공격하고 통과한다” 전체 기능 중 장애물 쪽 피해·상태·Nav 갱신 부분이다. CNavQueryFilter는 존재하지만 생성자에 비용 규칙 또는 제외 규칙을 설정하지 않으므로 적의 장애물 공격 정책이 여기서 완성된 것으로 설명할 수 없다.

### 10.3 지형 파일 구조와 확인 범위

Content/Materials/AutoLandscapeMaterial에는 M_AutoLandScape와 MI_AutoLandScape, Grass/Soil/Slope/Default/Covered LayerInfo, 풀·나무 GrassType, RVT_Height와 RVT_LandScape, 높이 블렌드·트라이플래너·텍스처 반복 완화 관련 MaterialFunction이 있다. 별도로 Content/MWLandscapeAutoMaterial에는 Landscape/MASTER 재질, Meshes, Textures, Procedurals의 Grass/Rock/Snow/Dirt/Stone GrassType이 있다. 두 묶음에서 총 107개의 uasset을 확인했다. 이것은 자동 지형 재질·RVT·식생 구성 자료가 존재한다는 뜻이고, 현재 대표 맵이 두 재질 중 어느 것을 실제 쓰는지는 아직 확인하지 않았다.

Content/Level에는 Level_Proto, Level_Proto2, LeveL_Proto3, Lobby, TestMap이 있다. LeveL_Proto3는 별도의 __ExternalActors__/Level/LeveL_Proto3 아래에 429개 외부 액터 파일과 HLOD 관련 에셋을 가진다. EditorStartupMap은 Proto3, GameDefaultMap은 FirstPersonMap이다. 에디터에서 처음 보이는 전장과 패키지를 실행했을 때 들어갈 맵이 다를 수 있다는 설정 구조이다. 신규 맵의 World Partition 기본 설정이 false인 것과 기존 Proto3가 World Partition인지도 별개이다.

Saved/Logs/SGAProjectMain.log는 2026-05-20의 과거 기록이다. 당시 Proto3의 World Partition 초기화, 동일 에이전트 NavMesh 등록 중복 경고, Patrol Point is OFF-NAVMESH 경고 57,872회를 확인했다. 현재 코드가 같은 경고를 낸다고 실행 검증한 것은 아니다. 지형의 실제 크기, Landscape 컴포넌트 수, LOD, Nanite, 물리 충돌, RVT 연결, 식생 밀도, HLOD 상태, 그림자·재질 비용은 BP/맵 내부와 실행 프로파일을 보지 않았으므로 이 문서에서 확정하지 않는다.

대표 근거: Character/Enemy/Enemy.cpp:44, :104, :140; Character/Enemy/Squad/SquadSpawner.cpp:19; Config/DefaultEngine.ini:312, :364, :380; Object/Map/DestructableObject.cpp:13, :43, :54, :89; Navigation/NavArea/NavArea_Destructable.cpp:6; Character/Enemy/CNavQueryFilter.cpp:6; Saved/Logs/SGAProjectMain.log:1183, :1195, :1847.

## 11. 플레이어 상호작용에서 단말 입력까지

### 11.1 AInteractable과 선택 대상

AInteractable은 월드 상호작용 객체의 공통 기반이다. FInteractableInfo에 Gun/Sample/SupplyBox 등의 아이콘 타입과 표시 텍스트를 보관하고, Screen 공간 WidgetComponent에 BP_InteractionWidget을 지정한다. BeginPlay에서 위젯을 초기화하고 표시를 숨긴다. ShowDefaultMark는 객체가 가까이 있다는 기본 표시, ShowKeyButtonMark는 현재 우선 상호작용 대상 표시, HideMark는 범위 밖 숨김을 담당한다. 부모의 Interact는 빈 함수이므로 실제 획득·보급·단말 조작은 파생 객체가 구현해야 한다.

PlayerCharacter는 반경 500cm의 감지 Sphere와 100cm의 상호작용 Sphere를 따로 가진다. 감지 범위에 들어온 AInteractable은 _detectedItems에 저장하고 기본 마크를 보인다. 상호작용 범위의 객체는 _interactableItems에 저장한다. 시작 시 이미 겹친 객체가 있을 수 있으므로 다음 Tick의 CheckInitialOverlaps가 두 범위를 다시 수집한다. Tick의 FindBestItem은 완전히 같은 위치에 있는 객체를 먼저 고르고, 그 외에는 플레이어 전방과 객체 방향의 내적을 거리로 나눈 점수를 사용한다. 가까우면서 앞에 있는 객체를 우선한다.

사용자가 상호작용 입력을 누르면 PlayerCharacter::Interact는 _bestItem->Interact(this)를 호출한다. 실제 키는 Enhanced Input의 _interactAction·Mapping Context 에셋에 달려 있어 C++ 함수 이름만으로 “항상 E 키”라고 확정할 수 없다. 선택 마크와 실행 가능한 객체의 범위는 두 Sphere의 반경과 충돌 설정에 의해 결정되고, 단말의 _isInteractable 같은 객체별 조건은 해당 Interact 구현이 추가로 검사한다.

### 11.2 단말의 화면, 플레이어와 카메라 소유

ATerminalConsole은 AInteractable을 상속한다. World 공간 WidgetComponent, CamAnchor, LookAt, PlayerAnchor를 가지고 편집기에서 _curTask를 지정한다. BeginPlay는 작업이 제공하는 WidgetClass를 컴포넌트에 지정하고 위젯을 만든 뒤 InitializeTask를 호출하며 작업 완료 이벤트에 자기 OnTaskCompleted를 연결한다. 단말이 위젯 컴포넌트와 작업 참조를 보유하고, 조작 중인 플레이어는 _player로 저장한다.

Interact는 _isInteractable을 먼저 확인한다. 이미 조작자가 있으면 같은 플레이어의 재입력은 취소·리셋으로 처리하고 다른 플레이어의 입력은 무시한다. 아무도 조작하지 않으면 입력된 AHellDiver가 APlayerCharacter인지 캐스팅하여 저장하고 ActivateTerminalConsole을 실행한다. 따라서 일반 HellDiver만으로 구현된 비플레이어 캐릭터가 단말을 조작하는 흐름은 이 함수에 없다.

ActivateTerminalConsole은 플레이어의 BeginTerminalInputMode(this)를 호출하여 EActionState::InterActing과 _curTerminal을 설정한다. H_CharacterLoc::SetCharacterToGround는 PlayerAnchor 위 200cm에서 아래 5,000cm까지 Visibility Trace를 수행하고 지면+캡슐 반높이로 플레이어를 위치시킨다. 카메라는 별도 ACameraActor를 Spawn하여 CamAnchor에서 LookAt을 바라보게 하고, 기존 ViewTarget을 저장한 뒤 0.85초 Cubic Blend로 전환한다. 작업을 StartTask하고 화면을 Visible로 바꾼다.

W/A/S/D용 OnStrataKey 함수는 Stratagem 상태이면 스트라타젬 버퍼에 넣고, InterActing 상태이면 현재 단말의 ReceiveInput에 EKeys::W/A/S/D를 보낸다. 이름은 스트라타젬 입력 함수이지만 단말도 같은 입력 경로를 재사용한다. InterActing 중 일부 이동·시선·전투 입력 함수가 빠져나오도록 되어 있어 플레이어를 조작 화면에 묶는 효과가 있다. 단말 입력이 별도 운영체제 텍스트 입력 모드로 전환되는 것은 아니다.

ResetTerminalConsole은 플레이어의 상태를 None으로 되돌리고 _curTerminal을 비우며 작업을 리셋한다. 카메라를 원래 ViewTarget으로 Blend하고 임시 카메라를 Destroy한 뒤 _player를 비우고 현재 상호작용 가능 상태에 따라 마크를 표시한다. 여기서 임시 카메라의 생성·소멸은 명시되어 있지만 플레이어 사망, 단말 외부 Destroy, 레벨 전환 중 조작 종료 같은 모든 예외 정리 경로가 구현되어 있다고 가정할 수 없다.

대표 근거: Object/Interactable.cpp:12, :46, :56; Character/PlayerCharacter.cpp:93, :193, :203, :1999, :2161, :2173, :2181, :2228, :2287; Object/Map/TerminalConsole.cpp:18, :34, :49, :102, :118, :151; Helper/H_CharacterLoc.cpp:10.

## 12. 단말 작업의 입력 판정과 임무 오브젝트 이벤트

UTerminalTaskBase는 UDataAsset 기반 작업 객체이다. WidgetClass와 실행 중인 Widget 포인터, 완료 multicast delegate를 가진다. InitializeTask는 위젯을 저장하고 숨기고, StartTask는 보여주며, EndTask는 완료 이벤트를 발송한다. 기본 ResetTask·ReceiveInput은 빈 함수이고 실제 입력 과제는 파생 UCommandInputTask가 제공한다. Game/TerminalTaskBase.h라는 별도 빈 파일도 있지만 사용되는 구현은 Game/Mission/TerminalTaskBase이다.

UCommandInputTask는 _command라는 정답 FKey 배열과 _playerInputBuffer, UCommandWidget 포인터를 보관한다. 초기화에서 CommandWidget을 받아 화살표 슬롯을 만들고, 시작 때 입력 버퍼를 비운다. ReceiveInput은 키를 추가하고 CheckInputCombo를 호출한다. 버퍼가 정답과 완전히 같으면 EndTask와 위젯 OnCompleted를 실행한다. 그렇지 않으면 현재 입력이 정답의 앞부분과 맞는지 순서대로 검사하고 틀리면 버퍼와 위젯을 리셋하며, 맞으면 진행 표시를 갱신한다. UCommandWidget은 W/S/A에 맞춰 화살표 회전을 적용하고 D는 기본 방향을 사용한다.

작업 객체가 DataAsset이라는 점은 실행 상태 해석에 중요하다. 단말의 BeginPlay는 편집기에서 지정된 _curTask를 복제하지 않고 그대로 초기화한다. 따라서 같은 자산을 여러 단말이 사용하면 위젯·입력 버퍼·완료 구독을 공유할 수 있다. 이것은 “단말별 독립 작업 인스턴스”가 현재 C++에서 생성되는 구조와 다르다. 향후 작업 데이터를 분리할 때 정답·위젯 클래스 같은 정의와 입력 버퍼·조작자·delegate 같은 실행 상태를 나누면 단말 주변의 코드도 함께 바뀐다.

단말 OnTaskCompleted는 먼저 _missionCompletedEvent를 발송하고 _isInteractable=false로 바꾼 뒤 콘솔을 리셋한다. 작업 완료와 임무 완료라는 이름이 비슷하지만 두 delegate는 단계가 다르다. 작업 완료 이벤트는 CommandInputTask → TerminalConsole이고, 단말의 이벤트는 TerminalConsole → 그것을 포함한 월드 오브젝트이다. GameMode의 주 임무 완료 이벤트는 그 다음 목표 ID 판정 결과이다.

ATerminalOperable은 UChildActorComponent로 단말을 포함하는 일반 액터이다. BeginPlay에서 ChildActor를 ATerminalConsole로 캐스팅하여 단말의 이벤트에 가상 OnCommandCompleted를 연결한다. 부모 OnCommandCompleted는 빈 함수이므로 실제 효과는 송신탑과 탈출 비콘이 재정의한다. ChildActorComponent가 어느 콘솔 BP를 생성하는지는 에셋 설정이며, 클래스가 맞지 않으면 이 구독도 이루어지지 않는다.

ABroadcastTower는 GeometryCollection을 루트로 갖고 OnCommandCompleted에서 GameMode::OnObjectiveCleared(_objectiveID)를 호출한다. 단말 완료 외에 직접 파괴하는 경로도 있다. TakeDamage에서 DamageAmount가 3,000 이상이면 피격 위치에 ADestructFieldActor를 Spawn하고 ActivateField를 실행한 뒤 목표 완료를 보고하며 단말 ChildActorClass를 null로 설정한다. 이 경로는 일반 DamageAmount 한 번의 크기를 보는 방식이고, 공장처럼 DemolitionDamage를 검사하거나 체력을 누적하는 방식이 아니다.

ADestructFieldActor::ActivateField는 기본 반경 500, 강도 10,000의 URadialFalloff를 만들고 Field_ExternalClusterStrain에 적용하여 GeometryCollection의 클러스터 파괴를 유도한다. 실제 깨지는 크기와 강도, 잔해 움직임, 충돌은 GeometryCollection 설정에 달려 있다. Field 액터에 이 함수에서 LifeSpan을 설정하거나 바로 Destroy하는 후처리는 보이지 않는다. 송신탑 목표 완료 ID와 시각적 파괴 성공 여부가 같은 검사로 묶여 있는 것도 아니다.

ADropPlaneBeacon은 정적 메쉬와 단말 ChildActor를 가진다. OnCommandCompleted는 GameMode::CallEscapePlane을 호출한다. SetInteractable은 자기 bool을 바꾸는 것이 아니라 ChildActor 단말의 SetInteractable을 호출한다. GameMode가 주 목표 완료 후 EnableExtraction에서 비콘을 활성화하는 연결이 여기로 도착한다. 초기 비콘이 비활성인지 여부는 단말 BP의 초기 _isInteractable 설정까지 봐야 하며, C++ StartPlay가 항상 false로 만드는 것은 아니다.

대표 근거: Game/Mission/TerminalTaskBase.h:15; Game/Mission/TerminalTaskBase.cpp:6, :19, :24; Game/Mission/CommandInputTask.cpp:8, :23, :41, :47; UI/CommandWidget.cpp:8, :51; Object/Map/TerminalConsole.cpp:179; Object/Map/TerminalOperable.cpp:19; Object/Map/BroadcastTower.cpp:36, :70; Object/Map/DestructFieldActor.cpp:10; Object/Map/DropPlaneBeacon.cpp:18, :27.

## 13. 목표 완료, 탈출기와 전투 종료의 호출 순서

GameMode는 BeginPlay에서 GameInstance의 PreDeploymentState가 선택한 현재 MissionDataAsset을 받아 _missionProgress._curMission과 남은 시간을 설정한다. StartPlay에서는 월드의 DropPlaneBeacon을 찾아 저장하고 1초 주기 UpdateTimer를 시작한다. 단말, 송신탑, 공장 등은 전체 진행 상태를 직접 수정하지 않고 OnObjectiveCleared(FName)를 부른다. GameMode는 현재 Mission ID와 같으면 주 목표로 보고 ObjectiveCompleted·MissionCompleted 이벤트를 발송하고 완료 bool을 true로 만든 뒤 EnableExtraction을 호출한다. 유효한 선택 목표 ID이면 ObjectiveCompleted를 발송하고 완료된 선택 목표 집합에 추가한다. 이 집합은 TSet<FName>이므로 동일 목표 ID가 중복으로 들어가지는 않는다.

일반적인 성공 흐름은 “송신탑 단말 입력 완료 또는 파괴 → 목표 ID 전달 → 주 목표 일치 → 탈출 비콘 단말 활성화 → 비콘 입력 완료 → 항공기 생성”이다. CallEscapePlane은 설정된 _escapePlaneClass를 _planeSpawnLoc에 Yaw 90도로 Spawn하고 항공기의 탑승 이벤트에 OnHelldiverExtracted를 연결한 뒤 _escapePlane에 저장한다. _planeSpawnLoc이 비콘 위치에서 자동 계산되는 코드는 이 함수에 없다. 따라서 비콘 이동과 탈출기 착륙 위치 변경은 각각의 설정을 함께 봐야 한다.

AEscapePlane 생성자는 SkeletalMesh를 루트로 두고 UBoxComponent를 부착한다. BeginPlay는 Box 충돌과 Overlap 이벤트를 끈 다음 OnTriggerBoxOverlap을 구독하고 UPlaneAnimInstance에서 몽타주의 Landing 섹션을 재생한다. 즉 생성 직후 항공기가 있다고 해서 탑승 가능한 것은 아니다. 착륙 몽타주의 OpenHatch Notify가 UPlaneAnimInstance::AnimNotify_OpenHatch → AEscapePlane::EnableTriggerBox를 호출해야 Box가 QueryOnly와 Overlap 활성 상태가 되고 _isEscapeEnabled=true가 된다.

EnableTriggerBox 시점에 GameMode::IsTimeOver가 true이면 20초 후 이륙 타이머를 건다. GameMode의 UpdateTimer는 남은 시간이 0 이하인 것을 확인하면 임무 타이머를 끄고, 항공기가 이미 있으면 StartTimerToTakeOff를 호출하며 없으면 바로 CallEscapePlane을 호출한다. 시간 초과 후 생성한 항공기는 해치가 열릴 때 남은 20초를 시작한다. 기존 항공기에 대해서는 GameMode의 시간 초과 시점과 해치 개방 시점 둘 다 타이머 시작을 시도할 수 있지만 활성 타이머가 있으면 추가하지 않는다.

Box에 들어온 액터가 AHellDiver이고 탈출이 허용된 상태이며 아직 기록되지 않았으면 _helldiverExtractEvent를 발송하고 _extractedHellDivers에 저장한다. 현재 C++은 한 명이 들어오면 즉시 TakeOff를 호출한다. “첫 탑승 후 20초 동안 나머지 인원을 기다리는 방식”은 주석으로 남아 있고 현재 호출은 아니다. 이벤트가 GameMode::OnHelldiverExtracted에 도착하면 탑승 인원이 1 증가한다. Box에 들어왔다고 플레이어를 항공기 좌석에 Attach하거나 숨기는 코드는 이 함수에 없으므로 실제 탑승 시각 연출은 별도 콘텐츠 확인 대상이다.

TakeOff는 몽타주 TakeOff 섹션을 재생하고 _isEscapeEnabled=false로 두어 추가 탑승을 막는다. 전투 결과 화면은 이 순간 바로 여는 것이 아니라 항공기 몽타주의 Exit Notify가 UPlaneAnimInstance::AnimNotify_Exit를 통해 GameMode::EndBattle을 호출할 때 진행된다. Landing·TakeOff 섹션명, OpenHatch·Exit Notify 이름, BP_PlaneAnimInstance의 부모 클래스와 몽타주 설정은 기능 계약이다. 하나가 빠지면 탑승 허용 또는 결과 처리까지 이어지지 않을 수 있다.

EndBattle은 임무 타이머를 지우고 남은 시간 비율, 선택 목표, 탈출 인원, 주 임무 성공 여부, 현재 작전과 완료 임무 수로 MissionResult를 만든다. 보상 계산과 GameInstance::ApplyMissionResult 뒤 결과 Widget을 생성하고 보상 표시 완료 이벤트를 OpenLobby에 연결한다. OpenLobby는 Lobby 레벨을 연다. 이 문서의 범위에서 결과 UI의 세부 애니메이션과 작전 진행 데이터는 연결 지점까지만 다루며, 보상은 주 목표 100 경험치/500 requisition, 선택 목표당 50/200, 탈출 인원당 20/50, 시간 비율 100/400으로 구성되는 현재 상수를 확인할 수 있다.

대표 근거: MainGameMode.cpp:22, :66, :75, :99, :107, :121, :161, :177, :240, :245; Object/Map/EscapePlane.cpp:12, :25, :39, :51, :57, :73; Object/Map/PlaneAnimInstance.cpp:10, :18, :27.

## 14. 인접 월드 오브젝트와 수정을 이어갈 때의 경계

AShopActor도 AInteractable이므로 플레이어의 동일한 선택·Interact 경로로 들어온다. Interact는 ShopWidget을 만들고 게임 Mapping Context를 제거하여 Widget Context를 우선순위 10으로 추가한다. Viewport에 위젯을 올리고 GameAndUI 입력 모드, 위젯 포커스와 마우스 커서를 설정한다. 단말의 InterActing 상태 방식과 상점의 Mapping Context/UI 입력 방식은 서로 다른 구현이다. 상점 닫힘 후 복귀는 ShopWidget 계열까지 봐야 한다.

ACorpse는 풀 유닛과 달리 별도 임시 월드 액터이다. 공통 SpawnGhost가 현재 메쉬와 포즈를 복사하도록 생성하고, Corpse::BeginPlay는 수명을 20초로 둔다. InitCorpseMesh는 스켈레탈 메쉬·선택적 머티리얼·포즈를 복사한 뒤 Ragdoll 충돌과 물리를 활성화한다. 적의 Dead에서 원본이 래그돌이 되는 흐름, 원본이 재사용 풀로 돌아가는 흐름, 별도 Corpse가 잠시 남는 흐름을 서로 같은 생명주기로 이해하면 안 된다. 실제 SpawnGhost 호출 시점은 공통 캐릭터·콘텐츠 연결을 함께 봐야 한다.

수정을 시작할 때는 원하는 화면상의 결과가 어느 책임을 바꾸는지 먼저 정하는 것이 좋다. “더 멀리서 소리를 듣게 하기”는 감각 설정과 보고되는 소리 강도·리스너 갱신의 문제이고, “소리를 들으면 더 빨리 전투로 들어가기”는 경계치와 Service 우선순위의 문제이다. “특정 적이 중거리에서 새 공격을 쓰기”는 CheckAbleTryMiddle·TryMiddle, 몽타주, Notify, 공격 DataAsset과 BT 가지가 모두 연결된 변경이다. “증원 위치를 호출한 적 주변으로 옮기기”는 Standard::CallingReinforce의 callPoint와 스포너의 Nav 준비를 같이 봐야 한다.

“공장 네 개를 파괴해야 선택 목표 완료”는 FGarrisonData의 Spawners와 objectiveID 설정, 중복 파괴 방어, MissionDataAsset의 유효 ID가 연결된다. “단말 완료 후 여러 단계의 조작”은 CommandInputTask의 단일 입력 배열 완료 이후 새로운 작업 인스턴스·단계 전환을 추가하는 변경이다. “여러 명 탑승 후 이륙”은 EscapePlane의 첫 탑승 TakeOff와 타이머, _isEscapeEnabled, GameMode의 탈출 인원·보상까지 영향을 준다. UI나 애니메이션처럼 보이는 변경도 결과 판정의 이벤트 순서를 바꿀 수 있다.

현재 소스를 읽은 뒤 에디터에서 확인할 핵심 연결은 적 BP의 AIControllerClass·AutoPossessAI·BT·Blackboard·공격 DataAsset·부위 데이터, BT의 Service/Decorator/Task 배치, 각 몽타주의 CallFunc/AttackEnd/MeleeCollision Notify, 분대 BP의 _unitList와 매니저의 _squadList·_paths·_garrisonDatas, 단말 ChildActorClass와 작업 DataAsset·WidgetClass, 탈출기 AnimInstance·몽타주 섹션·Notify, Proto3의 Landscape·NavBounds·Recast override·Invoker이다. 이 문서에서 확인한 파일 존재와 C++ 호출 계약은 그 에디터 확인을 시작하는 지도이며, 아직 보지 않은 연결을 완성된 동작으로 대신 설명하지 않는다.

대표 근거: Object/Map/ShopActor.cpp:9; Character/CharacterBase.cpp:542; Object/Corpse.cpp:24, :39; Content/Blueprints/Enemy, Content/Blueprints/Object/Map, Content/Materials/AutoLandscapeMaterial.

