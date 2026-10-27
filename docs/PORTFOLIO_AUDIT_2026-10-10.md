# SGAProjectMain 상태 점검과 포트폴리오 개선 계획

점검일: 2026-10-10, 한국 시간. 현재 작업 폴더의 수정·미추적 파일까지 포함한 점검이다. 게임 소스와 콘텐츠는 수정하지 않았다. 이 문서와 빌드·검증 로그를 생성했다.

## 1. 판단

포트폴리오로 발전시킬 구현 소재는 충분하다. 부위별 장갑·내구도·코어 피해 전달, 상태 이상, 감지 기반 AI, 분대 풀링, 캐릭터 이동·애니메이션 계층이 이미 있다. 현재 가장 큰 문제는 기능 사이의 상태 전환과 수명주기 처리가 일관되지 않다는 점이다. 컴파일 성공만으로 사망·증원·재사용·모션 종료·임무 종료가 정상 동작한다고 볼 수 없다.

추천 방향은 **캐릭터·전투·애니메이션을 먼저 안정화하고, 부위 파괴와 AI 반응을 연결한 짧은 전투 데모를 완성한 뒤 전체 게임 흐름을 정리하는 것**이다. 기능을 늘리기 전에 이미 구현한 기능이 반복 실행에서도 정확하게 동작한다는 증거를 만드는 것이 우선이다.

사용자가 밝힌 기여 범위는 애니메이션, 캐릭터, AI, 랜드스케이프, 상태, 부위 파괴 시스템이다. 장비·UI·스트라타젬은 다른 팀원이 작성했다. 이는 사용자 설명에 따른 구분이며 Git 기록으로 개인 기여를 확정한 것은 아니다. 개선 순서는 사용자가 선택한 **캐릭터·전투·애니메이션 → AI → 게임플레이 전체**를 따른다. 팀원 시스템의 문제도 점검하되, 초기에는 자신의 기능을 시연하는 데 필요한 연동과 진입 경로를 우선 고친다.

## 2. 실제로 확인한 범위

| 항목 | 확인 결과 | 해석과 한계 |
|---|---|---|
| 엔진 | 프로젝트 EngineAssociation 5.4, 설치 엔진 UE 5.4.4 | 우선 이 버전에서 안정화하는 기준을 세울 수 있다. |
| 규모 | Source의 .cpp/.h/.cs 398개, 비어 있지 않은 줄 31,463개 | 파일 수에는 헤더와 Target/Build 설정이 포함된다. 전체 파일을 같은 깊이로 리뷰한 것은 아니다. |
| 콘텐츠 | 파일 9,041개, 약 8.63 GiB, .umap 38개 | 현재 폴더의 값이다. 실제 데모에서 사용하는 양이나 패키지 크기는 아니다. |
| 구조 | 하나의 Runtime 모듈에 캐릭터·전투·AI·임무·UI·데이터가 포함됨 | 당장 모듈 분할보다 클래스 책임과 실행 상태 분리가 중요하다. |
| 작업 상태 | 시작 시 추적 파일 24개 수정, AnimLayer/Movement/콘텐츠 등의 미추적 작업 존재 | 기존 변경을 보존했다. 리팩터링 전 현재 진행 중인 애니메이션 작업을 기록해야 한다. |
| 빌드 | SGAProjectMainEditor / Win64 / Development 성공, 종료 코드 0 | 기존 Intermediate를 사용한 증분 빌드다. 깨끗한 체크아웃, Game/Shipping, Cook/Package는 미검증이다. |
| 컴파일 경고 | 이번 빌드 출력에서 경고 20개 | CalculateDirection, Detach 계열, 일부 UI·메시 직접 접근의 deprecated API와 변수 가림 등이 있다. 전체 소스 경고 수는 아니다. |
| 에셋 로드·검증 | DataValidation 커맨드렛 성공, 종료 코드 0. MissionDataAsset 1개 검증. 엔진 요약 0 errors / 5 warnings | 대상은 Mission_Broadcast 1개이며, 게임플레이 논리 검증이나 전체 에셋 검증을 의미하지 않는다. 의존 에셋 로드 중 애니메이션 BP 경고도 확인됐다. |
| 과거 실행 기록 | 2026-05-20 로그의 구조체 초기화 오류 6개와 ensure 5개 확인 | 오늘 플레이로 재현한 기록은 아니다. 원인 코드는 현재도 남아 있는지 별도로 확인했다. |
| 문서·자동 검증 | 추적된 README/Markdown/CI YAML 없음. .github/workflows 폴더는 비어 있음 | Source에서 프로젝트 자동 테스트와 IsDataValid 구현도 찾지 못했다. 외부 테스트나 BP 테스트의 존재까지 부정하지 않는다. |

로그는 `SGAProjectMain/Saved/Logs/PortfolioAudit-Build.log`, `PortfolioAudit-DataValidation.log`에 있다. 과거 기록은 `SGAProjectMain/Saved/Logs/SGAProjectMain.log`이다. Saved는 Git ignore 대상이므로 검증 결과를 공유하려면 필요한 요약을 문서 또는 CI 결과로 남겨야 한다.

이번에 실행한 명령:

```powershell
& 'D:/ProgramFile/EpicGames/UE_5.4/Engine/Build/BatchFiles/Build.bat' SGAProjectMainEditor Win64 Development '-Project=D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/SGAProjectMain.uproject' -WaitMutex -NoHotReloadFromIDE '-Log=D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/Saved/Logs/PortfolioAudit-Build.log'

& 'D:/ProgramFile/EpicGames/UE_5.4/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/SGAProjectMain.uproject' -run=DataValidation '-AssetType=/Script/SGAProjectMain.MissionDataAsset' -unattended -NullRHI -nosound -nop4 -nosplash -stdout -FullStdOutLogOutput '-AbsLog=D:/Git/SGA_Team_LDK/SGAProjectMain/SGAProjectMain/Saved/Logs/PortfolioAudit-DataValidation.log'
```

실제 게임 화면, 조작감, 모든 Blueprint 그래프·배치값, 랜드스케이프의 시각 품질, 패키지 완주, CPU/GPU 성능은 확인하지 않았다. 아래의 ‘코드 확인’은 결함이 소스에 존재한다는 뜻이며 현재 데모에서 해당 경로를 사용하는지까지 보장하지 않는다.

아래 소스 경로의 기준은 `SGAProjectMain/Source/SGAProjectMain/`이고 줄번호는 점검 당시 현재 파일 기준이다.

## 3. 캐릭터·전투·애니메이션: 먼저 해결할 항목

| ID | 근거와 확인 수준 | 문제와 영향 | 권장 수정·완료 조건 |
|---|---|---|---|
| C01 | `Character/CharacterAnimInstance.cpp:397,415`, `.h:94,97`. 코드 확인 + 과거 ensure의 호출 스택 일치 | BlueprintThreadSafe 경로에서 OwningPawn->AddActorWorldRotation으로 액터·물리 상태를 변경한다. 과거 로그의 게임 스레드 관련 ensure 4개가 이 경로를 지목한다. | 애니메이션 작업 스레드는 값 계산만 수행하고 회전 적용은 게임 스레드 캐릭터/Movement 쪽에서 수행. 제자리 회전·조준 전환을 반복해 ensure와 회전 튐이 없는지 확인. 단순히 ThreadSafe 메타데이터를 붙이는 것으로 해결하지 않는다. |
| C02 | `Character/CharacterBase.cpp:129`, `Character/CharacterStateComponent.h:129`. 코드 확인 + 과거 바인딩 실패 로그 | Dynamic delegate 대상 RecieveMotionFinish에 UFUNCTION이 없다. Source에서 _motionFinished Broadcast도 찾지 못했고 FinishStateTransition은 비어 있다. | 동적 바인딩을 정상화하고 실제 모션 완료 발생 지점을 구현. 전환 완료·취소 후 상태와 입력 잠금이 반드시 복구되어야 한다. BP 별도 발생 경로는 에디터에서 확인. |
| C03 | `Character/CharacterStateComponent.cpp:103,274`, `Character/HellDiver/HellDiverStateComponent.cpp:11`. 코드 확인 | `if (!IsComponentTickEnabled()==false)`는 이미 켜져 있을 때만 true다. 파생 컴포넌트는 bCanEverTick=false로 생성한다. 기본 코드 경로에서 상태 이상 시간·누적치 감소가 동작하지 않는다. | Tick 가능 여부는 생성 시 확정하고 런타임에는 SetComponentTickEnabled만 사용. 기절·감속·지속 피해가 적용되고 정해진 정책으로 종료되는지 확인. |
| C04 | `Character/CharacterStateComponent.cpp:115–125,148,237–247`. 코드 확인 | 변경 후 prev/cur 상태를 읽고 감속을 매번 적용한다. 현재 속도에 0.75를 다시 곱하므로 중첩 피격마다 속도가 줄어든다. 감속과 무관한 상태 해제도 속도를 증가시킬 수 있다. | 변경 전후 상태 경계에서 효과 적용. 기본 속도×배율로 계산하고 지속시간 재적용 정책 명시. 반복 적용·비관련 상태 해제 후 기본 속도로 정확히 복구되어야 한다. |
| C05 | `Character/StatComponent.cpp:141–149`. 코드 확인 | StartRegen의 part와 layer 루프가 값 복사여서 원본 HP를 복구하지 않는다. ResetUnit/Spawn/풀 재사용 경로에 연결된다. | 참조로 수정하고 HP·부위·상태·타이머·행동 플래그를 포함한 Reset 계약을 정의. 사망→증원 또는 적 풀 회수→재사용을 반복해 정상 초기값 확인. |
| C06 | `Character/StatComponent.cpp:236–238`, `Character/UnitDataTable.h:60`. 코드 확인 | 코어 직접 피격이면 같은 HP에 부위 피해와 코어 전달 피해를 두 번 적용한다. 기본 영향도 1에서는 두 배가 될 수 있다. | 코어 직접 피해와 다른 부위의 전달 피해를 구분. 코어 직접 피격·부위 피격·폭발의 예상 HP를 계산값과 대조. 부위 파괴/사망 이벤트는 한 번만 발생. |
| C07 | `Character/CharacterBase.cpp:964–966`. 코드 확인, 데이터 조건부 | 부위 조회가 nullptr을 반환할 수 있지만 바로 역참조한다. HitBox 태그와 유닛 데이터가 어긋나면 크래시 경로가 있다. | 유효한 코어로 fallback 또는 피해 거부·오류 기록. DataTable/HitBox 태그를 로딩 전에 검증하고 잘못된 데이터 테스트도 통과. |
| C08 | `Character/StatComponent.cpp:137,236–238`, `Character/PlayerCharacter.cpp:1874`. 코드 확인 | HP 변화 이벤트는 ChangeCoreHp에서만 발생하지만 피해·일부 회복은 ChangeHp로 처리한다. HP 표시가 실제 값과 달라질 수 있다. | 코어 HP 변경 지점과 이벤트 발행을 통일. 피격·치료·부활의 실제 HP와 HUD가 일치해야 한다. BP 별도 갱신은 미검증. |
| C09 | `Gun/Component/Damage/HitscanDamageComponent.cpp:37`, `Character/CharacterBase.cpp:933`. 코드 확인, 사용 경로 조건부 | Hitscan은 ApplyPointDamage를 쓰지만 캐릭터 HP 처리 경로는 FCDamageEvent에 한정한다. 기본 C++ 경로의 피해 이벤트 형식이 다르다. | 표준/커스텀 피해 이벤트를 공통 처리로 연결. 실제 사용 무기와 BP 추가 처리를 확인한 뒤 hitscan·projectile·explosion을 같은 계약으로 검증. |
| C10 | `Character/CharacterAnimLayerInstance.h:17–23`, `Data/ShopItemTable.h:44,53`. 코드 확인 + 과거 초기화 오류 | 방향별 애니메이션 포인터 4개와 상점의 정수 필드 2개가 명시적으로 초기화되지 않는다. | nullptr/정수 기본값을 지정하고 필수 에셋 누락 검증. 오늘 제한된 커맨드렛에서는 해당 초기화 오류를 재현하지 않았으므로 과거 오류와 구분. |

이번 에셋 로드에서도 `ABP_Helldiver_MK5`의 hand_l Two Bone IK에서 스레드 안전하지 않은 프로퍼티 접근/Is Valid 경고, `ABP_Helldiver_Layer`의 Sequence Player 프로퍼티 경로에 None이 있다는 경고가 발생했다. 근거: `PortfolioAudit-DataValidation.log:1815–1818`, `:2952–2956`. 작업 스레드에서 필요한 데이터를 게임 스레드에서 복사해 사용하는 구조와 함께 BP 연결을 점검해야 한다.

추가 정리 대상:

- `HellDiverStateComponent.cpp:292`의 IsMovable은 모든 경로 false이며 현재 C++ 호출은 찾지 못했다. `:356,366`의 모션/무기 변경 함수도 임시 false 반환 상태다. 사용 에셋 확인 후 정상 구현 또는 미사용 API 정리.
- `Character/HellDiver/PakourComponent.cpp`는 벽 탐색 실패 시 이전 HitResult를 재사용할 가능성이 있다. 매 시도마다 결과를 초기화하고 탐색 성공 여부를 후속 계산에 전달. 실제 재현 필요.
- `Character/CharacterBase.cpp:718`의 애니메이션 인터페이스 무검사 사용, `GunFireComponent.cpp:23`의 빈 fireModes 접근, `GunBase.cpp:84–93`의 클래스/컴포넌트 생성 무검사 사용은 콘텐츠 누락 시 크래시 위험이다.
- `GunAmmoComponent.cpp:83–90,158–163,208–212`에서는 배낭 탄약이 필요한 무기가 자체 spare로 재장전을 허용하고 실제로는 배낭 자원만 소비하는 분기가 있다. initialSpare가 양수인 해당 무기 데이터에서 무료 재장전 가능. 장비 시스템 담당과 자원 허용/소비 계약을 맞춘다.
- `AnimNotify/ANS_CallFunctionInterval.cpp:11–21`은 비어 있다. 사용하는 애니메이션 에셋이 있으면 구현, 미사용이면 제거 후보.

## 4. AI: 캐릭터 안정화 다음에 해결할 항목

| ID | 근거와 확인 수준 | 문제와 영향 | 권장 수정·완료 조건 |
|---|---|---|---|
| A01 | `Character/Enemy/AI/BT_Task_MoveToDynamicTargetLoc.cpp:11–19,46,66`. 설치 UE 5.4 기본값까지 확인 | bNotifyTick을 켜지 않고 InProgress를 반환하며 종료 처리는 TickTask에만 있다. 해당 노드를 사용하면 태스크가 끝나지 않는 경로다. | Tick 활성화 또는 이동 완료 이벤트 사용. 도착·이미 도착·경로 실패·타겟 소실·중단 모두 종료 보장. |
| A02 | `Character/Enemy/AI/InBattle/BT_Task_SurroudingMove.h:29`, `.cpp:48,71–88`. 코드 및 엔진 기본값 확인 | 공유 BT 노드 멤버에 적별 이동/회전 상태를 저장한다. 여러 적이 같은 BT를 쓰면 덮어쓴다. 직접 AbortTask를 호출하고 반환값을 버리는 경로도 있다. | NodeMemory 또는 노드 인스턴스에 개별 상태 저장. 정상 종료·실패·중단에서 공통 정리. 같은 BT를 사용하는 다수 적으로 검증. |
| A03 | `Character/Enemy/EnemySquad.cpp:16,112,172,314`. 코드·엔진 Tick 등록 방식 확인 | Tick 불가능 상태로 생성 후 플래그만 바꿔 회수 검사가 등록되지 않는다. SquadReturn은 컨트롤러 있는 생존 유닛을 건너뛴다. | Tick/회수 타이머와 생존 판정 수정. 생성→대기→예약→활성→회수→재사용 상태를 명시하고 유닛 Reset과 함께 검증. |
| A04 | `Character/Enemy/Squad/FactorySquadSpawner.cpp:60–65`, `Game/EnemyGarrisonManager.cpp:43–48`. 코드 확인, 재피격 조건부 | 공장 파괴 후 재피격되면 nullptr 분대로 파괴 이벤트를 다시 보내고 매니저가 조회 결과를 역참조할 수 있다. | 파괴 전환을 한 번만 처리하고 충돌/타이머 해제. 매니저의 null·중복 이벤트 방어. 같은 공장을 반복 공격해 임무 카운트가 한 번만 증가. |
| A05 | `Character/Enemy/Squad/SquadSpawner.h:112`, `.cpp:58`. 코드 확인 | _curSpawnCount는 -1이며 최대치 초기화/성공 소환 차감 코드가 없다. 공장의 최대 4개 설정이 적용되지 않는다. | 배치별 카운트 초기화·차감과 무제한 정책 구분. 실제 생성 수와 설정 일치. |
| A06 | `Game/EnemyReinforceManager.cpp:161`. 코드 확인 | 재충전에 Max(current+1,max)를 사용해 최대치로 바로 충전하거나 최대치를 넘긴다. | Min/Clamp로 범위 제한. 0→1→2 및 최대치 변경 검증. |
| A07 | `SGAProjectMain/Saved/Logs/SGAProjectMain.log:1195,1847`. 2026-05-20 과거 실행 기록 | NavData RegistrationFailed_AgentAlreadySupported와 Patrol Point is OFF-NAVMESH 경고가 기록돼 있다. 후자는 57,872회. 당시 내비게이션 실패·로그 폭증의 근거이며 오늘 재현한 것은 아니다. | 현재 대표 맵에서 순찰 위치 투영 실패 재현→중복 NavMesh 확인→Invoker 생성 범위/준비 시간 검증. 실패 시 원시 위치로 매번 이동 요청하기보다 재시도/대체 위치 정책 수립. |

추가 항목:

- `Controller/EnemyController.cpp:164,426`: 시야 소실 후 실제 타겟 액터를 계속 추적한다. 마지막으로 본 위치를 수색하도록 할지 게임 디자인을 결정한다. 단순 버그로 단정하지 않는다.
- `Controller/EnemyController.cpp:349`: 감지 범위를 바꿔도 listener 갱신이 없고 경계 전환에서 호출되는 흐름이 부족하다. 시각·청각·피해 감지별 전환을 확인.
- `EnemySquad.cpp:200,346`: IsActivatedUnit과 풀 선택 함수의 의미가 반대지만 현재 C++ 호출은 찾지 못했다. 공개 API 정리 대상.
- `Game/EnemyReinforceManager.cpp:46`: 지연 소환 전에 분대를 예약하지 않아 동시 요청이 같은 분대를 선택할 수 있다. 현재 임무 요청 C++ 호출은 테스트 콘솔뿐이므로 실제 사용 여부 확인.
- `Enemy_Charger.cpp:19` 이후 돌진 관련 함수, `Enemy_BroodCommander.cpp:13`의 CallWarrior는 미구현이다. 초기 데모에는 완성된 적 2~3종을 선정하고, 포함할 종류만 행동을 완성한다.
- 각 적 공격 DataAsset·부위 배열·Nav 시스템·주둔지 설정의 null/인덱스 검사도 추가해야 한다.

## 5. 팀원 시스템과의 연동 및 전체 진행 문제

초기 캐릭터 데모를 막는 문제는 먼저 최소 수정하되, UI·장비·스트라타젬의 큰 구조 변경은 마지막 단계로 둔다.

| ID | 근거 | 문제·권장 수정 |
|---|---|---|
| G01 | `CGameInstance.cpp:263–288` | 손상/다른 클래스 세이브 로드가 null이면 LoadGame→SaveGame→LoadGame 재귀. 로드 실패와 새 데이터 생성을 별도로 처리하고 저장 실패·버전·ID 검증 추가. |
| G02 | `CGameInstance.cpp:277–278`, `UI/PreDeployment/PreDeployStratagemPanel.cpp:209` | 신규 세이브는 총 2개만 소유. 소유 스트라타젬이 없는 상태에서 빈 슬롯을 열면 sections[0]/entries[0] 무검사 접근. 빈 목록을 정상 화면 상태로 지원. |
| G03 | `UI/PreDeployment/PreDeployHubWidget.cpp:135` | 열린 장비 패널 없이 섹션 이동 입력을 하면 _curPanel null 접근. 입력 가능 상태와 패널 수명을 맞춤. |
| G04 | `MainGameMode.cpp:121,197–236`, `CGameInstance.cpp:388–391` | EndBattle 중복 호출 방어가 없고 결과 배열·합계에 누적해 보상 재지급 가능. 종료 가드·결과 초기화·임무 실행 ID 정산으로 한 번만 처리. |
| G05 | `SGAProjectMain/Config/DefaultEngine.ini:73,80,82`, `Game/PreDeployment/PreDeploymentFlow.cpp:80`, `MainGameMode.cpp:31–35,125,161–178` | 에디터 시작은 LeveL_Proto3, 게임 기본 시작/출격은 FirstPersonMap. 선택 임무 없이 시작하면 남은 시간 0과 결과 복귀 실패 경로가 있다. 패키지 시작을 로비로 통일하고 전투 진입 전 선택 임무 검증. 실제 맵/BP 추가 초기화는 미검증. |
| G06 | `CGameInstance.cpp:56,63,166,173,302,309` | 일부 테이블/행 조회를 무검사 사용. 누락된 설정과 오래된 세이브의 ID에서 실패할 수 있다. 성공 여부를 반환하는 조회 계약과 로드 시 정합성 검사. |
| G07 | `Game/Mission/TerminalTaskBase.h:16,37`, `Object/Map/TerminalConsole.cpp:39–44,120` | DataAsset에 단말별 위젯·입력·delegate를 저장한다. 같은 작업 자산을 여러 단말이 쓰면 상태/이벤트를 공유하고 미사용 단말에서 null player 접근 가능. 작업 정의 DataAsset과 단말별 실행 UObject 분리. |
| G08 | `CGameInstance.cpp:403–406`, `UI/MissionResultWidget.cpp:174–186` | 마지막 임무/실패에서 작전 상태를 결과 UI보다 먼저 초기화해 완료 체크가 사라진다. 결과 객체에 완료 상태 스냅샷 저장. |

그 밖에 스트라타젬 목록을 ranged-for 중 Remove하는 코드(`PreDeployStratagemPanel.cpp:94–99`), 상점 해금 조건 항상 true/잘못된 구매 ID 처리(`CGameInstance.cpp:480,498,517`), 재시작 시 로드아웃 초기화(`PreDeploymentState.h:64`, `.cpp:12`)도 확인했다. Scene UI의 첫 열기 null 접근(`UIManager.cpp:103,110,127`)은 현재 PlayerCharacter 호출이 주석이라 잠재 문제로 분류한다. GameInstanceSubsystem이 보관한 위젯의 맵 이동 수명도 실행 확인이 필요하다.

## 6. 구조 개선의 목표

1. **피해 계산과 결과 적용을 분리한다.** 장갑·관통·내구도·폭발·부위 전달을 계산해 결과를 만들고, HP·부위 파괴·상태·사망 이벤트는 명확한 적용 단계에서 한 번 처리한다. 자신의 부위 파괴 시스템을 설명하고 테스트하기 쉬워진다.
2. **캐릭터와 풀의 Reset 계약을 만든다.** HP/부위/상태/타이머/행동/무기/AI/이벤트를 어디서 복구하는지 명시한다. 죽음과 회수 후에도 같은 계약으로 초기화한다.
3. **애니메이션 계산과 게임 상태 변경을 분리한다.** 작업 스레드에 전달할 데이터는 게임 스레드에서 준비하고, 액터 회전·충돌·이동 변경은 게임 스레드가 적용한다. AnimLayer/Movement 리팩터링은 이 경계를 중심으로 정리한다.
4. **공유 정의와 개별 실행 상태를 분리한다.** DataAsset은 정의를 보관하고 단말·AI·임무 진행은 각 실행 인스턴스가 상태를 보관한다.
5. **큰 클래스를 역할별로 나눈다.** PlayerCharacter.cpp 2,565줄, HellDiver.cpp 1,291줄, CharacterBase.cpp 1,088줄이다. 입력→행동, 상호작용, 조준·카메라, HUD 연동부터 작은 단위로 옮기고 한 번에 전체를 재작성하지 않는다.

처음부터 GAS 도입, 모듈 전체 분할, 멀티플레이 확장을 완료 조건으로 삼을 필요는 없다. 현재 기능의 실행 계약을 정리한 뒤 확장 가치와 비용을 판단한다. Source에서 복제/RPC 구현은 찾지 못했으며 네트워크 플레이는 이번에 검증하지 않았다.

## 7. 실행 순서와 단계별 완료 기준

| 단계 | 작업 | 완료 기준 |
|---|---|---|
| 0. 기준 상태 기록 | 현재 애니메이션 작업을 백업/커밋 가능한 단위로 정리, 사용 캐릭터/BP/맵을 지정, 알려진 오류 목록과 조작법 기록 | 같은 소스 상태와 UE 5.4.4에서 빌드·동일 테스트 맵 진입을 재현할 수 있음. 필요한 UI 빈 목록/진입 크래시도 해소. |
| 1. 캐릭터·전투 정확성 | C03~C09, 피해·상태·HP/부위 Reset와 이벤트 일관성 | 코어/팔다리/장갑 경계/상태 중첩/치료/사망/증원에서 예상 수치와 결과 일치. 반복 재사용 정상. |
| 2. 애니메이션·조작 완성도 | C01/C02/C10, BP 경고, 전환 완료·취소, 이동·제자리 회전·조준·재장전·파쿠르 조합 | 스레드 ensure 없이 전환, 입력 잠금 복구, 무기/손 IK 정합성. 30/60/120fps 조건에서 결과와 조작 반응 확인. |
| 3. AI 완성도 | A01~A07, 소환·회수, 감지/수색/전투 상태, 데모 적 2~3종 | BT 중단/종료가 보장되고 동일 BT를 쓰는 여러 적의 상태가 독립. 지형에서 순찰·접근이 정상이고 부위 파괴가 공격·이동·AI 행동 변화로 연결됨. |
| 4. 랜드스케이프·성능 | 데모 경로·엄폐·단차·파쿠르 지점 구성, 내비게이션 검증, Insights/CPU/GPU 측정 | 데모 경로에서 적 이동·회수 정상. 목표 하드웨어/해상도/적 수와 Game/Render/GPU 시간, 시작 시간, 메모리 기록. 수정 전후 비교. |
| 5. 게임 전체 완주 | G01~G08와 로드아웃/상점/결과 정리, 로비→출격→임무→결과→로비→재실행 | 신규·기존·손상 세이브와 성공·실패를 포함해 Standalone 및 패키지에서 완주. 중복 정산 없음. |
| 6. 제출물 구성 | 실행 파일, 소개 영상, 기술 문서, README, 기여/에셋 출처 목록 | 다른 사람이 설치·실행·조작·핵심 기술·본인 기여를 이해하고 재현할 수 있음. |

랜드스케이프는 바이너리 맵 내부와 화면을 직접 검사하지 않아 지형 배치·머티리얼·LOD·조명 품질을 평가할 수 없다. 자동 지형 머티리얼, LayerInfo, GrassType, RVT 리소스가 존재하지만 실제 맵의 사용 여부는 미검증이다. 현재 Config에는 Dynamic NavMesh와 Navigation Invoker 사용, TileSizeUU=8000cm/CellSize=400cm/CellHeight=50cm/AgentMaxSlope=80°가 설정되어 있고 과거 로그에 경사/스텝 설정 경고가 있다(`DefaultEngine.ini:318–333,364,380`). 레벨별 Recast override가 있을 수 있으므로 Config만 보고 실제 내비게이션 품질을 단정하지 않는다. 파쿠르 높이·플레이어 캡슐·적 크기와 지형 단차를 같이 검증해야 한다. 과거 Proto3 로그에는 World Partition 맵으로 나타나므로 대표 맵의 World Partition/Recast 설정도 직접 확인한다.

초기 성능 검증은 같은 장면에서 적 20/50/100마리, 동일 하드웨어·해상도·설정으로 비교한다. 빈 Tick, 긴 화면 디버그 출력, 미리보기 SceneCapture, 시작 시 전체 풀 생성은 측정 후보다. 현재 프레임 속도나 병목은 측정하지 않았으므로 확정하지 않는다.

## 8. 먼저 만들 회귀 검증

| 검증 | 기대 결과 |
|---|---|
| 코어 직접 피격/다른 부위 피격 | 코어 피해 한 번 적용, 전달 비율 정확, 부위/사망 이벤트 한 번 |
| 장갑 관통 낮음/같음/높음, 내구도 경계, 폭발 | 정의한 공식과 실제 HP 변화 일치 |
| 감속 반복 적용, 기절 추가, 서로 다른 순서로 해제 | 정책에 따른 중첩·시간 갱신, 기본 속도/입력으로 정상 복구 |
| 사망→풀 반환/증원→재사용 반복 | HP·부위·상태·타이머·행동·AI가 초기 상태로 복구 |
| 모션 전환 완료/취소, 조준/재장전/무기 교체 | 잠금이 남지 않고 애니메이션과 행동 상태 일치 |
| 동일 BT를 쓰는 다수 적, 타겟 소실/이동 실패/중단 | 적별 상태 독립, 모든 태스크 종료 가능 |
| 같은 공장 반복 파괴, 같은 단말 자산을 두 액터가 사용 | 크래시·중복 카운트·공유 입력 없음 |
| 신규/손상 세이브, 장비 없음, 잘못된 아이템/부위 ID | 무한 재귀/인덱스 오류 없이 정해진 복구 정책 실행 |
| 임무 종료 이벤트 두 번, 마지막 작전 완료/실패 | 보상 한 번 지급, 결과 화면의 완료 상태 정확 |

피해 계산·상태 배율·정산처럼 입력/출력을 명확히 할 수 있는 부분은 작은 Automation Test가 효과적이다. 애니메이션·풀링·BT·맵 이동은 테스트 맵에서 통합 검증한다. 검증 코드가 구현을 그대로 복제하지 않도록 예상 결과를 독립적으로 정의한다.

## 9. 포트폴리오에서 보여줄 내용

핵심 주제는 세 가지로 좁히는 것이 좋다.

- **캐릭터·애니메이션:** 이동/조준/제자리 회전/전환 취소/파쿠르, 게임 스레드와 애니메이션 계산 경계, 수정 전후 동작 비교.
- **부위 파괴·상태·전투:** 장갑/내구도/코어 전달의 규칙, 부위 파괴에 따른 이동·공격 변화, 상태 중첩 정책과 자동 검증 결과.
- **AI·전장:** 감지→수색→교전→회수, 적별 행동 차이, 지형/엄폐/단차와의 연동, 다수 적의 측정 수치.

추천 시연은 하나의 작은 전장에서 이동·조준 → 부위 공격 → 상태 변화 → AI 대응 → 사망/증원 또는 재사용 → 목표 완료로 이어지는 3~5분 분량이다. 전체 제출 영상에는 로비/장비 선택/출격/결과도 짧게 포함한다. 장비·UI·스트라타젬은 팀 구현임을 명시하고 본인이 수정한 연동 부분은 따로 기록한다.

README에는 엔진/도구 버전, 빌드 방법, 시작 맵, 실행·조작법, 핵심 기능, 본인/팀원 기여, 외부 에셋 출처·사용 조건 확인 결과, 알려진 한계, 테스트·성능 측정을 적는다. Content에는 Starter/Lyra/ALS/HELLDIVERS_2 등 여러 출처로 보이는 폴더가 있으나 파일명만으로 실제 출처나 공개 가능 여부를 판단하지 않았다. 배포 전 각 에셋의 원출처와 공개 조건을 확인하고 필요한 교체·제외를 결정해야 한다.

첫 실제 수정 묶음으로는 **C03/C04/C05/C06/C08: 상태 이상 Tick·감속·HP 복구·코어 피해·HP 이벤트 정상화**를 권한다. 서로 연결된 작은 범위이며, 본인 담당 기능의 정확성을 개선하고 독립적인 수치 검증을 만들 수 있다. 애니메이션 제자리 회전 경로를 사용 중이라면 C01의 스레드 문제도 초기 안정화에 포함한다.
