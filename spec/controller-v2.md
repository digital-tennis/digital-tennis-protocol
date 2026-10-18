# 컨트롤러 패킷 v1/v2

20바이트 배치·little-endian·CRC-8/ATM(앞19바이트,poly0x07/init0/xor0)을 유지한다. 오프셋0 magic0x44,1 version,2 type,3 flags,4 sequence(u16),6 deviceTimeMs(u32),10 eventId(u16),12 kind,13 power,14 yawCdeg(i16),16 pitchCdeg(i16),18 battery,19 crc.

v1 SWING kind1=포핸드,2=백핸드,3=서브를 보존한다. v2만4=스매시,5=발리,6=슬라이스,7=로브,8=스킬을 추가한다. 기존 수신기가 v2를 v1로 추측해서 해석하면 안 된다. 배포 펌웨어와 SDK의 지원 버전을 함께 고정한다. v3 이상은 거부한다. 추가 값은 **명시적 게임 의도**이고 실물에서8개 동작을 정확히 구분했다는 뜻이 아니다. 버튼·기본 제스처의 샷 선택 정책은 firmware/application 계층에 둔다.

type1 SWING,2 BUTTON(kind1확인/2시작/3일시정지),3 STATUS(kind0/eventId0),128 CALIBRATE(kind0/flags0/time0/battery255),129 ACK(kind0성공/1바쁨/2센서미준비/3보정실패). SWING 외 power/각도는0. flags는 bit0보정완료/bit1센서준비/bit2왼손만 허용한다. power0..100,yaw±18000,pitch±9000,battery0..100또는255를 검사한다. 보정 미완료 SWING은 디코딩 가능하지만 SDK가 게임으로 전달하지 않는다.

sequence 비교는0<((new-old)&65535)<32768이다. 이벤트 반복 전송은 sequence가 달라도 같은 type/eventId를 재실행하지 않는다. 연결마다 새 PC epoch를 만들고 대기열·순번·이벤트 이력을 폐기한다. SDK는 MCU 시간을 PC시각으로 간주하지 않고 실제 수신시각을 PC단조시계로 찍는다. 보정 ACK는 같은 연결/명령ID에만 대응하며5초 제한 시간을 사용한다. GATT write 성공만으로 보정 성공을 표시하지 않는다.

Service `6d7c1000-9d40-4cb7-a381-5e908be0a001`, Notify `6d7c1001-9d40-4cb7-a381-5e908be0a001`, Write-with-response `6d7c1002-9d40-4cb7-a381-5e908be0a001`. UUID/버전/실제 연결 상태를 확인한 뒤 구독한다. 보드 패킷에는 경기의 Near/Far·플레이어 권한이 없다.

`python3 tools/check.py`는 보관된 공통 벡터와 시드107의 생성/손상 패킷을 C·C# 양쪽에 똑같이 전달하고 에러·모든 필드·왕복 바이트를 비교한다. 실행 보고서는 Git 밖 `.work/validation.json`에 기록한다. 실물 배선·라디오·Windows 실기 검증은 별도다. 원래12일·3명·별도 부착 외장IMU 조건을 유지한다.
