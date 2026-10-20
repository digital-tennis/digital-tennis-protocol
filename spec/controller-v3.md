# DT-ICD-23-30-00 · 물리 컨트롤러 이벤트 v3

R01 / 2026-10-20 / NU40-DK+별도 외장 IMU. NU54는 센서 없는 임시 시험 보드. 자체 문서 관리 형식이며 ATA iSpec2200 공식 적합성을 주장하지 않는다. 기존 [v1/v2 계약](controller-v2.md)은 그대로 유지한다.

## 책임

펌웨어는 보정·축 변환·잡음 제거·물리 패턴/버튼 edge를 소유한다. SDK는 본딩·현재 프로필·보정 ACK·순번·중복·PC 수신시각과 중립 이벤트를 소유한다. 게임은 경기 단계·서버/플레이어·공의 높이/바운드에 따라 액션을 결정한다. 경기 상태를 보드로 보내지 않는다. 게임의 서브/스매시/발리/Skill은 v3 motion kind가 아니다.

## 20바이트 wire

배치·CRC8·little endian·순번/기기시각은 v2와 동일하다. byte1=3이며 v3 전용 type을 사용한다. v3에 legacy SWING(1)/BUTTON(2)를 허용하지 않으며 v1/v2에 아래 type을 허용하지 않는다.

| byte | 필드 | 의미 |
| --- | --- | --- |
| 0..3 | 0x44, version=3, type, flags | flags bit0보정/bit1센서준비/bit2좌손 보정. 나머지0 |
| 4..5 | sequence | 전송마다 증가, half-range wrapping 비교 |
| 6..9 | deviceTimeMs | 이벤트가 생성된 MCU 시각. 게임 타이밍에는 PC 수신시각 사용 |
| 10..11 | eventId | 새 이벤트마다 고유, 입력0 금지. 재전송은 같은ID |
| 12 | kind | 아래 물리 패턴 또는 실제 버튼 번호 |
| 13 | power | Motion의 정규화 세기0..100. 버튼은0 |
| 14..17 | 상대 yaw/pitch | Motion만 ±18000/±9000centidegree. 전역 heading/위치가 아님 |
| 18..19 | battery, CRC8 | 0..100또는255미측정, byte0..18의CRC |

Motion type4는 이미 검출된 회전 구간의 완료 이벤트다. 현재 시작/피크/끝 streaming을 제공하지 않는다. 서버·공·선수 역할은 포함하지 않는다. 측정 세기는 실제 타구 속도나 근력을 보장하지 않는다.

| type/kind | 중립 의미 |
| --- | --- |
| 4/1 | HorizontalPositive:장착/좌손 보정 후 +Z 횡회전 |
| 4/2 | HorizontalNegative:같은 기준 −Z 횡회전 |
| 4/3 | OverheadDown:위에서 내려오는 회전 후보 |
| 4/4 | ShortRotation:작은 범위의 짧은 회전 |
| 4/5 | ObliqueDown:대각선 아래 방향 회전 |
| 4/6 | ObliqueUp:대각선 위 방향 회전 |
| 5/1..5 | ButtonDown:해당 번호의 debounce된 눌림 |
| 6/1..5 | ButtonUp:해당 번호의 debounce된 해제 |

버튼은 부팅 때 이미 눌려 있으면 놓을 때까지 이벤트를 만들지 않는다. 길게 누름은 Down1회/Up1회이며 반복 타격은 게임이 임의로 만들지 않는다. 동시 버튼도 각각의 edge·eventId로 전달한다. 연결이 끊기면 PC의 눌림 상태와 보류 입력은 초기화하고, 이전 edge를 새 연결에 재생하지 않는다.

## 조회·보정과 호환

보안 특성과 CCC/Write/Notify의 암호화·본딩 조건은 변경하지 않는다. 새 SDK는 기존 기기 지원을 위해 v2 LINK_PROBE(130,kind0)를 보낼 수 있다. 새 펌웨어는 v3 LINK_REPLY(131,동일 eventId)의 kind2=PhysicalMotion, kind3=PhysicalButtons로 응답한다. v3에서는 kind0/1을 프로필로 쓰지 않는다. v2 응답은 기존kind0/1을 유지한다. 구형 SDK는 v3을 거부하며 테니스 액션으로 잘못 해석하지 않는다.

CALIBRATE/ACK/STATUS type과 필드 제약은 유지하며 v3에서도 사용한다. 명령은 v2/v3을 수용하고 응답은 기기의 프로필 버전으로 만든다. STATUS flags만으로 PC 보정 성공을 만들지 않는다. 새 Motion은 현재 조회로 확인된PhysicalMotion 프로필과 해당 연결의 성공 보정ACK가 필요하다. PhysicalButtons는 현재 본딩/조회 확인 뒤 센서 없이 edge를 전달한다.

한 번의 유효 조회 응답만 프로필을 변경한다. 프로필 변경·단절·보정 실패·센서 flags 상실은 보류 입력과 준비 상태를 무효화한다. 입력 큐/재전송/만료 정책은 소비 제품이 검증하며 MCU 시각을 PC 수신시각으로 위장하지 않는다.

## 게임 해석 예

같은 OverheadDown20바이트는 자기 서브 대기에서Serve, 랠리의 높은 공에서Smash 후보가 된다. 낮은 공/거리 밖/상대 서브 차례에서는 기존 타격 gate가 거부한다. ShortRotation은 바운드 전Volley, 바운드 뒤에는 일반 스트로크 후보로 해석할 수 있다. 버튼4를Skill에 연결하는 정책도 게임의 바인딩이며 SDK/보드의 이름이 아니다. 온라인은 중립 입력을 호스트 게임까지 유지한다.

공유 벡터의v3 정상/오류와 C/C# 결과가 동일해야 한다. 기존 v1/v2 정상 입력의 ID와 의미는 변경하지 않는다. 새 규격으로 version3이 지원되므로 과거 unsupported-version 표본의 version값은4로 옮겨 검사한다.
