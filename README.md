# Digital Tennis 컨트롤러 통신 규격

보드와 PC가 공유하는 바이너리 규격, C 및 C# 코덱과 테스트 벡터를 담당하는 독립 제품 저장소입니다. 전체 프로젝트는 **외장 IMU를 별도로 부착한 NU40-DK**, Unity PC 게임, 1인 AI와 온라인 2인 대전을 12일·3명으로 개발합니다.

## 현재 상태

C99 및 Unity 비의존 C# 코덱과 교차 언어 검사를 구현했습니다. `python3 tools/check.py`로 공통 벡터·정상/손상 패킷·encode/decode 왕복을 검사합니다. C는 Address/UndefinedBehavior sanitizer를 함께 사용합니다. 실물 BLE 수신·센서 인식 성공을 의미하지 않습니다.

[v1/v2 계약](spec/controller-v2.md)을 확인하세요. v1의 포핸드1·백핸드2·서브3과20바이트 배치는 유지하고, v2에서만 스매시4·발리5·슬라이스6·로브7·스킬8을 허용합니다. v1 수신기는 기존대로 미지원 버전을 거부해야 하며 v2 송신 펌웨어와 v2 지원 SDK를 함께 배포합니다.

## 개발할 기능과 제출 기준

[초보 개발자용 상세 과제](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/products/digital-tennis-protocol.md)에서 상황·입력·출력·예제·예외·완료 기준을 확인합니다.

- [PR-01 하드웨어와 PC가 합의하는 계약 작성](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/products/digital-tennis-protocol.md#pr-01)
- [PR-02 C와 C# 패킷 인코더 및 디코더](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/products/digital-tennis-protocol.md#pr-02)
- [PR-03 공통 벡터와 순번 비교 검증](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/products/digital-tennis-protocol.md#pr-03)
- [PR-04 버전과 소비 제품 의존성 관리](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/products/digital-tennis-protocol.md#pr-04)

## 개발 환경과 예상 구조

초기 담당 역할은 C / A 검토입니다. 실제 IMU·배선·펌웨어 SDK·Unity·Windows·런타임 버전은 D1에 [환경표](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/project/overview.md)와 결정 기록에 남깁니다.

`c/`는 C99 라이브러리, `Runtime/`은 netstandard2.1/Unity UPM 라이브러리, `tests/`와 `tools/check.py`는 독립 검사입니다. .NET10 SDK·C99 컴파일러·Python3를 사용합니다. Unity 소비자는 이 저장소의 검토한 Git SHA를 고정해 패키지로 사용합니다.

## 검증과 협업

- [12일 WBS](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/planning/wbs.md)
- [컨트롤러 계약](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/contracts/controller-protocol.md)
- [온라인 계약](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/contracts/online-protocol.md)
- [수용 테스트](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/testing/acceptance.md)
- [브랜치·PR 작업 방법](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/guides/contributing.md)
- [모든 제품과 의존 관계](https://github.com/digital-tennis/digital-tennis-docs/blob/main/docs/project/repositories.md)

라이선스는 코드와 제3자 자산 조건을 검토한 뒤 팀이 선택합니다.
