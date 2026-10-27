# 3. 게임 진행, UI, 세이브와 데이터의 실제 연결

## 3.1 이 장을 읽는 기준

이 장은 팀원이 작성한 화면과 장비 선택 코드까지 포함해, 플레이어가 로비에서 임무를 고르고 전투를 마친 뒤 다시 시작할 때 어떤 객체가 어떤 정보를 전달하는지 설명한다. 소스에서 확인한 구현을 기준으로 작성했으며 Blueprint 그래프와 레벨 배치값을 직접 열어 확인하지는 않았다. 따라서 C++의 실행 순서와 필드의 의미는 설명할 수 있지만, 특정 맵에 어떤 Blueprint가 실제 배치되었는지와 화면의 최종 디자인은 별도 확인 대상이다. 무기·인벤토리·스트라타젬의 내부 동작은 해당 장에서 설명하고, 여기서는 선택한 데이터가 그 시스템으로 들어가는 연결을 다룬다.

이 장의 소스 경로는 저장소의 `SGAProjectMain/Source/SGAProjectMain/`를 기준으로 한다. 예를 들어 `CGameInstance.cpp:31`은 그 폴더 아래 파일의 31번째 줄이다. Config와 Content 경로는 저장소의 `SGAProjectMain/`를 기준으로 쓴다. 모든 줄번호는 2026-10-10 현재 작업 폴더 기준이다. 함수 이름에 Mission과 Operation이 섞여 있으므로, 이 설명에서는 Operation을 여러 미션을 묶는 **작전**, Mission을 한 번 출격해서 수행하는 **임무**, Objective를 그 임무 안의 **목표**라고 구분한다.

전체 연결은 `GameInstance의 공용 데이터와 세이브 → 로비의 작전·임무 선택 → PreDeploymentState의 현재 선택 → 출격 준비 UI의 로드아웃 변경 → 새 전투 맵의 GameMode와 캐릭터 초기화 → 목표·탈출 진행 → MissionResult 정산 → SaveGame 갱신 → 로비 복귀`이다. 화면이 다음 화면을 모두 직접 호출하는 구조는 아니다. 화면은 선택을 알리고, GameInstance는 맵 사이에 정보를 보관하며, GameMode는 현재 맵의 실행 결과를 결정한다.

## 3.2 객체의 생성, 소유와 수명

구조를 기억할 때 가장 먼저 구분해야 하는 것은 앱을 켠 동안 살아 있는 객체, 맵 하나에서만 살아 있는 객체, 디스크에 기록되는 데이터다.

| 객체 | 생성·참조 방식 | 실제 책임과 수명 |
|---|---|---|
| UCGameInstance | Config의 BP_GameInstance로 지정되고 엔진이 생성 | 맵 이동을 넘어 데이터 테이블, 세이브 객체, 출격 상태를 보관한다. 앱 재시작 시 새로 생성된다. |
| UCSaveGame | LoadGameFromSlot 또는 CreateSaveGameObject | 메모리의 플레이어 기록이며 SaveGameToSlot을 호출해야 파일에 반영된다. GameInstance의 UPROPERTY가 참조한다. |
| UPreDeploymentState | GameInstance::Init에서 NewObject, Outer는 GameInstance | 선택한 작전·임무·무기·스트라타젬과 작전 내 완료 상태를 보관한다. 맵 이동 동안 유지되지만 모든 필드가 세이브되는 것은 아니다. |
| ALobbyGameMode / AMainGameMode | 해당 맵의 GameMode 설정으로 생성 | 로비의 출격 가능 상태, 전투의 시간·목표·탈출·정산을 처리한다. OpenLevel로 맵을 바꾸면 새로운 GameMode가 필요하다. |
| AMainPlayerController | GameMode/BP 설정으로 생성되는 PlayerController | 기본 입력 MappingContext를 추가하고 Possess한 Pawn의 팀 ID를 맞춘다. 구체적인 캐릭터 행동과 다수 UI 연결은 PlayerCharacter에 있다. |
| UPreDeploymentFlow | 헬포드가 첫 상호작용에서 NewObject, Outer는 헬포드 | 플레이어·미리보기·출격 화면·입력 모드 변경을 묶는 실행 객체다. 헬포드는 UPROPERTY로 이 객체를 보관한다. |
| UUIManager | GameInstanceSubsystem으로 엔진이 관리 | 이름으로 팝업 클래스를 찾고 화면에 생성한 위젯을 스택에 보관한다. 모든 UI가 이 관리자를 이용하지는 않는다. |
| 글로브·상점·단말·탈출기 | 맵 배치 또는 SpawnActor | 월드 안의 상호작용·연출 객체다. 연결할 클래스, 에셋, 위치는 주로 Blueprint의 EditAnywhere 속성으로 설정한다. |

이 구조에서 UPROPERTY는 객체 참조를 Unreal의 리플렉션과 GC가 추적하도록 하는 역할이다. UPROPERTY로 보관했다고 곧바로 디스크에 영구 저장되는 것은 아니다. 예를 들어 GameInstance가 출격 상태를 강하게 보관하므로 로비에서 고른 총은 전투 맵에서도 읽을 수 있다. 그러나 그 총 ID를 UCSaveGame 필드에 넣고 저장하는 코드가 없으면 앱 종료 후에는 복원되지 않는다. 반대로 작전 진행은 클래스 포인터 자체 대신 작전 ID와 완료 미션 ID로 저장하여, 시작 시 DataAsset을 다시 찾는 방식이다. 근거는 `CGameInstance.cpp:31`, `CGameInstance.h:224`, `Game/PreDeployment/PreDeploymentState.h:64`, `Object/Map/PreDeploymentHellpod.cpp:27`이다.

## 3.3 GameInstance가 제공하는 데이터 창구

UCGameInstance의 Init은 Super 호출 뒤 세이브를 로드하고, 출격 상태를 만들고, 유닛 데이터·상태 이상 데이터·작전과 미션 데이터를 캐싱한다. 이 순서로 먼저 영속 기록을 확보하고 다른 시스템이 조회할 공용 데이터 색인을 준비한다. 다만 Init 자체에서는 출격 상태에 세이브의 작전 기록을 적용하지 않는다. 작전 진행 복원은 로비 GameMode의 BeginPlay에서 수행한다. 시작 시 세이브 객체가 존재하는 것과 현재 선택된 미션이 존재하는 것은 다른 조건이다.

총 조회 함수는 int32 ID를 문자열 RowName으로 바꾼 다음 FGunData 행을 찾는다. 같은 패턴으로 총 클래스와 미리보기 텍스처를 얻는다. 스트라타젬도 숫자 ID를 행 이름으로 사용하고 FStratagemSlot과 StratagemClass를 반환한다. 이 때문에 상점의 itemID, 세이브의 소유 ID, 출격 화면의 ID, DataTable의 행 이름이 맞아야 한다. 상점 테이블의 RowName은 직접 이용하지 않고 모든 행을 순회하여 shopType과 itemID를 비교하므로 총 테이블과 조회 규칙이 다르다. 근거는 `CGameInstance.cpp:53`, `:60`, `:299`, `:306`, `:517`이다.

유닛 데이터는 단순 조회보다 한 단계 가공한다. FUnitData의 기본 이동속도·전투 이동속도·저항·이름·설명·유닛 클래스를 FProcessedUnitData로 복사한다. 부위마다 들어 있는 PartID를 별도 PartDefinitionTable에서 찾아, 각 Layer의 HP·장갑·내구도·폭발 면역·코어 영향도를 FUnitPartStat 배열로 만든다. 최종 캐시는 숫자 ID가 아니라 유닛 클래스 TSubclassOf를 키로 사용한다. 즉 콘텐츠 작성자는 유닛 정의와 재사용 가능한 부위 정의를 나눠 관리하고, 실행 중 캐릭터는 이미 결합된 정의를 받아 자신의 StatComponent를 구성하는 설계다. 이 캐시는 공용 정의이며 각각의 적이 입은 현재 피해를 모두 여기서 공유하려는 구조는 아니다. 근거는 `CGameInstance.h:19`, `CGameInstance.cpp:73`, `:107`, `:140`, `:465`이다.

상태 이상 캐시는 한 행에서 두 가지 결과를 만든다. FProcessedAbnormalityDefinitionData에는 상태 종류·지속시간·가중치·이름을 넣고, 별도의 FCDamageEvent에는 일반 피해·내구도 피해·관통·DamageType을 넣는다. CharacterStateComponent가 지속 상태를 관리하면서 정의와 틱 피해를 따로 조회할 수 있는 연결이다. `CachingAbnormalityDataFromTable`과 GetProcessedAbnormalityDefinitionData/GetAbnormalDamageEventData가 이 실제 경로이며, `GetAbnormalDefinitionDataFromTable(FName)`은 현재 기본 구조체만 반환하는 빈 구현이다. 또한 InitializeUnitData라는 유사 가공 함수가 남아 있지만 Init의 해당 호출은 주석이고 현재 호출되는 쪽은 CachingUnitDataFromTable이다. 두 함수를 같은 실행 단계라고 읽으면 안 된다. 근거는 `CGameInstance.cpp:31`, `:209`, `:246`, `:412`이다.

작전·임무 DataAsset은 GameInstance의 `_operations`, `_missions` 배열에 연결하고, 각각의 ID를 키로 `_operationMap`, `_missionMap`에 넣는다. 배열에 있는 콘텐츠를 색인으로 바꿀 뿐 AssetManager를 통해 모든 DataAsset을 자동 검색하거나 비동기로 로드하는 구현은 아니다. 따라서 Content 폴더에 새 미션 에셋을 만드는 것과 BP_GameInstance의 배열에 등록하는 것은 별도 작업이다. 같은 ID를 두 번 쓰면 TMap의 키가 겹친다. 데이터를 고칠 때 에셋 등록과 ID 정합성을 함께 보는 이유다.

## 3.4 작전·임무·목표·결과·통화 모델

| 모델 | 필드와 의미 | 읽는 주요 시스템 |
|---|---|---|
| UOperationDataAsset | 작전 ID·이름·설명, MissionDataAsset 배열, 완료 순서별 메달 배열 | 글로브 지역, 출격 상태, 임무 정산, 결과 UI |
| UMissionDataAsset | 미션 ID·이름·설명·아이콘, 선택 목표 배열, 제한 시간 | 글로브 미션 아이콘, MissionWidget, MainGameMode |
| UObjectiveDataAsset | 목표 ID·이름·설명·아이콘 | 선택 목표 유효성 검사와 HUD/결과 아이콘 |
| FMissionProgress | 현재 임무, 완료 선택 목표의 TSet, 주요 목표 성공 여부, 탈출 인원 | 한 전투 맵의 AMainGameMode |
| FMissionReward | 보상 분류, 경험치, RequisitionSlips | 결과 화면의 보상별 행 |
| FMissionResult | 작전·임무 포인터, 완료 선택 목표, 성공 여부, 탈출 인원, 작전 완료 개수, 시간 비율, 보상 행과 합계 | GameInstance 정산과 MissionResultWidget |
| FPlayerCurrency | RequisitionSlips, Medals, FSampleBundle | 세이브, 상점 가격·잔액, 임무 총보상 |
| FSampleBundle | Common/Rare/Super별 int32 수를 TMap으로 보관 | 수집 아이템, 인벤토리, SampleWidget, 세이브 통화 |
| FShopItemData / FOwnedItem | 판매 행에는 이름·가격·해금 조건·종류·아이콘·ID, 소유 항목에는 종류와 ID만 존재 | 상점 표시·구매, 소유 장비 기반 출격 목록 |

현재 MissionDataAsset에서 별도 `_mainObjective` 필드는 주석 처리되어 있다. MainGameMode는 전달된 objectiveID가 missionID와 같으면 주요 목표 완료로 처리한다. 즉 이 프로젝트의 주요 목표는 독립 ObjectiveDataAsset을 찾는 방식이 아니라 **미션 ID와 같은 ID를 가진 맵 목표가 완료를 통지하는 방식**이다. 선택 목표는 `_optionalObjectives` 안에 같은 objectiveID가 있는지 별도로 확인한다. 새 목표를 추가할 때 Blueprint의 `_objectiveID`만 바꾸고 미션/목표 DataAsset의 ID를 바꾸지 않으면 HUD와 정산에 들어가지 않는 이유가 이 비교 규칙이다. 근거는 `Data/MissionDataAsset.h:21`, `MainGameMode.cpp:75`이다.

UPreDeploymentState는 작전 내 각 MissionDataAsset 포인터에 None/Available/Cleared/Failed 중 하나의 상태를 붙인다. SetCurOperation은 미션 배열을 Available로 만들고 현재 선택 미션을 비운다. ApplySaveGameData는 현재 작전 ID를 GameInstance 색인에서 찾고, 세이브에 있는 완료 ID에 해당하는 항목을 Cleared로 바꾼다. 세이브에 실패 상태와 현재 선택 미션은 저장하지 않는다. 모든 미션이 Cleared면 작전 완료이고, 하나라도 Failed면 작전 실패이다. 완료 또는 실패한 작전은 GameInstance의 결과 적용에서 작전 ID와 완료 ID 집합을 지운다. 여기의 completedMissionIDs는 전체 플레이 기록 업적 목록이 아니라 **현재 진행 작전 안의 완료 목록**이다. 근거는 `Game/PreDeployment/PreDeploymentState.cpp:15`, `:56`, `:86`, `:101`, `CSaveGame.cpp:17`이다.

## 3.5 앱 시작과 PlayerController의 위치

Config는 GameInstanceClass를 BP_GameInstance, GlobalDefaultGameMode를 BP_MainGameMode로 지정한다. 에디터가 처음 여는 맵은 LeveL_Proto3이고 게임 기본 맵은 FirstPersonMap이다. 코드의 로비 복귀는 Lobby, 출격은 FirstPersonMap을 사용한다. 그러므로 프로젝트에 로비가 존재하는 것과 실행 파일이 처음 로비에서 시작하는 것은 현재 같은 사실이 아니다. 실제 맵 World Settings에서 GameMode를 덮어쓸 수 있으므로 배치값을 확인해야 하지만, 기억할 기준 경로는 `Config/DefaultEngine.ini:73`, `:79`, `:80`, `:82`와 `MainGameMode.cpp:245`, `Game/PreDeployment/PreDeploymentFlow.cpp:80`이다.

AMainPlayerController의 BeginPlay는 LocalPlayer의 EnhancedInputLocalPlayerSubsystem을 찾아 `_inputMappingContext`를 우선순위 0으로 추가한다. 생성자는 HellDiver 팀 ID를 만들고 OnPossess에서 Pawn이 IGenericTeamAgentInterface를 구현하면 그 팀 ID를 넣는다. PostInitializeComponents와 OnUnPossess에는 추가 실행 내용이 없다. 이 컨트롤러가 모든 이동·사격·메뉴 입력을 독자적으로 관리하는 것은 아니다. 캐릭터의 SetupPlayerInputComponent와 각 상호작용 객체/위젯이 EnhancedInputComponent에 행동을 바인딩한다. 근거는 `Controller/MainPlayerController.cpp:15`, `:25`, `:43`이다.

ASGAProjectMainPlayerController는 First Person Template에서 온 별도 클래스이며 MappingContext를 추가하는 기본 구현을 유지한다. ASGAProjectMainGameMode는 BP_FirstPersonCharacter를 기본 Pawn으로 지정하는 템플릿 구현이다. 이 둘은 AMainPlayerController/AMainGameMode와 이름이 비슷해도 서로 다른 클래스다. CameraContainActor는 CameraComponent를 Root로 만드는 간단한 카메라 보조 액터이며 BeginPlay와 Tick은 비어 있다. EnemyController와 SquadController의 행동 상태는 AI 장에서 설명한다.

플레이어 HUD는 PlayerCharacter의 PostInitializeComponents에서 각 위젯 클래스를 확인한 뒤 CreateWidget으로 생성한다. BeginPlay에서는 화면에 붙이고, 스태미나는 StatComponent 델리게이트를 연결한다. 미니맵과 스태미나는 처음에 Hidden, 스트라타젬은 생성 후 접힌 상태로 시작한다. 나침반·샘플·임무·타이머·플레이어 상태는 각 위젯이 별도 객체다. UIManager가 하나의 공통 HUD를 만드는 코드와 혼동하지 않아야 한다. 실제 PlayerCharacter의 SceneUI 호출은 주석 처리되어 있다. 근거는 `Character/PlayerCharacter.cpp:108`, `:154`, `:189`이다.

## 3.6 로비: 3D 글로브에서 임무를 선택하는 과정

ALobbyGameMode의 BeginPlay는 GameInstance의 GamePhase를 Lobby로 바꾸고, 플레이어가 있으면 저장된 샘플 수를 AddSample으로 넘긴다. 이어 출격 상태의 `_missionSelectedEvent`에 OnMissionSelected를 연결하고 ApplySaveGameData를 호출하여 작전을 복원한다. 맵에 있는 모든 PreDeploymentHellpod를 모아 두었다가 미션 선택 이벤트가 true이면 각 헬포드의 상호작용을 허용한다. 세이브에서 작전이 복원되어도 현재 미션은 null로 초기화되므로 다시 미션을 고르는 흐름이다. 근거는 `LobbyGameMode.cpp:17`, `:35`, `:51`, `:57`이다.

AGalacticPlanetGlobe는 화면에 표시하는 목록만 가진 클래스가 아니다. 상호작용 가능한 3D 글로브, Browse/Focus 카메라 ChildActor, 이동 앵커, 회전 밴드, 두 Timeline, 선택 링, 지역 ChildActor들을 보관한다. `_operations`의 각 FOperationData에는 위도·경도와 OperationSiteClass가 있다. InitializeOperations는 그 클래스의 ChildActorComponent를 런타임 생성하고 글로브 메시에 붙인다. 위도·경도를 삼각함수로 구면 좌표로 바꾸어 사이트를 놓고 표면 방향으로 회전시킨다. 작전 DataAsset 자체는 각 OperationSite의 BP 설정에 연결된다. 근거는 `Object/Map/GalacticPlanetGlobe.cpp:22`, `:110`, `:493`이다.

APlanetOperationSite는 지역 데칼, 전체 작전을 나타내는 mainIcon, 개별 미션용 IconA/B/C를 갖는다. BeginPlay에서 OperationDataAsset의 첫 세 미션을 각 아이콘에 연결한다. Browse에서는 mainIcon을 표시하고 개별 아이콘을 숨기며, Focus에서는 mainIcon 충돌을 끄고 개별 미션 아이콘을 표시한다. 데이터 배열에 미션을 더 넣을 수 있어도 현재 사이트 UI는 최대 세 개의 미션 표시를 전제로 한다. APlanetMissionIcon은 미션 데이터와 인덱스를 보관하는 메시 액터이고, APlanetSelectRing은 구 표면 위치·법선·접선으로 자신의 위치와 방향을 맞추는 액터다. 근거는 `Object/Map/PlanetOperationSite.cpp:49`, `:96`, `:126`, `Object/Map/PlanetSelectRing.cpp:25`이다.

글로브 BeginPlay는 링을 Spawn하고 링 메시 overlap을 OnIconInRange/OutOfRange에 연결한다. 링에 아이콘이 들어오면 Browse에서는 그 아이콘의 부모 사이트를 현재 사이트로 선택하고 Operation 설명을 표시한다. Focus에서는 아이콘 자체를 현재 미션으로 선택하고 Mission 설명을 표시한다. TickBrowseMode는 마우스 이동으로 글로브를 회전시키고 화면 중심을 월드 레이로 바꾼 뒤 글로브 전용 채널에 trace하여 링을 표면에 올린다. TickFocusMode는 지역 주변에서 링을 움직이고 글로브를 조금씩 기울인다. 사용자는 마우스 레이로 일반 UMG 버튼을 클릭하는 것이 아니라 링과 3D 아이콘의 충돌을 통해 후보를 바꾸는 셈이다. 근거는 `GalacticPlanetGlobe.cpp:395`, `:431`, `:538`이다.

상호작용 시작은 원래 플레이어 ViewTarget을 기억하고 글로브 카메라로 0.5초 블렌딩하며, 게임 MappingContext를 제거하고 글로브 MappingContext를 우선순위 10으로 넣는다. 선택 입력은 Browse에서 EnterFocus, Focus에서 SelectMission을 호출한다. EnterFocus는 잠시 actor Tick과 링 overlap을 끄고 링을 메시와 함께 움직이게 붙인 뒤 Timeline으로 글로브 위치·쿼터니언을 보간한다. 완료 콜백에서 overlap을 다시 켜고 링을 떼고 Tick을 복구한다. 뒤로 가기는 Focus에서 Browse, Browse에서 상호작용 종료이다. 근거는 `GalacticPlanetGlobe.cpp:238`, `:289`, `:502`, `:526`, `:574`이다.

SelectMission은 이미 Cleared인 미션 선택을 거부하고, 현재 사이트와 아이콘이 유효하면 StopInteracting을 거쳐 GameInstance::SetOperationAndMission으로 작전·임무를 전달한다. 이 함수는 작전 ID를 세이브에 기록하고 저장한 뒤 출격 상태를 세이브와 맞추고, 미션이 작전 배열에 포함되는지 확인하여 SetCurMission을 호출한다. SetCurMission은 `_missionSelectedEvent(bool)`를 Broadcast하고 로비 GameMode는 헬포드를 활성화한다. 작전 ID는 이 시점에 파일에 남지만 미션 포인터는 현재 앱의 출격 상태에만 남는다는 차이가 중요하다. 근거는 `GalacticPlanetGlobe.cpp:265`, `:357`, `CGameInstance.cpp:357`, `PreDeploymentState.cpp:76`이다.

## 3.7 헬포드와 출격 준비 화면

PreDeploymentHellpod::Interact는 자신이 상호작용 가능하고 플레이어가 유효할 때 상호작용을 잠그고 UPreDeploymentFlow를 처음 한 번 생성한다. Flow에 PlayerCharacter를 넘겨 Initialize한 뒤 EnterFlow한다. Initialize는 맵의 PreviewStage를 찾고, EnterFlow는 플레이어 외형을 미리보기로 보내고 PlayerController와 GameInstance를 찾아 현재 출격 상태를 받는다. 이어 BP_PreDeployHub 클래스를 LoadClass하고 위젯을 생성한다. 클래스 경로는 EditAnywhere 필드만 사용하는 방식이 아니라 함수에서 특정 경로로 다시 대입한다. 근거는 `Object/Map/PreDeploymentHellpod.cpp:19`, `Game/PreDeployment/PreDeploymentFlow.cpp:15`, `:22`, `:38`이다.

Flow는 게임 입력을 제거하고 출격 UI MappingContext를 추가하며, UPreDeployHubWidget을 화면에 붙이고 GameAndUI 입력 모드와 마우스 커서를 켠다. Hub는 발사 요청을 `_OnLaunchEvent`로 알리고 Flow의 HandleLaunch가 이 이벤트를 받는다. HandleLaunch는 게임 입력과 GameOnly 모드를 복원하고 커서를 숨긴 뒤 FirstPersonMap을 OpenLevel한다. 현재 launch 이벤트는 선택 미션·필수 장비를 다시 검증하는 절차 없이 맵 이동을 수행한다. 이 부분을 고칠 때 화면 버튼 기능과 실제 출격 허용 조건을 별도 책임으로 이해하는 것이 좋다.

Hub는 장비/스트라타젬 페이지를 바꾸는 WidgetSwitcher와, 장비 소개/주무기 패널/보조무기 패널을 바꾸는 또 다른 WidgetSwitcher를 가진다. 주·보조 슬롯을 누르면 해당 패널을 `_curPanel`로 지정한다. 방향 입력은 그 패널의 MoveLeft/Right/Up/Down, 섹션 입력은 JumpToPrev/NextSection으로 전달한다. ESC는 총 패널에서 장비 소개로 돌아가거나 열린 스트라타젬 패널을 닫는다. 현재 ESC는 Flow 전체를 종료하여 로비 조작으로 복귀하는 취소 기능까지 구현하지는 않는다. 근거는 `UI/PreDeployment/PreDeployHubWidget.cpp:10`, `:42`, `:60`, `:111`, `:124`이다.

UPreDeployPanelBase는 선택 ID를 직접 결정하는 공통 장비 데이터가 아니라 목록의 포커스·상세 표시·키보드 탐색을 재사용하는 기반이다. `_sections`와 현재 section/row/col, 선택 entry, 마지막 장착 entry를 보관한다. FocusEntry는 이전 선택의 테두리를 해제하고 새 entry를 강조하며 상세 패널과 섹션 제목을 바꾸고 ScrollIntoView한다. 좌우는 행 안에서 순환하고 위아래는 이전·다음 행 또는 인접 카테고리로 이동한다. CategorySection은 ID 배열마다 entry 위젯을 만들고 UniformGrid에 넣으며 section/row/col 인덱스를 설정한다. entry가 생성될 때 패널은 `_onPickedEvent`를 연결한다. 버튼 클릭이 entry → panel → state 또는 슬롯 갱신으로 전달되는 구조다. 근거는 `PreDeployPanelBase.cpp:19`, `:55`, `:73`, `:144`, `PreDeployCategorySection.cpp:9`이다.

WeaponPanel은 UCSaveGame의 소유 항목 중 Gun만 고르고, FGunData의 slotType이 패널 타입과 같은 항목을 category별로 묶는다. 현재 출격 상태의 총 ID와 맞는 entry를 초기 선택으로 찾고, 장착 요청에서는 이전 장착 표시를 지운 뒤 state::SetGunID로 선택 ID를 기록한다. `_selectChangedEvent(int32)`를 Broadcast하면 Hub가 연결한 주·보조 슬롯의 InitializeEntry가 아이콘을 바꾼다. 총 슬롯은 기본 UPreDeployEntryBase의 미리보기 조회를 쓰고, WeaponDetail은 이름·설명·관통 특성·탄창·피해·반동·분당 발사수를 읽어 표시한다. 수치 바는 정해진 tierSize로 나누어 10% 단위로 채운다. 근거는 `PreDeployWeaponPanel.cpp:9`, `:84`, `:98`, `PreDeployWeaponDetail.cpp:10`이다.

StratagemPanel도 소유 항목만 목록으로 만들지만 분류는 스트라타젬 클래스의 CDO에서 GetStgType을 읽는다. CDO는 기본 이름·아이콘·종류·투입 지연을 조회하기 위한 객체이며 이 UI에서 실제 스트라타젬 액터를 소환하는 것은 아니다. 네 슬롯 중 하나를 고르면 목록 패널이 열리고, 선택한 스트라타젬 ID가 이미 네 슬롯에 있으면 중복 장착을 거부한다. 기존 슬롯 ID의 entry를 해제하고 새 ID를 state에 기록하며 슬롯 아이콘과 상세를 바꾼다. 다음 빈 슬롯으로 넘어가고 빈 슬롯이 없으면 패널을 닫는다. 슬롯 펼침 애니메이션과 페이지 버튼 표시도 `_panelOpenedEvent(bool)`를 통해 Hub와 맞춘다. 근거는 `PreDeployStratagemPanel.cpp:13`, `:78`, `:139`, `:190`, `:231`이다.

PreDeploymentState의 기본 총 ID는 1과 101, 스트라타젬 ID는 -1 네 개다. SetGunID는 100 미만을 주무기, 200 미만을 보조무기로 취급하는 숫자 범위 규칙이다. 이 규칙은 WeaponPanel에서 쓰는 FGunData.slotType과 별개이므로 새 ID 체계를 넣을 때 둘을 함께 맞춰야 한다. 선택한 총은 새 캐릭터의 InvenComponent::ApplyLoadOut에서 클래스 조회에 사용되고, 스트라타젬은 StratagemComponent의 초기 구성으로 연결된다. 선택 상태가 세션 중 맵 이동을 넘기는 연결점이며 실제 총·지원 장비의 전투 로직은 각각의 장을 참조한다.

## 3.8 전투 GameMode, 목표와 탈출의 순서

AMainGameMode의 BeginPlay는 GamePhase를 InMission으로 바꾸고 출격 상태의 현재 미션을 FMissionProgress에 넣고 미션 제한 시간을 `_remainingTime`으로 복사한다. 설정된 클래스가 있으면 EnemyReinforceManager와 HelldiverReinforceManager를 생성하고, 맵에 이미 배치된 EnemyPatrolManager와 EnemyGarrisonManager를 찾는다. StartPlay는 DropPlaneBeacon을 찾아 보관하고 1초 반복 타이머를 등록한다. 이 타이머는 실제 DeltaSeconds를 합산하는 방식이 아니라 매 호출마다 remainingTime에서 1을 빼고 `_timerUpdatedEvent(float)`를 Broadcast한다. 근거는 `MainGameMode.cpp:22`, `:66`, `:161`이다.

맵 목표는 GameMode::OnObjectiveCleared(objectiveID)를 호출한다. 미션 ID와 같으면 `_objectiveCompletedEvent(FName)`와 `_missionCompletedEvent()`를 알리고 주요 목표 성공을 기록한 다음 탈출 비컨을 활성화한다. 선택 목표 ID이면 objective 이벤트를 알리고 완료 ID 집합에 추가한다. MissionWidget은 두 이벤트에 연결되어 해당 슬롯을 비활성 표시하고, 주요 목표 완료 후 Extraction Available 행을 추가한다. 완료 선택 목표는 TSet이라 같은 ID의 보상 수는 집합 크기로 계산되지만, 이벤트와 주요 목표 처리 전체에 일회성 종료 가드가 있는 것은 아니다. 근거는 `MainGameMode.cpp:75`, `UI/MissionWidget.cpp:32`, `:54`, `:85`이다.

탈출 비컨은 TerminalOperable의 파생 클래스다. 명령 입력이 끝나면 GameMode::CallEscapePlane을 호출하고, GameMode는 클래스·스폰 위치·Yaw 90도로 EscapePlane을 생성한 뒤 `_helldiverExtractEvent`를 OnHelldiverExtracted에 연결한다. 비행기는 BeginPlay에서 트리거 충돌을 끄고 착륙 몽타주 Landing 섹션을 재생한다. PlaneAnimInstance의 OpenHatch Notify가 트리거를 켠다. 그때 이미 임무 시간이 끝난 상태면 20초 뒤 이륙 타이머를 등록한다. 시간이 먼저 끝났다면 GameMode의 UpdateTimer가 기존 비행기에 타이머를 시작하거나 비행기를 호출한다. 근거는 `Object/Map/DropPlaneBeacon.cpp:18`, `MainGameMode.cpp:107`, `Object/Map/EscapePlane.cpp:28`, `:43`, `Object/Map/PlaneAnimInstance.cpp:18`이다.

탈출 트리거에는 HellDiver가 들어와야 하고, `_extractedHellDivers` 배열로 같은 캐릭터의 중복 탑승을 거른다. 탑승하면 추출 이벤트로 탈출 인원이 증가하고 현재 구현은 즉시 TakeOff 몽타주를 재생한다. 첫 탑승 후 일정 시간 대기하는 StartTimerToTakeOff 호출은 해당 위치에서 주석 처리되어 있다. 이륙 완료를 나타내는 Exit Notify가 GameMode::EndBattle을 호출한다. 따라서 현재 임무 종료는 비행기 자체의 Tick이 끝을 감지하는 것이 아니라 **몽타주 섹션과 AnimNotify가 게임 진행을 이어 주는 구조**다. 애니메이션 Notify가 빠지거나 PlaneAnimInstance가 연결되지 않으면 결과 단계로 이어지지 않을 수 있다. 별도의 실패 종료는 증원 예산 소진 경로에서도 EndBattle을 호출한다. 근거는 `EscapePlane.cpp:62`, `:79`, `PlaneAnimInstance.cpp:27`, `Character/HellDiver/HelldiverReinforceManager.cpp:39`이다.

## 3.9 임무 보상, 경험치와 결과 화면

EndBattle은 임무 타이머를 중단하고 남은 시간/제한 시간을 0~1로 Clamp한 뒤 MissionResult에 작전·현재 임무·완료 선택 목표·탈출 인원·성공 여부를 복사한다. 작전에서 이미 클리어한 수에 이번 성공분을 더하여 메달 인덱스를 결정한다. CalculateMissionReward는 아래 네 항목을 보상 행으로 만들고 합계를 계산한다. 근거는 `MainGameMode.cpp:121`, `:177`, `Data/MissionResult.h:23`이다.

| 보상 항목 | 경험치 | RequisitionSlips | 추가 규칙 |
|---|---|---|---|
| 주요 목표 | 성공이면 100 | 성공이면 500 | Operation.rewardMedals[완료 미션 수 - 1]을 총 메달에 추가한다. |
| 선택 목표 | 완료 ID 수 × 50 | 완료 ID 수 × 200 | 주요 목표 성공 여부와 별도 계산한다. |
| 탈출 인원 | 인원 × 20 | 인원 × 50 | 한 명 이상 탈출하면 현재 PlayerCharacter 0의 인벤토리 샘플을 총보상에 추가한다. |
| 남은 시간 | 시간 비율 × 100 | 시간 비율 × 400 | 결과 필드는 int32라 소수 결과는 정수 대입 과정에서 잘린다. |

예를 들어 주요 목표 성공, 선택 목표 1개, 탈출 1명, 제한 시간의 절반이 남았다면 경험치는 100+50+20+50=220, RequisitionSlips는 500+200+50+200=950이다. 메달은 이 숫자에서 산출하지 않고 작전 에셋의 해당 완료 순서 값이다. 샘플도 보상 행의 경험치/돈과 별도로 전체 통화에 들어간다. 현재 멀티플레이어 전원의 샘플을 합산하는 코드가 아니라 플레이어 인덱스 0의 캐릭터에서 가져온다는 점을 기억해야 한다. 실패해도 선택 목표·탈출·시간 보상은 각 공식대로 계산되므로 성공만 보상한다는 해석은 현재 코드와 다르다.

GameInstance::ApplyMissionResult는 총통화를 세이브에 더하고 AddExperience를 호출한다. 경험치는 현재 레벨/잔여 경험치에 더한 뒤 다음 레벨 기준을 넘는 동안 반복해서 기준 경험치를 빼고 레벨을 증가시킨다. 기준식은 `500 + (현재 레벨 - 1) × 100`이다. 따라서 레벨 1의 다음 레벨은 500, 레벨 2는 600이다. 세이브의 초기 `_playerLevel`에는 명시적인 1 기본값이 없어 생성 초기값을 포함해 실제 레벨 정책을 정리할 필요가 있다. PlayerStatusWidget은 생성 시 현재 레벨·잔여 경험치·다음 기준을 한 번 읽고 텍스트와 진행바에 넣는다. 실시간 경험치 변경 델리게이트를 별도로 구독하는 구현은 없다. 근거는 `CGameInstance.cpp:380`, `:534`, `:559`, `CSaveGame.h:42`, `UI/PlayerStatusWidget.cpp:10`이다.

결과 UI를 열기 전에 세이브 정산이 먼저 적용된다. 결과 화면의 보상 애니메이션이 돈을 조금씩 지급하는 것은 아니다. MissionResultWidget은 복사한 MissionResult로 제목을 만들고 항목별 MissionResultSlotWidget을 초기화하며, 타이머로 각 보상을 보여 주고 숨긴다. 3초 대기 후 0.5초 간격 표시, 표시 후 2초 대기와 0.5초 간격 숨김, 총보상 표시 후 3초 대기, 작전 진행 표시 후 7초, 최종 지급 요약 후 5초 흐름이다. 슬롯은 목표 아이콘의 완료 여부·탈출 인원 아이콘·남은 시간 백분율과 XP/돈을 보여 준다. `_rewardFlowFinishedEvent`가 Execute되면 GameMode::OpenLobby가 Lobby를 연다. 근거는 `MainGameMode.cpp:145`, `:154`, `UI/MissionResultWidget.cpp:24`, `:98`, `:129`, `:233`이다.

현재 작전의 마지막 미션 성공 또는 실패에서는 ApplyMissionResult가 출격 상태를 먼저 ResetOperation한다. 결과 UI의 작전 표시 일부는 MissionResult만 읽는 대신 현재 GameInstance의 GetMissionStates를 다시 읽는다. 그래서 결과 스냅샷과 현재 실행 상태의 수명이 어긋나는 지점이 있다. 정산 방식을 바꾸면 보상 계산, 저장 호출, 결과 데이터, 표시 완료 후 맵 이동 순서를 함께 살펴야 한다. EndBattle을 두 번 호출하면 기존 보상 배열과 합계에 다시 누적하며 또 저장하므로, 종료/정산이 한 번만 실행되어야 한다는 계약도 이 연결 전체에 적용된다.

## 3.10 단말 명령 입력이 목표 완료로 연결되는 과정

ATerminalConsole은 AInteractable 계열의 월드 단말로, WidgetComponent·상호작용 위치 앵커·카메라와 `_curTask`를 갖는다. BeginPlay에서 Task의 위젯 클래스를 WidgetComponent에 넣고 위젯을 초기화한 뒤 `_taskCompletedEvent`에 자신을 연결한다. Interact는 다른 플레이어가 사용 중이면 거절하고, 같은 플레이어가 다시 상호작용하면 입력 모드를 종료한다. 새 상호작용이면 PlayerCharacter의 BeginTerminalInputMode로 현재 단말을 설정하고, 플레이어를 앵커 지면 위치로 맞추고 카메라를 바꾸며 Task::StartTask를 호출한다. 근거는 `Object/Map/TerminalConsole.cpp:34`, `:49`, `:102`, `Character/PlayerCharacter.cpp:2161`이다.

UTerminalTaskBase는 UDataAsset이다. 기본 구현은 단말 위젯을 저장하고 숨기며, StartTask는 위젯 표시, EndTask는 완료 이벤트 Broadcast만 한다. 실제 입력 예시는 UCommandInputTask로, `_command`의 FKey 배열과 플레이어 입력 버퍼를 비교한다. W/A/S/D 입력이 들어올 때마다 버퍼에 추가하고 명령 전체와 같으면 완료, 명령의 올바른 접두사면 다음 화살표 진행, 중간에 틀리면 버퍼와 화면을 초기화한다. UCommandWidget은 키마다 화살표 UImage를 만들고 W는 -90도, S는 90도, A는 180도 회전하여 순서를 보여 준다. 입력 수에 따라 투명도나 노란색으로 진행을 표시하고 완료 시 WidgetSwitcher를 바꾼다. 근거는 `Game/Mission/TerminalTaskBase.cpp:6`, `Game/Mission/CommandInputTask.cpp:39`, `UI/CommandWidget.cpp:8`이다.

단말 OnTaskCompleted는 `_missionCompletedEvent`를 알리고 상호작용을 끄고 입력·카메라를 복구한다. ATerminalOperable은 자신 아래 TerminalChild의 이벤트를 받아 OnCommandCompleted를 호출하는 공통 기반이다. 기본 OnCommandCompleted는 비어 있고, BroadcastTower는 GameMode에 objectiveID 완료를 알리며 DropPlaneBeacon은 탈출기를 요청한다. BroadcastTower는 큰 피해 3000 이상을 받으면 Chaos 파괴 필드를 생성하고 같은 목표 완료 함수를 호출하는 대체 해결 경로도 있다. 명령 입력 UI는 목표 완료의 한 입력 경로이며 임무 승패 판정 자체는 GameMode에 있다는 구분이 중요하다. 근거는 `TerminalConsole.cpp:179`, `TerminalOperable.cpp:22`, `BroadcastTower.cpp:36`, `:70`이다.

Task DataAsset에는 정의뿐 아니라 `_terminalWidget`, 입력 버퍼, 완료 델리게이트까지 들어 있다. 같은 Task 에셋을 여러 단말이 참조하면 실행 상태를 공유하는 현재 특성이 생긴다. 이 점은 향후 정의용 DataAsset과 단말별 Task 실행 객체를 분리할 때 고려해야 한다. `Game/TerminalTaskBase.h`는 현재 내용이 없는 파일이고 실제 구현은 `Game/Mission/TerminalTaskBase.*`에 있다. 빈 파일을 다른 단말 시스템이 구현되어 있다는 근거로 읽으면 안 된다.

## 3.11 상점, 소유 장비와 영속 저장

AShopActor::Interact는 플레이어의 Controller로 ShopWidget을 만들고 게임 MappingContext를 상점용으로 바꾸며 GameAndUI 모드와 마우스 커서를 켠다. 화면은 UUIManager에 등록된 이름을 사용하지 않고 상점 액터의 위젯 클래스로 직접 생성한다. ShopWidget::NativeConstruct는 테이블을 읽어 화면을 초기화하고 구매 버튼과 ESC 입력을 연결한다. InitializeWidget은 판매 행을 순회하여 ShopSlotWidget을 만들고 Gun/Stratagem 패널에 나누어 넣으며, 현재 통화와 첫 슬롯의 상세를 표시한다. 근거는 `Object/Map/ShopActor.cpp:9`, `UI/ShopWidgetBase.cpp:17`, `:30`이다.

슬롯을 고르면 Gun 또는 Stratagem 상세 패널을 열고 가격을 표시한다. 소유 중이면 OWNED와 비활성 버튼, 조건 미달이면 LOCKED, 살 수 있으면 PURCHASE, 돈이 부족하면 LOW FUNDS이다. 슬롯 자체도 소유·잠금·구매 가능 여부에 따라 아이콘과 투명도를 달리한다. FPlayerCurrency::CanAfford는 RequisitionSlips와 Medals뿐 아니라 가격에 있는 각 샘플 종류의 수까지 비교한다. 다만 현재 상세 가격 텍스트는 Gun/Stratagem 양쪽 모두 RequisitionSlips만 표시한다. 가격 모델 전체와 화면 표현은 같지 않다. 근거는 `UI/ShopWidgetBase.cpp:72`, `:127`, `UI/ShopSlotWidgetBase.cpp:26`, `Data/PlayerCurrency.h:78`이다.

구매 버튼은 GameInstance::TryPurchaseShopItem을 호출한다. 실제 흐름은 행 찾기 → 이미 소유했는지 검사 → 해금 조건 검사 → 지불 가능 여부 검사 → 통화 차감 → 소유 항목 추가 → 저장이다. 성공하면 상점의 모든 슬롯과 통화 표시를 다시 초기화한다. 소유 정보는 상품 이름·가격을 저장하는 것이 아니라 EShopType와 itemID의 쌍이고, 그 쌍으로 중복 구매를 판단한다. 출격 목록도 이 소유 배열을 읽으므로 상점 구매와 출격 가능 장비가 연결된다. ESC는 상점 MappingContext를 제거하고 게임 입력·커서를 복원한 뒤 위젯을 제거한다. 근거는 `CGameInstance.cpp:493`, `CSaveGame.cpp:33`, `UI/ShopWidgetBase.cpp:173`, `:202`이다.

해금 조건 값은 판매 데이터에 있지만 IsShopItemUnlockConditionMet는 현재 항상 true를 반환한다. 레벨 제한이나 임무 완료 조건이 이미 구현되어 있다고 기억하면 안 된다. 구매 함수의 행 조회 실패도 명시적인 실패 결과 대신 기본 FShopItemData로 이어지는 현재 구현이다. 상점을 확장할 때는 표시에 앞서 상품 ID 유효성·해금 정책·가격 표시·차감 정책을 함께 맞추는 것이 필요하다.

UCSaveGame이 실제 저장하는 필드는 레벨, 잔여 경험치, FPlayerCurrency, 현재 작전 ID, 현재 작전의 완료 미션 ID 집합, 구매 항목 배열이다. LoadGame은 고정 슬롯 이름 SaveSlot과 사용자 인덱스 0을 사용한다. 파일이 없으면 새 객체에 RequisitionSlips 50,000을 더하고 Gun 1과 Gun 101을 소유 항목으로 추가한 뒤 저장한다. 파일이 있으면 로드 후 곧바로 SaveGame을 호출한다. SaveGameToSlot의 성공 반환값을 처리하는 UI나 오류 복구 분기는 없다. 저장 파일이 존재하지만 로드/Cast에 실패하면 LoadGame과 SaveGame이 서로 다시 호출되는 것도 현재 경로다. 근거는 `CSaveGame.h:41`, `CGameInstance.cpp:263`, `:284`이다.

## 3.12 UI 종류별 역할을 찾는 지도

아래 목록은 화면 클래스가 어디에 쓰이는지 기억하기 위한 지도다. 같은 이름의 Blueprint는 배치와 스타일을 공급하고, C++은 데이터 입력·이벤트·표시 변경을 담당한다. BindWidget 속성은 BP 위젯 트리에서 이름이 맞는 버튼·텍스트·패널·이미지를 연결하는 계약이다.

| UI 또는 보조 클래스 | 역할과 데이터 입력 |
|---|---|
| UIManager | GunSetting 이름을 클래스로 연결하고 팝업 중복 검색·생성·닫기·ZOrder 스택을 관리한다. SceneUI API도 있으나 현재 캐릭터 호출은 주석이다. |
| GunWidget | 현재 탄약 비율, HP 비율, 예비 탄약·수류탄·주사기 수, 총 아이콘을 표시하는 전투 HUD다. 이름과 달리 총 정보만 표시하지 않는다. |
| GunSettingWidget / GunSettingSlotWidget | 총 모드와 설정용 DataAsset을 조합하여 발사·스코프·조명 옵션 목록과 현재 옵션 강조를 표시한다. 옵션 변경 자체는 캐릭터/총이 수행한다. |
| CrosshairWidget | 명중 결과에 맞는 HitMarker를 표시하고 타이머로 숨긴다. 피해 판정의 생산자가 아니라 결과를 받는 표시 객체다. |
| DummyHpBar | 전달받은 HP 비율로 ProgressBar를 채우는 간단한 테스트용 체력 표시다. |
| StaminaBarWidget | 스태미나 변화 이벤트의 비율과 회복 여부로 막대와 색/점멸을 바꾼다. 지속적인 자체 Tick으로 수치를 계산하지 않는다. |
| PlayerStatusWidget | 세이브의 레벨·경험치와 다음 레벨 기준을 생성 시 읽어 표시한다. |
| SampleWidget | FSampleBundle을 받아 Common/Rare/Super 텍스트를 먼저 0으로 만든 뒤 해당 수를 넣는다. |
| InteractionWidget | 기본 마크와 조작 키 마크를 WidgetSwitcher로 바꾸고 설명을 표시한다. 아이템 종류별 switch 내용은 현재 비어 있다. |
| InventoryWheelWidget | 인벤토리의 버릴 수 있는 샘플·배낭·지원 총을 네 방향 선택 항목으로 만든다. 마우스 delta 누적으로 커서를 제한 반경에 두고 각도/90도로 선택 인덱스를 만든다. 가운데는 취소다. |
| CommandWidget | W/A/S/D 명령 배열의 화살표, 진행 상태, 완료 상태를 표현하는 공통 기반이다. |
| StratagemWidget / StratagemSlotWidget | 지원 목록과 각 명령 입력·운용·쿨다운·횟수 표시를 맡는다. 쿨다운 시작/종료 관련 타이머와 슬롯 상태가 있다. 실제 투입은 StratagemComponent/액터 측이다. |
| StratagemEtaWidget | 남은 ETA 초를 0 이상으로 표시하고 화면 좌표·중앙 정렬·표시 여부를 설정한다. 월드 투영 좌표와 남은 시간은 지원 액터 쪽에서 공급한다. |
| MissionWidget / MissionSlotWidget | 현재 미션과 선택 목표의 아이콘·제목·ID로 목록을 만들고 목표 완료 이벤트에 따라 표시를 바꾼다. |
| MissionTimerWidget | GameMode의 timerUpdated 이벤트를 받아 남은 초를 분:초로 표시한다. |
| MissionResultWidget / MissionResultSlotWidget | 정산 스냅샷으로 성공/실패·항목별 보상·작전 진행·최종 지급 요약을 순차 표시한다. 표시 완료 이벤트로 로비 복귀를 알린다. |
| PlanetGlobeWidget | 글로브 액터의 현재 사이트/아이콘을 받아 작전 제목·설명·메달과 미션 제목·설명·아이콘을 표시한다. 글로브 회전/선택 판정은 액터 측이다. |
| PreDeployHubWidget | 장비/스트라타젬 페이지, 주·보조 슬롯, 현재 탐색 패널, 출격 요청을 묶는다. |
| PreDeployPanelBase / CategorySection | 카테고리별 목록 생성, 그리드 좌표, 선택/포커스와 키보드 탐색을 제공한다. |
| PreDeployEntryBase / StratagemEntry | 항목 ID·선택 테두리·장착 표시·클릭 이벤트를 보관한다. 기본 entry는 총 미리보기, 파생 entry는 스트라타젬 CDO 아이콘을 사용한다. |
| PreDeployWeaponPanel / StratagemPanel | 소유한 장비 목록을 분류하고 실제 선택을 PreDeploymentState에 쓴다. |
| PreDeployDetailBase / WeaponDetail / StratagemDetail | 기본 상세 함수는 비어 있고 파생 클래스가 각 데이터의 이름·설명·수치·미리보기를 채운다. |
| ShopWidgetBase / ShopSlotWidgetBase | 판매 목록·상세·잔액·구매 버튼과 소유/잠금/지불 가능 표시를 맡는다. 구매 검증·차감·저장은 GameInstance에 요청한다. |
| MiniMapWidget / CompassWidget | 지도 커서·핑 위치를 2D로 변환하고 카메라 yaw와 목표 방향을 표시한다. |
| SceneCapturer / MinimapIconActor | 지도의 월드 영상과 커서/핑 정보를 공급하며 지도 확대에 맞춰 월드 아이콘의 크기를 조절한다. |
| PreviewStage / ImpactMarker | 전자는 캐릭터 모델의 UI용 영상, 후자는 WidgetComponent가 달린 월드 표식 위치를 공급한다. ImpactMarker의 Tick은 비어 있다. |

UIManager의 GetOrShowPopUp은 등록된 클래스와 같은 위젯이 스택에 있으면 그것을 반환하고, 없으면 생성하여 현재 ZOrder로 화면에 넣는다. 이름으로 닫기, 마지막 팝업 닫기, 전체 닫기 방식이 있다. 등록된 기본 이름은 GunSetting 하나다. 상점·글로브·출격 허브·결과 화면과 PlayerCharacter의 HUD는 직접 CreateWidget 경로이므로 UIManager의 CloseAll을 호출해도 이 프로젝트의 모든 화면을 닫는다고 생각하면 안 된다. 또한 SceneUI는 첫 생성/교체 조건과 null 처리가 현재 잘못되어 잠재 결함이 있고, 저장된 위젯을 맵 이동 때 정리하는 공통 훅은 구현되어 있지 않다. 근거는 `UI/UIManager.cpp:5`, `:33`, `:60`, `:92`, `:103`이다.

## 3.13 지도, 핑, 나침반과 캐릭터 미리보기

ASceneCapturer는 SceneCaptureComponent2D와 RenderTarget으로 월드의 위에서 내려다본 영상을 만든다. BeginPlay에서 플레이어 인덱스 0을 따라갈 대상으로 삼고 Orthographic 투영과 현재 확대 단계를 설정한다. Tick에서는 따라갈 대상의 XY를 지도 경계와 카메라 반폭 사이에 Clamp하여 캡처 액터 위치를 정하고, 목표 OrthoWidth까지 보간한다. 필터는 태그로 찾은 액터를 HiddenActors 또는 ShowOnlyActors 목록에 넣는 방식이다. UI가 적 전체의 좌표를 직접 Canvas에 그리는 방식과, 실제 월드/아이콘을 별도 카메라로 촬영하는 방식이 함께 연결되어 있다. 근거는 `UI/SceneCapturer.cpp:11`, `:23`, `:41`, `:232`이다.

지도를 드래그할 때 커서 액터를 만들고 따라갈 대상을 플레이어에서 커서로 바꾼다. 마우스 delta로 커서를 움직이고 맵 범위 안에 제한한다. 캡처 기준 커서 위치, 플레이어 기준 상대 위치, 현재 OrthoWidth 절반을 `_cursorUpdateEvent`로 보낸다. MiniMapWidget은 상대 XY를 UV 0~1로 바꿔 텍스트 위치를 놓고, 거리 cm를 m로 나누고 방위 이름과 각도를 표시한다. 확대/축소 단계는 현재 인덱스 0~2를 전제로 한다. ResetMap은 커서를 제거하고 다시 플레이어를 따라간다. 근거는 `SceneCapturer.cpp:96`, `:108`, `:127`, `:146`, `UI/MiniMapWidget.cpp:15`이다.

PingOnMap은 현재 커서 위치에 핑 액터를 생성한다. 기존 핑 근처 100cm 이내에서 다시 요청하면 제거하고, 다른 위치면 이동시킨다. Tick의 UpdatePing은 미니맵 상대 좌표·월드 위치·표시 여부를 이벤트로 공급하며 플레이어가 핑 50cm 이내에 접근하면 제거한다. PlayerCharacter::FindSceneCapturer가 다음 프레임에 캡처 액터를 찾아 커서 이벤트와 핑 상대 이벤트를 미니맵으로, 월드 핑 위치를 나침반으로 연결한다. 나침반은 매 프레임 Controller의 ViewPoint yaw를 읽어 0~360도로 표시하고, CompassTiles 머티리얼 파라미터와 N/E/S/W 텍스트 위치를 바꾼다. 핑이 시야 밖이면 나침반 끝에 흐리게 표시한다. 근거는 `SceneCapturer.cpp:159`, `:203`, `Character/PlayerCharacter.cpp:2256`, `UI/CompassWidget.cpp:38`이다.

APreviewStage는 출격 화면에서 캐릭터 외형을 렌더링하는 별도 액터다. 입력 플레이어의 SkeletalMeshAsset을 PreviewMesh에 복사하고 AnimationSingleNode로 바꾸어 설정된 AnimSequence를 반복 재생한다. SceneCapture는 ShowOnlyActorComponents로 자신만 촬영하며 RenderTarget을 UI가 사용한다. 현재 플레이어 AnimInstance의 행동 상태를 그대로 복제하거나 선택한 총의 모든 부착물을 자동으로 재구성하는 구현은 아니다. 캐릭터 메시와 지정된 미리보기 애니메이션을 보여 주는 기능으로 이해하면 된다. 근거는 `UI/PreviewStage.cpp:13`, `:44`이다.

## 3.14 한 번의 플레이와 재시작 시퀀스

| 순서 | 호출과 데이터 이동 | 남는 정보 |
|---|---|---|
| 1. 앱 시작 | GameInstance::Init → LoadGame → 새 PreDeploymentState → 캐시 준비 | 세이브의 레벨·통화·소유·작전 기록은 복원된다. 현재 미션·로드아웃은 기본값이다. |
| 2. 로비 진입 | LobbyGameMode → GamePhase Lobby → ApplySaveGameData → 헬포드 목록 수집 | 현재 작전과 작전 내 Cleared 상태를 메모리에 만든다. |
| 3. 작전·미션 선택 | 글로브 사이트/링 → SelectMission → SetOperationAndMission → missionSelected 이벤트 | 작전 ID는 저장 파일, 현재 미션은 출격 상태에 들어간다. |
| 4. 장비 준비 | 헬포드 → Flow → Hub → 장비/지원 패널 → State ID 갱신 | 선택한 총·지원은 세션 메모리에서 유지된다. 상점 구매로 소유가 바뀌면 별도 저장된다. |
| 5. 출격 | Hub launch 이벤트 → Flow::HandleLaunch → FirstPersonMap OpenLevel | 이전 맵 액터 대신 새 맵 액터가 생기고 GameInstance의 선택은 유지된다. |
| 6. 전투 초기화 | MainGameMode가 미션·시간 복사, 캐릭터가 로드아웃 읽기 | 한 전투의 목표 집합·성공·탈출 인원을 GameMode가 보관한다. |
| 7. 목표·탈출 | 단말/목표 액터 → OnObjectiveCleared → 비컨 → 비행기 Notify → EndBattle | 결과 구조체가 해당 전투의 정산값을 보관한다. |
| 8. 정산 | CalculateMissionReward → GameInstance::ApplyMissionResult → 경험치/통화/완료 ID 저장 | 보상은 화면 연출 전에 이미 저장된다. 작전 완료/실패면 진행 기록을 비운다. |
| 9. 결과·복귀 | ResultWidget 타이머 → rewardFlowFinished → OpenLobby | 새 로비가 저장된 진행을 다시 적용한다. 선택 미션은 다시 골라야 한다. |
| 10. 앱 재시작 | 새 GameInstance가 SaveSlot 로드 | 저장 필드만 복원된다. 마지막 선택 총·지원·선택 미션·맵 전투의 남은 시간/HP/적 상태는 복원하지 않는다. |

즉 이 세이브는 진행 중 전장을 정확히 이어 하는 체크포인트 세이브가 아니라, 통화와 장비 소유 및 현재 작전의 완료 기록을 보존하는 방식이다. 임무 시작 후 앱을 종료했다고 전투 시간·몽타주·적 위치·부위 피해가 같은 상태로 돌아오는 코드는 없다. 재시작 후 선택 로드아웃을 기억시키려면 단순 화면 초기값뿐 아니라 SaveGame 필드, 로드 시 검증, 출격 상태 적용 순서를 추가해야 한다.

## 3.15 기능을 수정할 때 같이 확인해야 하는 연결

새 미션을 만들 때는 MissionDataAsset의 ID·시간·선택 목표, OperationDataAsset의 미션 배열과 메달 배열, GameInstance 등록 배열, 글로브 OperationSite Blueprint, 전투 목표 액터의 objectiveID를 함께 맞춘다. 현재 미션 데이터에는 출격할 맵 참조가 없으므로 맵별 임무를 만들려면 Flow의 하드코딩과 새 맵의 GameMode 설정까지 확장해야 한다. 이름·아이콘만 바꾸는 표시 변경과 목표 판정·저장 ID 변경을 구분해야 기존 세이브가 가리키는 데이터가 사라지지 않는다.

새 상점 아이템은 판매 테이블에만 추가해서 끝나지 않는다. Gun 또는 Stratagem 조회 테이블의 같은 ID, 실제 클래스, 미리보기/아이콘, 소유 타입, 총 슬롯 분류와 숫자 ID 규칙을 맞춰야 구매 뒤 출격 목록에 들어간다. 가격에서 메달/샘플을 사용한다면 현재 가격 텍스트가 그 통화를 표시하는지까지 확인한다. 해금 조건은 필드가 존재하지만 현재 true 반환이므로 새 조건에 맞는 정책 구현이 필요하다.

새 UI를 붙일 때는 누가 생성하는지와 누가 닫는지, 어떤 LocalPlayer의 입력을 바꾸는지부터 결정한다. BP에는 BindWidget 이름과 위젯 클래스·InputAction·MappingContext·애니메이션·RenderTarget을 연결해야 한다. C++의 AddUObject 델리게이트와 버튼 AddDynamic, Timeline BindUFunction, 결과의 BindUObject는 서로 다른 이벤트 연결이다. 클래스가 다르더라도 같은 화면을 다시 열면 중복 바인딩·오래된 목록·이전 맵 위젯 참조가 남지 않는지 살핀다. 캐릭터 풀 재사용 시 HUD 생성/표시 책임도 같이 점검해야 한다.

정산 변경은 결과 계산만 고치는 작업이 아니다. GameMode의 Progress에서 Result로 복사하는 경계, GameInstance의 저장 시점, 작전 Reset 시점, 결과 UI가 읽는 스냅샷, 로비의 복원 시점을 함께 확인한다. 단말 변경도 Task 정의·입력 버퍼·화면·완료 delegate·목표 ID의 연결을 봐야 한다. 이 장에서 설명한 현재 경로를 따라가면 화면이 잘못 표시될 때 어디서 값이 만들어졌고 어디에서 수명이 끝났는지를 다시 찾을 수 있다.

