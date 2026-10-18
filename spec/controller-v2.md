# DT-ICD-23-10-00 · 라켓 컨트롤러 인터페이스

## 00 문서 관리

| 항목 | 값 |
| --- | --- |
| 문서 ID / 개정 | DT-ICD-23-10-00 / R05 |
| 발행 / 상태 | 2026-10-19 / PR 검토본 |
| 책임 / 소비 제품 | protocol / firmware, ble-unity, game |
| 적용성 | 실제 제품 NU40-DK+별도 외장 IMU. NU-54는 임시 센서 없는 BLE/본딩 실기 타깃 |
| wire 호환 | 이벤트 v1/v2=20바이트. 보안 읽기 값은 별도 schema1=4바이트 |
| 기준 | 코드·벡터·소비 Git SHA를 함께 고정. 스쿼시 병합 후 참조 SHA 재고정 |

이 문서는 ATA iSpec2200의 문서 관리·계층 번호·적용성·개정/검증 추적 방식에서 영감을 받은 **프로젝트 자체 ICD**다. 아래 DT 번호는 항공기 ATA 공식 챕터가 아니다. 유료 iSpec 원문/SGML DTD를 복제하지 않았고 iSpec2200 준수 인증이나 SGML 교환 호환을 주장하지 않는다. [ATA 공식 범위](https://ataebiz.org/standards/).

### 유효 절 목록과 개정 이력

| 유효 절 | 제목 | 개정 |
| --- | --- | --- |
| 10 | 책임·데이터 흐름 | R04 |
| 20 | GATT·본딩 | R04 |
| 30 | 이벤트 wire 배치 | R03 보존 |
| 40 | 명령·상태 절차 | R04 |
| 50 | 순번·오류·수명 | R03 보존 |
| 60 | 검증·장애 격리 | R04 |

R01=v1 기본3샷. R02=v2의4~8 명시적 의도. R03=센서 없는 LINK_PROBE/REPLY. R04=암호화/영속 본딩 읽기와 제품 경계 명확화. R05=v2 버튼 게임 의도4~7과 LINK_REPLY 입력 프로필0/1. 본딩 읽기는 기존20바이트에 예약 비트를 임의로 끼워 넣지 않는 별도 특성이다.

## 10 설명과 작동 원리

`외장 IMU → firmware(샘플/보정/구간/액션) → protocol 코덱 → BLE → ble-unity(검증/중복제거/PC시각) → game 입력 adapter → 타격/경기 규칙`.

- DT-REQ-001: firmware는 센서 드라이버·장착 축·보정·샘플 주기·기본 액션 판정을 소유해야 한다.
- DT-REQ-002: protocol은 GATT/필드/단위/버전/정상·오류 벡터만 소유해야 한다. BLE OS 호출이나 모션 분류를 하지 않는다.
- DT-REQ-003: SDK는 OS 연결/페어링, 현재 연결의 보안 확인, wire 검증/중복 제거와 입력 의도를 소유해야 한다. 센서 샘플을 다시 분류하지 않는다.
- DT-REQ-004: game은 플레이어/온라인 권한·샷의 유효 거리/시점·물리·점수·스킬을 결정한다. 보드에는 Near/Far나 점수가 없다.

기본 경로는 **펌웨어 판정 이벤트**다. 향후 원시 샘플을 PC에서 연구할 수 있지만 현재 wire에 원시6축 데이터를 임의로 삽입하지 않는다. 원시 스트림 프로필을 추가하려면 새 명세·벡터·시계/누락 정책이 먼저 필요하다. 제품은 HID 마우스나 OS 커서 입력을 사칭하지 않는 사용자 정의 GATT 라켓이다.

20바이트는 ATT_MTU23의 notification value 한도(ATT_MTU−3)에 맞춘 호환 설계다. BLE 전체의 절대 최대 전송량은 아니다. MTU가 커져도 이 이벤트는20바이트로 유지한다. [Bluetooth SIG ATT](https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Core-54/out/en/host/attribute-protocol--att-.html).

예를 들어20바이트 샘플을100Hz로 보내면 값만2,000B/s다. 초당2스윙을3회 전송하고 초당 상태1회를 더하면140B/s다. 이는 헤더/재전송/보안 등 링크 비용을 제외한 비교이며 무선 처리량·입력 지연 보장이 아니다. 기본 방식을 고른 이유는 고정 샘플 주기와 보정 상태를 한 곳에서 소유하고 손실/OS일정에 인식기가 의존하지 않게 하기 위해서다.

## 20 연결·GATT·보안

| ID | UUID | 속성/권한 | 값 |
| --- | --- | --- | --- |
| service | 6d7c1000-9d40-4cb7-a381-5e908be0a001 | primary | 라켓 서비스 |
| TX | 6d7c1001-9d40-4cb7-a381-5e908be0a001 | Notify, CCC 암호화 읽기/쓰기 | 이벤트20바이트 |
| RX | 6d7c1002-9d40-4cb7-a381-5e908be0a001 | Write with response, 암호화·본딩 요구 | 명령20바이트 |
| secure link | 6d7c1003-9d40-4cb7-a381-5e908be0a001 | Read, 암호화 요구 | 아래4바이트 |

### 20.10 보안 읽기 값

| byte | 의미 | 유효 값 |
| --- | --- | --- |
| 0 | magic | 0x44 |
| 1 | schema | 1 |
| 2 | Zephyr 보안 수준 | 1..4; 정상 사용은2이상 |
| 3 | flags | bit0=현재 peer의 bond 저장됨, bit1=LE Secure Connections; 나머지0 |

level1에서 flags는0이어야 한다. C `dt_security_decode/encode`, C# `LinkSecurity.Decode`가 같은 조건을 검사한다. 예: `44 01 02 03`은 암호화+bond+SC, `44 01 01 00`은 준비 전, `44 01 02 00`은 암호화됐으나 미본딩이다. 마지막 두 값으로 연결 완료를 표시하지 않는다. 값에는 키·PIN·주소가 없다.

DT-REQ-005: OS에서 단순히 GATT 연결됐다는 이유만으로 본딩 성공을 표시하면 안 된다. firmware는 peer identity에 대한 bond와 실제 보안 수준을 보고한다. Mac은 보호된 CCC/읽기와 기기 보고를 확인하며 공개 API에 없는 OS key DB를 추측하지 않는다. Windows는 OS IsPaired도 함께 확인한다. 실기 완료 기록에는 OS 승인, 실제 응답, 연결 해제/재접속과 키 재사용 범위를 구분해야 한다.

DT-REQ-006: native helper는 보안 schema가 없거나 잘못된 기기를 호환 본딩 기기로 수용하지 않는다. 보안 읽기가 준비 전이면 짧게 제한 재시도하고 UI 연결 대기는60초로 제한한다. 사용자가 OS 승인을 취소하면 성공으로 바꾸지 않는다. 연결을 닫는 것과 bond를 삭제하는 것은 다른 동작이다.

### 20.20 정상 절차

선행 조건: 호환 테니스 펌웨어, 켜진 PC Bluetooth, 사용자 OS 승인 가능. 도구: game 무선 설정 또는 동봉 native helper. IMU는 본딩/왕복 확인의 선행 조건이 아니다.

1. service UUID로 검색하고 현재 검색한 본인 기기만 선택한다.
2. 연결→특성 발견→보호된 CCC 구독으로 OS 페어링을 진행한다. firmware는 L2이상·LE Secure Connections 페어링·bondable 설정과 BT settings를 사용한다. 화면 없는 보드의 현재 방식은 사용자 승인된 Just Works로, PIN/숫자 대조에 의한 MITM 인증을 제공하지 않는다. Zephyr 설정은 `CONFIG_BT_SMP_SC_PAIR_ONLY=y`이며 인증된 Level4를 강제하는 `SC_ONLY`와 구분한다.
3. 보안 읽기4바이트를 검증하고 level≥2 및 bond=1일 때 연결 완료로 이동한다.
4. LINK_PROBE를 보내 같은 nonce의 LINK_REPLY를 기다린다. 응답 왕복과 보정은 별도다.
5. 센서가 없으면 ready/calibrated=false로 남아 있어야 한다. game 설정의 동작 테스트/저장은 금지한다.
6. 해제→재접속/재시작으로 키 재사용을 확인하고, 새 보정/동작 테스트 전에는 오래된 입력을 적용하지 않는다.

기존 NU-54 참고 지갑 서비스 `6e753534-...`의 CBOR 조회 성공은 위 service·wire·본딩 검증을 대체하지 않는다. 프로필을 몰래 번역하거나 가짜 테니스 패킷을 만들어서는 안 된다.

## 30 이벤트 패킷 배치

다중 바이트 정수는 little-endian, 각도는 부호 있는2의 보수다.

| offset | 길이 | 필드 | 의미 |
| --- | --- | --- | --- |
| 0 | 1 | magic | 0x44 |
| 1 | 1 | version | 1 또는2 |
| 2 | 1 | type | 아래 표 |
| 3 | 1 | flags | bit0 calibrated, bit1 sensor ready, bit2 left handed; 나머지0 |
| 4 | 2 | sequence | 방향별 uint16 패킷 순번 |
| 6 | 4 | deviceTimeMs | uint32 기기 단조 시각; 명령0 |
| 10 | 2 | eventId | 스윙/버튼 ID 또는 명령 nonce; STATUS0 |
| 12 | 1 | kind | 아래 표 |
| 13 | 1 | power | SWING0..100, 그 외0 |
| 14 | 2 | yawCdeg | SWING ±18000,0.01도; 그 외0 |
| 16 | 2 | pitchCdeg | SWING ±9000,0.01도; 그 외0 |
| 18 | 1 | battery | 0..100 또는 미측정255; 명령255 |
| 19 | 1 | crc8 | 앞19바이트 CRC-8/ATM |

| type | kind/조건 |
| --- | --- |
| 1 SWING | v1=1포핸드/2백핸드/3서브; v2만4스매시/5발리/6슬라이스/7로브/8스킬 |
| 2 BUTTON | 1확인/2시작/3일시정지; v2만4서브/5포핸드/6백핸드/7스킬 |
| 3 STATUS | kind0/eventId0 |
| 128 CALIBRATE | kind/flags/time/power/각도0, battery255 |
| 129 ACK | 0성공/1BUSY/2SENSOR_NOT_READY/3CALIBRATION_FAILED |
| 130 LINK_PROBE | v2만, nonce1..65535, kind/flags/time/power/각도0, battery255 |
| 131 LINK_REPLY | v2만, 요청 nonce, 실제 flags/기기시각, kind=0 IMU/1 버튼 모드, power/각도0 |

CRC poly0x07/init0/refin=false/refout=false/xorout0. ASCII123456789=0xF4. 범위 내라도 v1은 v2 전용 의도를 수용하지 않는다. v3이상은 거부한다.8개 wire 의도 지원은 사람8제스처의 정확도를 뜻하지 않는다.

## 40 명령·센서 상태

보정은 동일 연결/명령 ID로만 대응하고5초 제한을 쓴다. BUSY는 제한을 연장하지 않는 진행 상태다. write 응답만으로 보정을 성공시키면 안 된다. 센서 없는 LINK_REPLY도 보정 성공이나 입력 intent를 만들지 않는다. LINK_REPLY는 현재 연결의 미완료 nonce와2초 제한이 맞아야 하며 반복/늦은 응답은 무시한다.

센서 장착 변환/왼손 처리는 firmware에서1회 적용한다. yaw/pitch는 스윙 구간의 상대 회전이며 절대 공간 위치가 아니다. SDK는 yaw/4500과pitch/4500을[-1,1]로 제한해 전달한다. 게임이 자기 코트/시점 변환과 실제 샷 조건을 적용한다.

## 45 버튼 입력 모드 · R05

NU-54 임시 시험 펌웨어는 센서 없이 실제 GPIO 버튼으로 플레이할 수 있다. NU40-DK+별도 외장 IMU의 기본 모드는 바꾸지 않는다. 본딩·암호화를 확인한 같은 연결에서 LINK_PROBE의 현재 nonce에 응답한 LINK_REPLY.kind=1만 버튼 모드를 선택한다. 응답 전·잘못된 nonce·만료·이전 연결·알 수 없는 프로필에서는 버튼 경기 입력을 열지 않는다. Unity 네이티브 backend는 연결 직후 자동으로 이 질의를 수행한다.

| 물리 버튼 | v2 BUTTON.kind | 정규화 의도 |
| --- | --- | --- |
| NU54 SW1 | 4 | Serve |
| NU54 SW2 | 5 | Forehand |
| NU54 SW3 | 6 | Backhand |
| NU54 SW4 | 7 | Skill |

BUTTON의 power/각도는 계속0이며 센서 flags는 실제 상태를 보존한다. SDK는 버튼임을 나타내는 별도 의도를 전달하고 게임 adapter가 현재 CSV 기반 기본 타격 세기와 중립 조준을 사용한다. 보정 완료를 위조하거나 BUTTON1~3을 경기 액션으로 바꾸지 않는다. 버튼 모드에서는 SWING을 거부하고, IMU 모드에서는 BUTTON4~7을 거부한다. IMU 모드의 보정 명령/완료 조건은 유지한다.

Unity는 버튼 모드임을 표시하고 IMU 보정 UI를 숨긴다. 저장은 같은 연결의 실제 서브·타격·스킬 입력 테스트를 모두 통과한 뒤에만 가능하다. 설정 테스트는 경기 입력과 분리한다. 저장 후 경기의 거리·타이밍·스킬 충전 조건은 그대로 적용된다. 중복 재전송은 한 번만 전달하며 단절·취소·화면 변경 때 대기 버튼을 폐기한다. 재연결은 모드 협상과 입력 테스트를 다시 진행한다.

## 50 순번·재전송·오류

uint16 순번은 `0<((new-old)&65535)<32768`일 때만 새롭다. eventId가 같은 반복 스윙은 한 번만 전달한다. firmware는 Swing/Button/제어3회·Status1회,20ms 재시도, 입력200ms/제어1000ms 만료와32개 유한 큐를 사용한다. 원래 eventId·기기시각을 보존하고 전송별 sequence는 증가한다. 단절/구독 변경 시 이전 연결 큐를 비운다.

MCU 시각은 약49.7일에 wrap한다. PC시각과 빼서 무선 지연으로 부르면 안 된다. SDK는 PC 단조 수신 시각으로 입력 수명을 관리하고 이전 연결 epoch·오래된 입력·중복·CRC/길이/버전/범위를 거부한다. 잘못된 바이트를0값 정상 스윙으로 대신하지 않는다.

## 60 검사·장애 격리·기록

| 증상/요구 | 검사/판정 | 근거 |
| --- | --- | --- |
| DT-REQ-001..004 책임 | 제품 경계·원시/이벤트 흐름 검토 | docs 아키텍처 결정 |
| 이벤트 양언어 일치 | 공통49벡터·생성/손상 총2065사례 | tools/check.py |
| DT-REQ-005/006 보안 문법 | 모든 level/flag·magic/version C/C#66046사례 | tools/check-security.py |
| 단순 connected/다른 peer 증거 | SDK는 Pairing에 남고 입력을 열지 않음 | SDK checks |
| 보호된 read 실패/거부 | OS 승인/호환 펌웨어/기존 키 상태를 표시, 자동 초기화 금지 | native helper·실기 |
| 통신 성공/센서 없음 | IMU 프로필은 probe성공만으로 보정/저장 불가; 버튼 프로필은 세 입력 테스트 후 저장 | Unity 패킷/실물 설정 검수 |
| 단절/재접속 | fresh epoch·본딩 재확인·이전 입력0 | SDK/Unity·실기 기록 |

각 실기 기록에는 코드/펌웨어 SHA, 보드, OS, 실제/모의 라디오, 최초 본딩/재접속/재시작 범위와 남은 IMU/Windows 항목을 남긴다. 결과 없이 PASS나 OS 본딩 완료를 표시하지 않는다. ATA 전체 규격을 포함하지 않은 이 문서의 형식 검증과 항공 규격 인증은 관계없다.
