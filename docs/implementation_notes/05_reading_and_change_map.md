# 실행 시나리오와 변경 영향 찾아보기

## 총 한 발이 적 부위를 파괴할 때

첫째, PlayerCharacter가 발사 입력을 현재 ActionState와 WeaponType에 맞게 해석한다. 총이라면 캐릭터를 통해 AGunBase의 StartFire로 들어간다. GunFireComponent는 현재 모드와 타이머/볼트/점사 상태를 보고 TryFire를 호출한다. 총이 발사 가능하면 _fireEvent가 GunBase::Fire를 호출한다.

둘째, DamageComponent가 실제 피해 경로를 선택한다. 투사체라면 ProjectileDataAsset의 클래스를 생성하고 FGunProjectileData를 넣는다. AGunBulletBase는 HitBox 충돌 컴포넌트와 법선을 받아 부위 AV와 입사각 AP를 비교한다. 이 결과로 계속 관통할지, 멈출지, 도탄할지 정하고 FCDamageEvent를 만든다.

셋째, CharacterBase::TakeDamage가 ColComp의 부위 태그와 데이터 레이어를 연결한다. StatComponent는 장갑·내구도·폭발 면역을 계산해 부위 HP와 코어 전달 HP를 변경한다. HP 0 경계에서 부위 파괴 델리게이트가 호출되고 적 파생 클래스의 부위별 반응 또는 코어 사망으로 이어진다. 이어 AIController, 행동 상태, 충돌, 래그돌/풀 반환과 HUD가 같은 변화에 반응해야 한다.

이 시나리오의 읽기 경로는 PlayerCharacter → HellDiver → GunBase → GunFireComponent → ProjectileDamageComponent → GunBulletBase → CharacterBase::TakeDamage → StatComponent::ProcessDamage → Enemy 파생 클래스의 PartInit/부위 반응이다. 피해 수치가 이상하면 실제 무기 데이터와 탄 데이터의 중복부터 확인하고, 잘못된 부위가 파괴되면 HitBox 프로필·태그·부위 레이어 선택을 먼저 확인한다.

## 재장전을 취소하고 다른 무기로 바꿀 때

Reload 입력은 PlayerCharacter의 현재 조작 모드와 누른 시간을 거쳐 HellDiver/총의 Reload로 들어간다. 총은 조준과 발사를 멈추고 AmmoComponent가 캐릭터 Reload 상태와 몽타주를 시작한다. 애니메이션 섹션이 끝나면 OnReloadSectionEnded가 탄창 제거/삽입/볼트 단계를 진행시킨다.

무기 교체가 들어오면 이전 총의 재장전 몽타주와 발사를 중단하고 등 소켓에 보관한 뒤 새 총을 손에 든다. 동시에 캐릭터 WeaponState/AnimLayer와 PlayerCharacter의 HUD 바인딩이 바뀐다. 다시 이전 총을 꺼낼 때 남아 있는 _reloadStage와 약실/탄창 수가 그 총의 실제 재개 상태를 결정한다. 캐릭터 상태를 false로 바꾸는 것과 총 내부 단계 초기화는 같은 작업이 아니다.

읽기 경로는 PlayerCharacter의 Reload/SwitchGun 입력 → HellDiver::SwitchGun → GunBase::CancelReload → GunAmmoComponent::CancelReload/FinishReload → 인벤토리 보관/장착 → AnimLayer 갱신 → PlayerCharacter 전후 교체 알림이다. 잘못된 탄약 표시에는 이전 총 delegate 해제와 새 총 초기 Broadcast도 포함해 확인한다.

## 사망 후 플레이어 증원과 적 풀 재사용

플레이어 사망과 적 풀 회수는 둘 다 HP/상태 복구가 필요하지만 같은 흐름은 아니다. 플레이어는 사망 이벤트를 증원 매니저에 전달하고 남은 증원·드롭포드·컨트롤러 빙의·카메라/입력을 연결한다. 적은 EnemySquad가 캐릭터와 AIController를 캐시한 풀에서 대기/활성/회수를 다루며 재소환 시 ResetUnit과 컨트롤러 재연결이 중요하다.

읽기 경로는 StatComponent 코어 파괴 → CharacterBase::Dead → HellDiver/Enemy 파생 처리 → HelldiverReinforceManager 또는 EnemySquad → Spawn/ResetUnit → StatComponent/StateComponent Reset → Possess/행동 재시작이다. HP만 복구해도 이전 상태 비트, 경직, 타이머, 무기, 숨김, 충돌, AI 타겟이 남으면 정상 재사용이 아니다. 현재 각 클래스가 담당하는 초기화 항목을 먼저 읽고 복구 계약을 한곳에서 확인한다.

## 장치를 던져 센트리를 호출할 때

출격 준비에서 선택한 스트라타젬 ID는 PreDeploymentState에 들어 있고 전장 UStratagemComponent가 슬롯으로 복사한다. 방향키 조합이 슬롯 CDO의 입력 시퀀스와 맞으면 SelectStratagem/EquipStratagem이 손에 장치를 만든다. 실제 release에서 AThrowable::Throw가 이동을 시작하고 사용 횟수·쿨다운 기록으로 연결된다.

장치는 붙을 수 있는 표면에 닿으면 지연과 ETA를 준비한다. 방어형은 낙하 외피 ADropPod를 생성하고 착륙 시 내부 ASentryTurret을 생성하는 흐름을 따른다. 센트리는 전개/정렬 상태 후 Perception으로 적을 선택하고 총구 본을 회전시키며 자기 탄 데이터를 AGunBulletBase에 넣어 발사한다. 따라서 플레이어 총과 센트리는 발사자/조준은 달라도 최종 탄과 부위 피해 처리를 공유한다.

읽기 경로는 PreDeployState → StratagemComponent → PlayerCharacter::CheckStratagemInputCombo → HellDiver::EquipStratagem/OnThrowReleased → AStratagem::OnHit → DeployStratagem → DropPod 착륙/내부 생성 → SentryTurret → GunBulletBase → CharacterBase다. 호출 수치 변경은 ETA, 연출 몽타주, 포드 내부 생성 시점을 같이 확인한다.

## 단말 목표 완료에서 결과 저장까지

플레이어 감지/상호작용이 TerminalConsole을 선택하면 단말이 작업 위젯과 실행 상태를 준비하고 ActionState를 InterActing으로 바꾼다. 이 상태의 W/A/S/D는 캐릭터 이동 대신 작업 입력으로 전달된다. 작업 완료 이벤트가 TerminalConsole에 돌아오고 목표 ID를 GameMode에 전달한다.

MainGameMode는 목표 ID를 현재 임무 정의와 비교해 필수 목표 완료 여부와 선택 목표 완료 집합을 갱신한다. 필수 목표가 만족되면 탈출 비콘이 활성화되고 탈출 호출·항공기 도착·탑승·출발 연출을 거쳐 EndBattle로 들어간다. 정산은 FMissionResult에 만들어져 GameInstance의 경험치·통화·완료 미션·작전 기록에 적용되고 SaveGame 파일에 반영된다. 결과 UI는 이 결과를 표시하고 로비 복귀로 연결한다.

읽기 경로는 TerminalConsole → TerminalTask/CommandInputTask → MainGameMode::OnObjectiveCleared → DropPlaneBeacon/EscapePlane → PlaneAnimInstance → EndBattle/CalculateMissionReward → CGameInstance::ApplyMissionResult → CSaveGame → MissionResultWidget이다. 단말 완료를 바꾸면 목표 ID와 위젯 종료/입력 복구를, 보상 계산을 바꾸면 결과 스냅샷과 저장의 중복 처리도 확인한다.

## 기능을 바꿀 때 함께 열 파일

| 바꾸려는 기능 | 시작할 파일 | 반드시 같이 확인할 연결 |
|---|---|---|
| 이동 속도 자세 회피 | HellDiver StatComponent StateComponent | 캡슐 스태미나 AnimInstance 감속 배율 |
| 제자리 회전 거리 매칭 | CharacterAnimInstance CharacterAnimLayerInstance | MovementComponent 캐릭터 transform BP 노드 연결 |
| 조준 손 IK 무기 Layer | HellDiverAnimInstance GunBase HellDiver | 소켓 좌표계 카메라 조준점 스켈레톤 |
| 파쿠르 높이와 종류 | PakourComponent | 충돌 채널 MotionWarping 몽타주 지형 |
| 부위 장갑 피해 | UnitDataTable StatComponent CharacterBase | GunProjectileData HitBox 태그 적별 파괴 delegate |
| 상태 이상 | Abnormality 데이터 CharacterStateComponent | StatComponent 이동 속도 Tick 재사용 Reset |
| AI 감지와 타겟 | EnemyController H_Targetting | 팀 ID BB 키 감지 listener 갱신 |
| AI 공격과 종료 | BT Service Task Enemy 파생 클래스 | AnimNotify ActionBegin ActionEnd 상태 잠금 |
| 적 소환과 회수 | EnemySquad SquadSpawner 매니저 | HP Reset 캐시 컨트롤러 예약 타이머 NavMesh |
| 무기 추가 | GunDataTable GunBase | Ammo Damage Projectile Layer HUD 보유 ID |
| 재장전 변경 | GunAmmoComponent | 몽타주 섹션 종료 배낭 자원 캐릭터 상태 |
| 스트라타젬 추가 | StratagemComponent AStratagem | 입력 CDO 테이블 그룹 쿨다운 지원 Actor ETA |
| 로드아웃 저장 | PreDeploymentState CSaveGame CGameInstance | 출격 준비 패널 소유 여부 ID 유효성 캐릭터 초기화 |
| 임무와 작전 추가 | Mission Operation Objective DataAsset | 글로브 ID 목표 Actor GameMode 맵 |
| 보상 경험치 | MainGameMode CGameInstance PlayerCurrency | 결과 위젯 중복 정산 작전 완료 스냅샷 |
| 맵 이동과 UI | PreDeploymentFlow GameMode UIManager | 위젯 수명 입력 모드 세이브 선택 상태 |

## 보조 코드와 충돌 계약

Helper 폴더는 각 시스템의 계산을 분리한 코드다. H_Vector/H_Rotator는 벡터 이동과 회전 보조, H_Relation은 시야각·거리·목표 상대 위치·바닥 trace, H_Targetting은 후보 배열의 타겟 선택, AIActingHelperLibrary는 포물선 발사 방향과 상대 조준 여부/분산 방향 계산을 제공한다. 함수 이름과 enum에 모든 선택 방식이 있어도 일부 저HP/고HP/복수 타겟 함수는 주석이므로 실제 구현을 확인해야 한다.

프로젝트의 공통 충돌 이름은 Source의 SGAProjectMain.h와 Config/DefaultEngine.ini를 함께 읽는다. GameDamage trace는 ECC_GameTraceChannel2로 정의하고 Config의 Projectile 객체 채널은 1, GameDamageBody 객체 채널은 3이다. Source의 OBJECT_GAMEDAMAGEBODY 상수는 현재 Channel2로 정의되어 Config 이름과 차이가 있으므로 사용처를 확인해야 한다. HitBox 프로필과 부위 태그는 투사체와 부위 시스템을 연결하는 계약이고 GlobeOnly는 글로브 선택을 위한 별도 채널이다. 이름·채널 번호·BP 충돌 응답을 하나씩 바꾸면 일부 공격 또는 상호작용만 사라지는 원인이 될 수 있다.

Navigation/NavArea의 Destructable 계열은 파괴물 관련 경로 영역을 표현하는 타입이다. 타입 정의가 존재한다는 사실과 실제 레벨 NavModifier/경로 필터에서 해당 비용을 사용하는 것은 별도로 확인한다. 랜드스케이프의 재질/RVT/식생 설정과 AI 이동은 지형 충돌·NavMesh를 통해 만나므로 시각 머티리얼 변경과 이동 가능 범위 변경을 같은 것으로 취급하지 않는다.

## 개발용 콘솔 명령의 실제 동작

UTestCommandSubsystem은 초기화에서 ECVF_Cheat 콘솔 명령을 등록하고 Deinitialize에서 해제한다. 다음 표는 이름의 의도보다 현재 함수 본문을 기준으로 한다. 명령 사용이 허용되는 개발 실행 환경과 필요한 매니저/GameMode가 있어야 한다.

| 명령 | 현재 요청하는 동작 | 읽을 코드 |
|---|---|---|
| KillMe | 현재 Pawn에 표준 ApplyDamage 2000 요청 | TestCommandSubsystem::OnKillMe |
| SelfKnockDown | 현재 CharacterBase에 KnockDown 호출 | OnSelfKnockDown |
| MoveToLobby | AMainGameMode에 EndBattle 호출 | OnMoveLobby |
| MoveToMainGame | FirstPersonMap을 OpenLevel | OnMoveMainGame |
| CallEscapePlane | GameMode에 탈출기 호출 요청 | OnCallEscapePlane |
| CallEnemyReinforce | 플레이어 위치로 일반 증원 호출 요청 | OnCallEnemyReinforce |
| CallEnemyMissionReinforce | 플레이어 위치로 임무 증원 요청 | OnCallEnemyMissionReinforce |

KillMe는 커스텀 FCDamageEvent를 만드는 코드가 아니므로 부위 피해 경로를 검증하는 명령과 동일하지 않다. MoveToLobby도 이름과 달리 즉시 OpenLevel(Lobby)을 실행하는 함수가 아니라 임무 종료 흐름을 요청한다. MoveToMainGame은 미션/로드아웃 선택을 완성해 주지 않는다. 콘솔 명령을 통해 테스트를 할 때 이 초기 조건을 구분해야 한다. 정의 위치는 TestCommandSubsystem.cpp:14 이후와 각 On 함수다.

## 용어 찾아보기

| 용어 | 이 프로젝트에서의 의미 |
|---|---|
| Definition | DataTable DataAsset BP에 지정된 종류와 기본 규칙 |
| Runtime State | 현재 객체의 HP 타이머 타겟 입력 버퍼 진행 상태 |
| GameInstance | 맵 이동 동안 유지되는 공용 데이터와 선택의 창구 |
| GameMode | 현재 맵의 생성 규칙과 전투 진행 종료 담당 |
| SaveGame | 명시적으로 저장한 플레이어 기록 |
| Outer | UObject의 생성 및 소유 문맥 |
| Owner | Actor의 소유 관계 장비 및 발사자 연결 |
| Instigator | 피해나 행동을 일으킨 Pawn 문맥 |
| CDO | 클래스 기본 객체 스트라타젬 입력 조합을 읽는 대상 |
| Delegate | 발사 HP 변화 부위 파괴 목표 완료 같은 사건의 연결 |
| Possess | Controller가 Pawn을 제어하는 연결 |
| Blackboard | BehaviorTree가 공유하는 판단 입력과 행동 상태 값 |
| Service | BT 실행 중 감지값이나 전투 판단을 갱신하는 노드 |
| Task | 이동 공격 수색 등 실제 행동을 수행하는 BT 노드 |
| NodeMemory | 공유 BT 노드의 실행별 상태를 보관할 공간 |
| Montage | 재장전 공격 전환처럼 호출해서 재생하는 애니메이션 |
| Section | 몽타주 안에서 점프하거나 이어 재생할 구간 |
| Notify | 애니메이션 시점에서 게임 코드에 알리는 사건 |
| AnimLayer | 자세나 장비별 애니메이션 선택과 재생 계산 계층 |
| MotionWarping | 파쿠르 등의 몽타주 움직임을 목표 위치에 맞추는 보정 |
| Distance Matching | 이동 또는 정지 거리로 애니메이션 재생 시간을 맞추는 계산 |
| AP AV | 공격의 장갑 관통 레벨과 부위의 장갑 레벨 |
| Durability | 일반 피해와 내구도 피해를 섞는 부위 비율 |
| Core | 사망과 전체 생존 HP를 연결하는 중심 부위 |
| Overlap Hit | 겹침 이벤트와 blocking 충돌 이벤트의 별도 경로 |
| Pool Reset | 객체 재사용 전 상태를 초기화하는 계약 |
| Navigation Invoker | 주변 내비게이션 생성을 요청하는 위치 제공자 |
| ETA | 지원 공격 또는 포드 도착까지의 예상 남은 시간 |

## 읽은 뒤 확인할 질문

캐릭터의 자세와 입력 모드는 어느 객체에 있는가. 실제 총의 피해 숫자는 어느 데이터에서 오는가. 부위 파괴는 어떤 이벤트를 통해 이동/공격/사망을 바꾸는가. BT Task가 행동 종료를 어떤 신호로 확인하는가. 분대가 회수될 때 어떤 캐릭터 상태와 컨트롤러 연결이 초기화되는가. 출격 선택 중 무엇이 맵 이동을 넘고 무엇이 앱 재시작을 넘는가. 임무 결과는 어느 시점에 저장되며 화면은 어느 객체의 값을 읽는가.

이 질문에 관련 클래스와 호출 순서로 답할 수 있으면 구현의 큰 연결을 다시 잡은 상태다. 이후 실제 에디터에서 사용 BP의 부모·기본값, AnimGraph Layer 연결, 몽타주 Section/Notify, DataTable 행, 배치 Actor의 참조, 충돌 프로필을 대조하면 소스와 콘텐츠의 연결까지 완성할 수 있다.

