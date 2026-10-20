# Digital Tennis Controller Protocol

**펌웨어와 PC를 연결하는 작은 공통 언어.** BLE GATT 계약, 20바이트 라켓 이벤트와 C99·C# 코덱을 제공합니다.

[규격 읽기](spec/controller-v2.md) · [빠른 시작](#빠른-시작) · [사용 예](#사용-예) · [기여하기](CONTRIBUTING.md)

## 이 프로젝트를 사용하는 곳

[컨트롤러 펌웨어](https://github.com/digital-tennis/digital-tennis-firmware)는 이 규격으로 이벤트를 만들고, [BLE SDK](https://github.com/digital-tennis/digital-tennis-ble-unity)는 같은 규격으로 검증·해석합니다. 센서 드라이버나 Unity 게임 없이도 양쪽 구현이 동일한 바이트를 읽는지 확인할 수 있습니다.

- **고정 길이 이벤트:** 순번, 이벤트 ID, 동작, 세기, 상대각, 장치 시각과 CRC를 20바이트에 담습니다.
- **버전 호환:** v1의 포핸드·백핸드·서브를 유지하고 v2에 스매시·발리·슬라이스·로브·스킬 의도를 추가합니다.
- **명시적 버튼 모드:** v2 BUTTON의 서브·포핸드·백핸드·스킬과 본딩 후 입력 프로필 협상을 지원합니다. IMU 보정 상태와 구분합니다.
- **제어와 진단:** 보정 명령/응답과 센서 없는 통신 왕복 확인을 정의합니다.
- **보안 상태:** 암호화·현재 상대의 본딩 상태를 별도의 보호된 GATT 특성으로 읽습니다.
- **교차 언어 검사:** 공통 정상·오류 벡터와 생성 사례를 C/C#에서 동일하게 검사합니다.

## 빠른 시작

검사에는 **Python 3, C99 컴파일러, .NET 10 SDK**가 필요합니다. C# 라이브러리 출력은 `netstandard2.1`이며 UnityEngine에 의존하지 않습니다.

```sh
git clone https://github.com/digital-tennis/digital-tennis-protocol.git
cd digital-tennis-protocol
python3 tools/check.py
python3 tools/check-security.py
```

첫 검사는 패킷을, 두 번째는 보호된 링크 상태의 형식과 범위를 확인합니다. 실제 Bluetooth 기기에 접속하지 않습니다.

라이브러리만 빌드하려면 다음 중 필요한 언어를 선택합니다.

```sh
# C99 정적 라이브러리
cmake -S . -B .work/build
cmake --build .work/build

# C# netstandard2.1
dotnet build DigitalTennis.Protocol.csproj -c Release
```

## 사용 예

### C# — 포핸드 이벤트 왕복

```csharp
using System;
using DigitalTennis.Protocol;

var sent = new ControllerPacket(
    version: 2, type: PacketType.Swing, flags: 3,
    sequence: 1, deviceTimeMs: 1000, eventId: 1,
    kind: (byte)SwingKind.Forehand, power: 65,
    yawCdeg: 1200, pitchCdeg: 500, battery: 255);

byte[] wire = PacketCodec.Encode(sent);
if (PacketCodec.Decode(wire, out var received) != DecodeError.None)
    throw new InvalidOperationException("Invalid controller packet");

Console.WriteLine($"{wire.Length} bytes / {(SwingKind)received.Kind}");
// 20 bytes / Forehand
```

이 예의 `flags: 3`은 준비·보정 완료 상태를 가정한 **소프트웨어 예제**입니다. 실기 펌웨어는 실제 상태에 따라 값을 설정합니다. 각도 단위는 0.01도, `battery: 255`는 미측정입니다.

### C99 — 수신 패킷 검사

```c
#include "dt_protocol.h"

dt_error read_racket_event(const uint8_t *bytes, size_t length,
                          dt_packet *event)
{
    return dt_decode(bytes, length, event);
}
```

`c/include`를 include 경로에 추가하고 `dt_protocol`에 링크합니다. 오류일 때 출력 구조체를 사용하지 않습니다. CRC는 손상 검출이며, 연결 상대를 인증하거나 암호화하지 않습니다.

## 규격을 찾는 방법

| 필요한 정보 | 문서·소스 |
| --- | --- |
| GATT UUID, 필드·단위, 버전, 제어 절차 | [컨트롤러 인터페이스 규격](spec/controller-v2.md) |
| 보호된 링크 상태의 기계 판독 정의 | [secure-link-v1.json](spec/secure-link-v1.json) |
| 정상·오류 패킷 예제 | [test-vectors.json](test-vectors.json) |
| C99 공개 API | [dt_protocol.h](c/include/dt_protocol.h) |
| C# 공개 API | [ControllerPacket.cs](Runtime/ControllerPacket.cs) |

전체 GATT UUID와 오프셋은 위 규격을 기준으로 합니다. README에 별도의 패킷 표를 복제하지 않습니다.

## 통합할 때 알아둘 점

코덱은 **패킷의 유효성**을 검사합니다. BLE 연결·OS 본딩, 재접속 세대, 입력 만료·중복 제거는 SDK가 맡고, IMU 보정·스윙 분류는 펌웨어가 맡습니다. 타격 성공과 테니스 점수는 [게임](https://github.com/digital-tennis/digital-tennis-game)이 판정합니다.

Unity 패키지 이름은 `com.digital-tennis.protocol`입니다. Git UPM 또는 C# 프로젝트 참조로 사용하고, 제품에 통합할 때는 검토한 커밋을 고정합니다. 새 동작을 추가하려면 규격·두 코덱·벡터·소비 제품을 함께 갱신하세요. [시스템 책임 구분](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/project/architecture.md)

## 도움말·기여

[Digital Tennis](https://github.com/digital-tennis)가 관리합니다. [Issues](https://github.com/digital-tennis/digital-tennis-protocol/issues)에 기대한 해석, 오류 코드와 재현 가능한 패킷을 남겨 주세요. 실제 기기 식별자·본딩 키는 제외합니다. 변경 절차는 [CONTRIBUTING.md](CONTRIBUTING.md)를 따릅니다.

## 라이선스

직접 작성한 코드·문서는 [Apache-2.0](LICENSE)입니다. [NOTICE](NOTICE)와 [외부 코드 고지](THIRD_PARTY_NOTICES.md)를 함께 확인하세요.

## 물리 컨트롤러 이벤트 v3

[새 계약](spec/controller-v3.md)은6종 물리 회전 패턴과 번호별 버튼Down/Up을20바이트로 전달합니다. 서브·스매시·발리의 의미는 게임이 결정합니다. v1/v2의 기존 ID는 유지하고, v3 전용 type과 프로필로 혼동을 차단합니다.
