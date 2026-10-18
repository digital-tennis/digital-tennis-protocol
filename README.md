# Digital Tennis · 컨트롤러 인터페이스 규격

펌웨어와 PC가 같은 패킷을 읽도록 GATT·20바이트 이벤트·보안 상태의 계약과 C/C# 코덱을 제공한다.

전체 제품은 NU40-DK에 **별도 외장 IMU**를 부착하는 라켓 컨트롤러와 Unity PC 테니스 게임으로 구성한다. 최초 계획은 **12일·3명**이다.

## 주요 기능

- v1 호환과 v2의8종 명시적 라켓 의도, CRC/범위/순번 검증
- 센서 없는 LINK_PROBE/REPLY와 암호화·본딩 상태 읽기
- Unity/Zephyr 비의존 라이브러리와 공통 오류 벡터

## 빠른 시작

Python3, C99 컴파일러, .NET10 SDK. C# 소비 라이브러리는 netstandard2.1.

```sh
python3 tools/check.py
python3 tools/check-security.py
```

## 책임과 현재 범위

패킷의 형식/의미만 소유한다. IMU 판정·BLE 연결·테니스 규칙은 소유하지 않는다. 코덱 검사 성공은 실제 본딩/IMU 인식의 증거가 아니다.

## 구조와 문서

`Runtime/`: C# 코덱 · `c/`: C99 코덱 · `spec/`: 규격 · `tests/`, `tools/`: 교차 언어 검사

[관리된 컨트롤러 ICD](spec/controller-v2.md) (v1 호환 포함), [공통 벡터](test-vectors.json)

## 도움말과 기여

재현 가능한 문제는 [이슈](https://github.com/digital-tennis/digital-tennis-protocol/issues)로 남긴다. 관리 주체는 [Digital Tennis](https://github.com/digital-tennis)이며 [기여 안내](CONTRIBUTING.md)를 따른다. 검증 커밋을 고정하고 실제 장치 성공·모의 검사·교차 빌드를 구분한다.

## 라이선스

프로젝트 자체 코드·문서는 **[Apache-2.0](LICENSE)**이다. [NOTICE](NOTICE)와 [외부 코드·자산 고지](THIRD_PARTY_NOTICES.md)를 함께 확인한다. 외부 자산에 Apache-2.0을 덮어쓰지 않는다.
