# 장비 투사체와 스트라타젬 구현

## 장비 시스템을 이해하는 기준

이 영역은 팀원이 작성한 무기·인벤토리·스트라타젬 기능과 캐릭터 시스템이 만나는 곳이다. 출격 준비의 선택은 ID이고, 전장에 들어오면 ID가 실제 Actor와 Component로 바뀐다. 플레이어 입력은 캐릭터가 받고, 발사 주기·탄약·피해·효과는 총의 컴포넌트가 처리한다. 캐릭터 상태와 애니메이션은 무기 사용을 허용하거나 중단하고 손·등 소켓에 장비를 연결한다.

다음 설명에서 수치는 C++의 기본값 또는 코드에 직접 들어 있는 값이다. 무기별 실제 수치, 몽타주, 소켓, 클래스는 DataTable과 Blueprint 설정을 함께 확인해야 한다. 특히 GunData와 ProjectileData는 서로 다른 정의이므로 어느 쪽이 실제 피해를 결정하는지 구분해야 한다.

| 클래스 | 보관하는 핵심 상태 | 담당 역할 |
|---|---|---|
| UHellDiverInvenComponent | 총 슬롯 3개 현재 총 배낭 샘플 | 소유 장비 생성 장착 교체 드롭 |
| AGunBase | FGunData 소유 캐릭터 컴포넌트 반동 | 총의 사용과 각 컴포넌트 연결 |
| UGunFireComponent | 발사 모드 타이머 점사 수 볼트 준비 | 언제 발사할지 결정 |
| UGunAmmoComponent | 탄창 약실 예비 탄약 재장전 단계 | 발사 자원과 재장전 진행 |
| UGunDamageComponent 계열 | 피해 정의 투사체 또는 샷건 데이터 | 발사 방향과 실제 피해 전달 방식 |
| AGunBulletBase | 속도 이동 거리 관통 수 피격 컴포넌트 | 탄의 이동 충돌 관통 도탄 |
| UStratagemComponent | 슬롯 개인 및 그룹 쿨다운 사용 횟수 | 지원 장비를 사용할 수 있는지 판단 |
| AStratagem | 투척 장치 부착 위치 도착 시간 | 지원 호출 위치와 전개 연출 |

## 출격 선택에서 실제 무기까지

UHellDiverInvenComponent 생성자는 _gunSlot의 길이를 3으로 만든다. 슬롯 의미는 Primary=0, Secondary=1, Support=2다. BeginPlay는 샘플 묶음을 비우고 소유 AHellDiver를 캐시한 뒤 GameInstance의 PreDeploymentState를 읽는다. ApplyLoadOut이 주무기/보조무기 ID를 GameInstance의 GetGunClassFromTable로 클래스에 연결하고, SpawnGun으로 두 총을 생성한 다음 캐릭터의 InitWeapon을 호출한다. 지원 무기 기본 생성 경로는 주석으로 남아 있다. 근거는 Character/HellDiver/HellDiverInvenComponent.cpp:29,47,67이다.

SpawnGun은 Actor 생성 → SetOwner(캐릭터) → InitializeGun → SetGun 순서다. 총 Actor의 BeginPlay에서는 _gunID로 FGunData를 읽는다. InitializeGun은 Actor Owner를 AHellDiver로 캐시하고 비활성 상태로 둔다. SetGun은 무기 데이터의 슬롯 종류를 보고 같은 슬롯의 기존 총을 드롭한 뒤 새 총을 등/허리의 보관 소켓에 붙인다. 따라서 장비 생성과 손에 드는 행위는 별도의 단계다.

EquipGun은 현재 총 포인터를 슬롯에서 가져와 ActivateGun과 AttachToHand를 호출한다. AHellDiver의 SwitchGun은 상태 검사, 이전 총의 조준·재장전·발사 중단과 보관, 새 총 장착, 무기 상태·애니메이션 Layer 갱신, 전후 알림으로 연결된다. PlayerCharacter의 전후 알림은 HUD에 표시할 총과 탄약 델리게이트도 교체한다. 슬롯을 교체하는 코드를 수정할 때 화면 정보와 AnimLayer까지 같이 확인해야 하는 이유다. 대표 근거는 HellDiver.cpp:596,614와 PlayerCharacter.cpp:1711,1717이다.

장비의 보관 소켓은 cc_mainweapon_socket, cc_subweapon_socket, cc_supportweapon_socket이며 손은 cc_weaponbone_r_socket이다. 보관할 때 장비 메시의 물리·중력을 끄고 충돌을 NoCollision으로 만든다. 드롭은 반대로 Attach를 해제하고 물리·중력과 충돌을 켜며 소유 Actor를 비운다. 다만 Actor Owner와 AGunBase의 _owner 캐시가 별개라는 점을 기억해야 한다. 현재 드롭 경로가 캐시까지 언제 정리하는지는 장비 재획득과 함께 점검할 부분이다.

배낭도 같은 원리를 따른다. ABackpack::PickupItem은 캐릭터 인벤토리에 배낭을 등록하고 캐릭터를 Owner로 지정한 뒤 cc_backpack_socket에 붙인다. 인벤토리는 기존 배낭이 있으면 먼저 DropBackpack한다. ReloadBackpack은 BeginPlay에서 예비 탄약을 최대치로 채우고 ConsumeSpare로 줄이는 구체 타입이다. 샘플은 개별 Actor 목록이 아니라 FSampleBundle 값으로 합쳐 보관하며, DropSample은 묶음 Actor를 생성한 후 인벤토리 값을 비운다.

## 총 데이터와 컴포넌트 조립

FGunData는 이름·아이콘·설명뿐 아니라 슬롯/카테고리/잡는 방식, 발사 모드·주기, 탄약 용량·예비량·재장전 종류, 반동·흔들림, 피해 수치·투사체 DataAsset, 컴포넌트 클래스, 부착물 클래스와 총 Actor 클래스를 정의한다. 예를 들어 _fireComponentClass와 _damageComponentClass를 바꾸면 같은 AGunBase 틀에 다른 발사 방식과 피해 전달 방식이 결합된다. 정의 위치는 Data/GunDataTable.h의 FGunData다.

AGunBase::SetGunData는 데이터를 복사하고 SetGunComponent로 Fire/Ammo/Damage/Effect 및 부착물 컴포넌트를 NewObject·RegisterComponent한다. 이후 Fire에 발사 모드, Ammo에 탄약 데이터, Damage에 피해 데이터를 전달하고 Effect와 부착물은 총 메시로 초기화한다. Fire의 _fireEvent는 AGunBase::Fire에 연결된다. 근거는 Gun/GunBase.cpp:82,484다. SetGunData는 현재 단순한 값 갱신이 아니라 컴포넌트 조립까지 수행하므로 런타임 무기 수치 변경용으로 반복 호출하려면 기존 컴포넌트 수명을 먼저 설계해야 한다.

FGunData와 FGunProjectileData의 역할이 겹치는 부분이 있다. Hitscan은 UGunDamageComponent에 복사된 _baseDamage와 거리 감쇠를 쓴다. ProjectileDamageComponent는 _projectileDataAsset의 데이터를 복사하고 실제 탄은 그 복사본으로 초기화한다. 일반 투사체의 피해를 바꾸려면 총 테이블의 피해 숫자만 바꾸는 것으로 충분한지 실제 경로를 확인해야 한다. 센트리는 이와 다르게 에셋 복사 후 센트리 전용 피해로 명시적으로 덮어쓴다.

## 발사 입력이 한 발로 바뀌는 과정

발사 흐름은 캐릭터 입력 → AGunBase::StartFire → UGunFireComponent::StartFire → TryFire → _fireEvent → AGunBase::Fire → Damage/반동/Ammo/Effect 순서다. GunBase::CanFire는 소유 캐릭터, 재장전 상태, 약실 준비를 확인한다. 한 발이 실제로 나가면 FireShot을 호출하고 반동을 적용한 다음 탄약을 소비하고 머즐·탄피 효과를 재생한다. 근거는 GunBase.cpp:131,142,149와 GunFireComponent.cpp:46,74다.

자동과 점사는 _fireInterval 간격의 반복 타이머로 TryFire를 부른다. 점사는 _burstCount를 3으로 시작해 각 시도에서 줄인다. 반자동은 입력 때 TryFire를 즉시 호출하고 StopFire한다. 볼트액션도 즉시 시도하되 _canBoltFire를 끄고 별도 타이머로 볼트를 다시 준비한다. StopFire는 반복 타이머를 지우고 캐릭터 Firing 상태를 해제한다. 반자동에는 별도의 클릭 간 간격 제한이 보이지 않으므로 모드 변경을 할 때 데이터의 _fireInterval과 실제 허용 주기가 같은 의미인지 확인해야 한다.

발사 모드 변경은 _fireModes 배열의 인덱스를 순환한다. 이 배열은 초기화에서 첫 요소를 바로 읽으므로 비어 있지 않아야 한다. _fireInterval도 양수여야 반복 타이머의 의미가 유지된다. 새 총을 추가할 때 데이터 필수값 검증이 필요한 지점이다.

## 탄창 약실과 재장전 상태

UGunAmmoComponent는 탄창 _curAmmo와 약실 _isChamberLoaded를 구분한다. 초기화는 최대 탄수를 설정한 뒤 탄창에서 1을 빼고 약실을 true로 만든다. 따라서 화면에서 보여주는 총 탄수는 _curAmmo + _isChamberLoaded다. 한 발을 쓰면 약실을 비우고 탄창이 남아 있으면 탄창에서 1을 옮겨 다시 약실을 채운다. CanFire가 검사하는 것은 총 탄수보다 약실 여부다. 근거는 Gun/Component/GunAmmoComponent.cpp:20,58,64다.

재장전 종류는 Magazine과 RoundsReload다. Magazine은 Idle → RemoveMag → InsertMag → CloseBolt → Idle 단계로 이어진다. 탄창 제거 완료 시 탄창을 비우고, 삽입 완료 시 예비 탄창 또는 배낭 자원을 소비해 새 탄창을 채운다. 이미 약실이 차 있으면 CloseBolt를 생략하며 약실이 비어 있으면 마지막 섹션에서 탄창 1발을 약실로 옮긴다. 이 설계에서는 약실이 찬 재장전 후 _maxAmmo+1을 표현할 수 있으므로 HUD의 최대치와 ‘탄창 용량’의 의미를 함께 확인해야 한다.

Reload는 캐릭터 상태를 StartReload로 바꾸고 _reloadMontage를 재생한다. PickReloadSection은 단계에 따라 섹션 인덱스 0/1/2를 고른다. 섹션 종료 시 AGunBase::OnReloadSectionEnded가 Ammo에 전달하고 ChangeReloadStage가 다음 단계를 실행한다. 몽타주 종료/중단 콜백은 FinishReload로 상태를 해제한다. RoundsReload는 섹션 끝마다 1발의 예비 자원을 소비하고 다시 Reload해 연속 장전을 구성한다. 대표 근거는 GunAmmoComponent.cpp:93,151,179,199,239다. UCharacterAnimInstance::JumpToSection은 인덱스를 Section0/Section1/Section2 이름으로 변환하므로 몽타주가 이 이름 계약을 따라야 한다. 실제 섹션 구성과 AnimNotify_Reload 배치는 몽타주 에셋에서 확인해야 한다.

CancelReload는 몽타주를 중단하고 캐릭터의 재장전 상태를 해제한다. _reloadStage를 초기화하지 않으므로 중단한 단계에서 재개하려는 구조다. 다만 CloseBolt에서 중단하면 탄창은 이미 _maxAmmo로 채워져 있고 약실은 비어 있어 AGunBase::Reload의 선행 CanReload 검사가 재개를 거부할 수 있다. Ammo 컴포넌트 내부에는 CloseBolt의 CanReload 검사를 생략하는 처리가 있지만 외부 총 진입점에는 이 예외가 없다. 수정할 때 재장전 취소 후 무기 교체, 약실 있는/없는 재장전, 배낭 유무를 함께 시험해야 한다. 현재 CanReload가 허용한 자원과 완료 시 소비하는 자원이 다른 분기도 있어 이 연결을 먼저 정리해야 한다.

## 조준 반동 손 위치와 부착물

StartAiming/StopAiming은 캐릭터의 Aiming 상태, 크로스헤어·ImpactMarker 표시, 모든 부착물의 OnAimChanged를 갱신한다. 조준 중 GunBase Tick은 총구 소켓에서 Visibility trace를 쏘아 맞을 위치를 얻고 부착물과 마커를 갱신한다. 이 trace는 화면 피드백용이며 실제 피해를 처리하는 GameDamage trace/투사체 충돌과 구분된다. 카메라 중앙과 총구 방향을 바꾸면 이 두 경로가 같은 목표를 가리키는지도 확인해야 한다.

ApplyFireRecoil과 RecoverRecoil은 소유 캐릭터의 제어 회전에 반동을 적용·복구한다. GetRecoilMultiplier는 이동 속도가 10 단위 미만인지와 Standing/Crouching/Proning 자세에 따라 배율을 계산한다. 조준 여부를 직접 검사하지는 않는다. 왼손 그립 변환 GetLeftHandleTrans, 머즐 변환, 오른손 기준 머즐 보정 CalculateMuzzleToHand_R은 캐릭터 AnimInstance와 IK의 입력이 된다. 이 부분은 총 메시의 소켓과 캐릭터 스켈레톤을 바꿀 때 함께 확인해야 한다. 근거는 GunBase.cpp:262,325,344,365,522,542다.

GunEffectComponent는 공통 Niagara 시스템을 하드코딩된 콘텐츠 경로에서 읽고 Muzzle/ShellEject 소켓에 붙인 컴포넌트를 재활성화한다. TacticalLight는 SpotLight를 TacticalLight 소켓에 붙이며 On/Off/Auto 모드를 순환한다. Auto는 조준 여부에 따라 켜진다. Laser는 LaserPoint에서 hitPoint까지 Beam Start/Beam End 파라미터와 피격점 효과를 갱신한다. Scope는 모드 인덱스 순환은 구현되어 있지만 OnAimChanged 본문은 비어 있어 실제 화면 배율 변경은 캐릭터/카메라/BP 연결을 확인해야 한다.

## Hitscan과 실제 투사체

HitscanDamageComponent는 발사점에서 10,000cm까지 GameDamage 채널의 line trace를 수행한다. ACharacterBase에 맞으면 거리 cm를 m로 바꾸어 CalculateDamage에 전달하고 표준 ApplyPointDamage를 호출한다. CalculateDamage는 25/50/100m 구간의 감소율을 선형 보간하고 100m 이후는 마지막 구간 기울기를 연장해 0~1로 제한한다. 결과는 baseDamage×(1-falloff)다. 근거는 Damage/HitscanDamageComponent.cpp:11,37와 GunDamageComponent.cpp:54다. 현재 캐릭터는 커스텀 FCDamageEvent 경로를 사용하므로 이 표준 피해 이벤트와의 연결은 별도 검증 대상이다.

ProjectileDamageComponent는 실제 AGunBulletBase를 생성한다. Owner는 총 Actor, Instigator는 소유 HellDiver이며 ProjectileData를 탄에 전달한다. 탄의 _bulletHitEvent는 GunBase::ShowHitMarker에 연결된다. ShotgunProjectileDamageComponent는 펠릿 수만큼 방향을 랜덤 cone으로 분산시킨 후 같은 생성 함수를 반복한다. 이 구조에서 샷건은 ‘하나의 큰 피해’가 아니라 여러 개의 독립된 탄과 부위 충돌이다.

AGunBulletBase는 sphere 충돌과 ProjectileMovement를 사용한다. 초기 속도 데이터는 m/s이고 실제 UE 이동에는 100을 곱한다. 매 Tick은 이동 거리를 누적하고 거리 감쇠를 속도에 적용한다. 속도가 매우 낮아지면 탄을 제거하고 폭발탄이면 폭발한다. OnBulletOverlap은 HitBox 프로필의 컴포넌트만 처리하며 같은 컴포넌트에 두 번 맞지 않도록 _hitComponents에 기록한다. 근거는 Gun/GunBulletBase.cpp의 Tick, OnBulletOverlap이다.

탄은 피격점 법선과 역속도 방향의 내적으로 입사각을 얻는다. 각도 25/60/80도 경계에 따라 FArmorPenetration의 direct/slightAngle/largeAngle/extremeAngle 중 AP를 선택하고 기존 관통 수를 뺀다. AP가 AV보다 2 이상 높으면 OverPenetrating, 1 높으면 FullPenetrate, 같으면 Penetrate, 낮으면 Ricochet이다. 이는 탄의 ‘통과·정지·반사’를 결정하고, StatComponent의 장갑 판정은 실제 HP 감소를 결정한다. 두 판정의 숫자와 의미를 같이 유지해야 한다.

OverPenetrating은 관통 횟수를 늘리고 기본 속도와 현재 속도를 감쇠시켜 계속 이동한다. FullPenetrate/Penetrate는 탄을 제거한다. Ricochet은 법선 반사 방향에 0.6 배 속도를 준다. 실제 피해는 현재 속도/기준 속도 배율을 기본·내구도 피해에 적용한 정수 값으로 전달한다. 기준 속도가 관통 후에도 수정되므로 현재 코드의 속도 비율을 단순한 최초 에너지 손실 공식으로 해석하면 안 된다.

피해 전달은 FCDamageEvent에 HitInfo, ShotDirection, ColComp, 기본·내구도·철거 피해, 관통 레벨, 경직·밀기 힘, 폭발 여부를 채워 OtherActor::TakeDamage를 호출하는 방식이다. 이 이벤트가 자신의 부위 파괴 시스템으로 넘어가는 계약이다. 반면 일반 blocking hit는 표준 FPointDamageEvent를 구성하는 별도 경로다. 캐릭터 HitBox 프로필, Collision 채널, 부위 태그와 이벤트 형식을 동시에 바꾸지 않도록 주의해야 한다.

## 폭발 수류탄과 보급

UExplosionComponent는 소유 Actor 위치 또는 전달받은 위치를 중심으로 GameDamage 채널 sphere overlap을 수행한다. 각 피격 컴포넌트의 위치와 폭발 중심 거리로 ratio=Clamp(1-distance/radius,0,1)을 계산하고 _damage×ratio를 적용한다. 기본/내구도 피해를 같은 actualDmg로 넣고 철거·관통 값과 IsExplosionDamage=true를 FCDamageEvent에 채운다. 같은 Actor의 여러 부위가 각각 overlap에 들어올 수 있어 폭발의 단위는 컴포넌트다. 근거는 Object/Explosive/ExplosionComponent.cpp:42,57,87이다.

현재 구현에서는 HitBox만 남기는 추가 필터가 주석이고 차폐물에 대한 line trace도 없다. canDamageOwner는 ExplosionComponent의 Owner Actor와 비교하는 값이며 총알/수류탄의 Instigator 플레이어까지 자동으로 같은 대상으로 취급하는 규칙은 아니다. 폭발 차폐·자해·부위별 다중 피격 정책을 수정할 때 이 의미를 명확히 해야 한다. 수류탄·폭발탄·폭격탄이 같은 컴포넌트를 사용하므로 공통 변경의 영향 범위가 넓다.

AThrowable은 손에 부착되어 있을 때 충돌과 ProjectileMovement를 끄고, Throw 시 분리·충돌 복구·이동 활성화 후 캐릭터 Power로 속도를 준다. 잠깐 소유 캐릭터와의 이동 충돌을 무시한 후 복구한다. ATimedGrenadeBase는 cooking 시작 시간을 기록하고 남은 fuse 시간으로 폭발 타이머를 만든다. 손에서 시간이 끝나는 경우 UpdateCookingGrenade가 폭발시키고, 던진 이후에는 타이머가 처리한다. _isExploded와 타이머 정리로 중복 폭발을 막는 구조다.

ASupplyBox::PickupItem은 플레이어의 RefillAllItem을 호출한다. RefillAllItem은 총 예비 탄약과 소모품을 복구하는 캐릭터 경로로 연결된다. AReloadBackpack은 예비량 감소가 중심이며 ‘어떤 무기를 재장전할 수 있는 배낭인지’는 HellDiver의 CanReloadUsingBackpack/UseBackpack과 함께 읽는다. 소모품 숫자와 HP 회복 애니메이션을 바꾸는 경우 StimPackComponent와 캐릭터 상태도 영향받는다.

## 스트라타젬 정의와 전장 사용 상태

FStratagemSlot에는 StratagemID, 실제 AStratagem 클래스, Cooldown, CooldownGroup, GroupCooldown, MaxCharges, UI 설명/이미지가 정의되어 있다. LastUsedTime, CurrentCharges와 횟수 델리게이트는 전장 사용 상태다. UStratagemComponent의 BeginPlay는 GameInstance의 출격 선택 ID를 받아 테이블 행을 슬롯 배열로 복사하고 마지막 사용 시간을 초기화하며 제한 횟수가 있으면 최대치로 채운다. 모든 선택 ID가 -1이면 BP 기본 슬롯을 유지하는 분기가 있다. 근거는 StratagemComponent.cpp:23,31이다.

쿨다운 판단 순서는 재무장 대기 → 사용 횟수 소진 → 개인 쿨다운 → 그룹 쿨다운이다. 개인 시간은 slot.LastUsedTime, 그룹 시간은 _groupLastUsedTime 맵에서 읽는다. 표시할 남은 시간은 재무장이 있으면 그 시간, 없으면 개인/그룹 중 큰 값이다. MaxCharges=0은 이 컴포넌트에서 횟수 제한을 사용하지 않는 의미다. 센트리의 maxAmmo=0과 의미가 다르므로 같은 숫자를 다른 시스템에 그대로 해석하지 않는다.

CommitStratagemUse는 실제 사용 확정 시 현재 시간을 기록하고 횟수를 줄여 델리게이트를 발행하며 그룹 시간도 갱신한다. Can/Select/Commit이 분리되어 있어 사용 가능 검사, 선택, 실제 투척이 별개의 시점이다. 관련 근거는 StratagemComponent.cpp:103,156,199이다. 외부에서 Commit을 호출하는 경로가 늘면 검사와 자원 소비가 한 번씩만 일어나는지 확인해야 한다.

## 방향키 조합에서 투척까지

PlayerCharacter는 BeginStratagemInputMode에서 ActionState를 Stratagem으로 바꾸고 W/A/S/D를 _stratagemInputBuffer에 쌓는다. CheckStratagemInputCombo는 사용 가능한 슬롯의 AStratagem 클래스 기본 객체인 CDO에서 _inputSequence를 읽는다. 완전 일치면 슬롯을 선택해 EquipStratagem하고, 아직 일치 가능한 접두사면 입력 진행 UI를 갱신한다. 가능한 접두사가 하나도 없으면 버퍼와 표시를 초기화한다. 근거는 PlayerCharacter.cpp:1972,2090이다. 동일 키는 InterActing 상태에서는 단말에 전달되므로 상태 분기가 입력 라우팅을 결정한다.

EquipStratagem은 총을 내려놓고 Layer/WeaponState를 바꾼 뒤 선택 클래스의 장치를 손 위치에 생성·부착한다. OnThrowReleased는 제어 회전에 pitch 20도를 더한 방향으로 던지고 _heldThrowable을 비운다. 이후 사용 확정과 이전 장비 상태 복구가 연결된다. 현재 이 함수의 CommitStratagemUse는 수류탄 종류를 구분하는 조건 밖에 있으므로, 수류탄과 스트라타젬을 같은 투척 함수로 유지하면서 소비 규칙을 바꿀 때 반드시 함께 검토해야 한다. 근거는 HellDiver.cpp:163,189이다.

투척 궤적 미리보기는 캐릭터의 스플라인/메시 표시와 Power 기반 예측을 사용한다. 장치 Actor의 AThrowable은 실제 물리 이동을 담당하므로 ‘미리보기 궤적’과 ‘실제 던지기’의 속도·중력·출발점 정의가 같아야 한다. 캐릭터 입력, 손 부착, 투척 완료 Notify를 한 묶음으로 따라 읽는 것이 이해하기 쉽다.

## 지면 부착과 지원 호출 도착 표시

AStratagem은 AThrowable의 파생 클래스다. 지면 충돌 시 ImpactNormal과 Up의 내적이 0.5보다 크면 붙을 수 있는 표면으로 판단한다. 부착한 뒤 이동을 멈추고 장치 몽타주를 재생한다. 공격형은 즉시 지원 Controller를 생성하고 그 Controller에 deployDelay를 전달한다. 방어/보급형은 deployDelay 후 상공에서 _objectToSpawn을 생성해 목표 방향으로 낙하시킨다. 근거는 Object/Stratagem/Stratagem.cpp의 OnHit, DeployStratagem이다.

비공격형의 ETA는 deployDelay + dropHeight/dropSpeed다. 공격형 Orbital은 첫 탄 도착 시간과 후속 폭격 지속시간을 따로 표시하고, Eagle은 첫 폭탄의 ETA를 표시한다. 장치 Tick이 남은 시간을 줄이며, 장치 몽타주가 Loop 섹션에 들어간 시점부터 ETA 표시 가능 상태가 된다. 화면에 어떤 ETA를 보여줄지는 PlayerCharacter::UpdateStratagemEtaUI가 여러 장치를 검사해 후보를 선정하고 다른 표시를 숨긴다. 근거는 PlayerCharacter.cpp:1726이다.

ETA는 Controller가 계산한 예상 시간이며 충돌·지형·탄 클래스 설정에 따라 실제 도착과 다를 수 있다. 낙하 속도나 호출 지연, 몽타주 Loop 이름을 바꾸면 ETA 준비와 표시 수명도 함께 바꿔야 한다. 장치가 제거될 때 HideEta가 호출된다.

## 궤도 폭격과 이글 공습

ABombardmentController는 웨이브 수, 웨이브당 탄 수, 웨이브 사이/내부 간격으로 폭격을 스케줄한다. InitializeBombardment는 중심 위치·Owner·랜덤 시드·인덱스를 준비하고 시작 지연 타이머를 건다. SpawnNextWave는 반복 탄 타이머를 만들고 SpawnNextShell이 산포 위치를 뽑아 탄을 생성한다. 웨이브가 끝나면 다음 웨이브 타이머로 넘어가며 전체 종료 후 Controller를 제거한다. 근거는 Offensive/BombardmentController.cpp:23,42,66,105이다.

현재 탄 출발점은 목표 바로 위가 아니라 월드 원점의 X/Y와 _dropHeight다. 중심 위치가 원점에서 멀면 탄 이동 거리와 ETA가 길어지는 구조다. 산포도 반경 r을 균등하게 뽑으므로 원 면적에서 균등한 분포와는 다르다. 목표 위치·산포·ETA를 바꾸려면 SampleTargetLocation, SpawnOneShellAt, GetEstimatedFirstImpactDelay를 함께 읽는다. 두 시간 함수는 실제 스케줄과 동일한 간격 정의를 유지해야 한다.

AEagleStrikeController는 공습 접근 방향, 폭탄 착탄점, 투하 시점, 비행 Actor를 연결한다. 8개 방향을 만들고 높은 장애물 trace에 막히지 않은 첫 방향을 고른다. BuildBombImpactPattern은 비행 방향을 따라 salvo 중심들을 만들고 좌우 폭탄을 배치한다. BuildDropTimingPattern은 비행 진행도 0.3~0.7 구간에 투하 시점을 배분한다. Tick의 UpdateBombing은 while로 이미 지나간 salvo를 모두 처리해 프레임이 낮아도 예정된 투하를 건너뛰지 않게 한다. 근거는 Offensive/EagleStrikeController.cpp:40,139,230,287,328이다.

AEaglePlane은 시작·중앙·이탈 위치를 만들고 정규화된 시간으로 2차 Bezier 위치를 계산한다. 진행도가 1이 되면 비행을 끝내고 제거한다. Controller는 현재 비행 진행도와 폭탄 생성 위치를 읽어 목표로 폭탄을 발사한다. ETA는 시작 지연 + 첫 투하까지의 비행 시간 + 폭탄 이동거리/속도다. 현재 다중 폭탄 패턴은 짝수 개수를 전제로 한 부분이 있으므로 홀수 개수를 데이터로 허용하려면 착탄점 수와 투하 수가 일치하는지 검증해야 한다.

EagleRearm은 일반 투척 경로의 예외다. 입력 조합이 맞으면 ExecuteEagleRearm을 직접 호출한다. ApplyEagleRearm은 재무장 슬롯의 LastUsedTime을 기록하고 동일 그룹의 제한 횟수를 즉시 최대치로 복구한다. 다만 그룹은 재무장 Cooldown 동안 사용 불가로 판정한다. ‘시간이 끝난 뒤 숫자를 채우는 방식’과 다르며 UI에서도 같은 의미를 표현해야 한다. 근거는 StratagemComponent.cpp:270,301과 PlayerCharacter.cpp:1847이다.

## 낙하 포드 보급 포드와 센트리

ADropPod는 실제 낙하 외피다. 캐릭터/센트리/움직이는 물리 오브젝트를 착륙 표면에서 제외하고, 유효한 지면에 닿으면 한 번만 HandleGroundLanding을 실행한다. 낙하 중 HitBox overlap에는 높은 관통의 FCDamageEvent를 주며 _damagedCharacters로 Actor 중복을 막는다. 착륙 후 내부 Actor를 생성하는 흐름이 보급/센트리의 외피와 내부 기능을 분리한다. 플레이어 증원용 HellDiverDropPod는 별도의 캐릭터 배치·조작 흐름이 있으므로 이름이 비슷하다고 하나의 클래스로 취급하지 않는다.

AHellPodBase는 보급 장비가 들어 있는 내부 포드다. BeginPlay에서 열기 몽타주를 Loop에 멈춰두고, 메시 높이를 기준으로 지면에서 올라올 목표 높이를 정한 뒤 소켓마다 아이템을 생성·부착한다. 각 아이템의 _onPreDespawn을 받아 남은 목록에서 제거한다. 모두 소모하면 닫기 몽타주를 재생하고 Close Notify에서 포드를 없앤다. HP가 0이 되면 남은 아이템을 드롭하고 포드를 제거한다. 근거는 Pod/HellPodBase.cpp:23,77,184,202,222다.

ASentryTurret은 포드 이후 전개되는 독립 전투 Actor다. Perception과 Team ID를 사용해 현재 보이는 살아 있는 적을 고르며, 기존 타겟이 계속 유효하면 유지하고 새 선택은 머즐에 가까운 캐릭터를 택한다. Yaw/Pitch 회전 속도를 제한하고 계산된 본 각도를 USentryAnimInstance에 전달한다. 준비 상태는 상승 완료, 정렬 중 아님, 하강 중 아님, 잔탄과 머즐 존재다. 근거는 Defensive/SentryTurret.cpp:52,143,242,271,314이다.

센트리의 실제 발사 gate는 ‘조준각이 허용치 안에 들어왔는가’가 아니라 ‘시야에 공격 가능한 적이 하나라도 있는가’다. bAimed 계산은 현재 발사 조건으로 쓰이지 않는다. Fire는 실제 머즐 방향으로 탄을 생성하고 잔탄을 줄이며, 잔탄 소진은 사격 중단·하강 퇴장으로 이어지고 HP가 0 이하가 되면 즉시 Destroy한다. Niagara 머즐/탄피 효과는 별도의 컴포넌트 풀을 순환한다. 데이터의 _maxAmmo 기본값 0은 현재 발사 준비 검사에서 사격 불가이며 무한 탄약 의미가 아니다. 근거는 SentryTurret.cpp:336,382,437,578,603,756,786이다.

## 이 영역을 바꿀 때 확인할 연결

새 무기는 총 DataTable 행, 총 Blueprint의 _gunID와 스켈레톤 소켓, Fire/Ammo/Damage 컴포넌트 클래스, ProjectileDataAsset, 몽타주와 섹션 종료 호출, AnimLayer/손 IK, HUD 바인딩까지 연결된다. 기존 무기 수치 변경은 실제 피해 경로가 테이블 수치인지 투사체 수치인지부터 확인한다.

새 스트라타젬은 출격 ID와 테이블 행, AStratagem CDO 입력 조합, 지원 타입과 _objectToSpawn, 쿨다운 그룹·횟수, 공격 Controller 또는 낙하 포드/내부 Actor, ETA UI를 함께 정의한다. 그룹명을 바꾸면 재무장 슬롯도 같은 그룹이어야 한다. 새 보급 아이템은 AItemBase의 획득/소멸 이벤트 계약과 인벤토리 반영을 따른다.

읽는 순서는 HellDiverInvenComponent → GunBase → GunFire/GunAmmo → 실제 Damage 파생 클래스 → GunBulletBase → CharacterBase/StatComponent다. 스트라타젬은 PlayerCharacter::CheckStratagemInputCombo → StratagemComponent → HellDiver::EquipStratagem/OnThrowReleased → AStratagem → 지원 Controller/DropPod → 내부 포드/센트리 순서다. 이렇게 입력부터 결과까지 한 번 따라가면 이름이 비슷한 클래스들의 역할이 분명해진다.

