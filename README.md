# AOTTG2_DX11

DirectX 11 기반 자체 엔진으로 [AoTTG 2](https://github.com/AoTTG2-Archive/AoTTG-2)를 재구현한 비상업적 포트폴리오 프로젝트입니다.

## 프로젝트 구성

| 폴더 | 내용 |
|---|---|
| `Engine` | 엔진 코어 (DLL) |
| `EngineSDK` | 엔진 공개 헤더 / 라이브러리 |
| `Editor` | 에디터 |
| `Client` | 게임 클라이언트, `Client/Bin/Assets` 에 에셋 |
| `Converter` | 원본 에셋을 엔진 포맷으로 변환하는 도구 |
| `Docs` | 설계 문서 |

## 에셋 출처 및 라이선스

`Client/Bin/Assets` 에 포함된 모델, 텍스처, 애니메이션, 사운드 등 일부 에셋은
**AoTTG 2** 프로젝트의 것입니다.

- 원작: AoTTG 2 Contributors — https://github.com/AoTTG2-Archive/AoTTG-2
- 라이선스: [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
- 변경 사항: 원본 에셋을 이 엔진 전용 포맷(`.model`, `.mesh`, `.mat` 등)으로 변환했으며,
  일부는 크기·구성을 수정했습니다.
- 위 에셋 및 이를 변경한 결과물은 동일하게 **CC BY-NC-SA 4.0** 으로 배포되며,
  **상업적 용도로 사용할 수 없습니다.**
- 원 라이선스에 따라 에셋은 "있는 그대로(as-is)" 제공되며, 어떠한 보증도 하지 않습니다.

이 프로젝트는 AoTTG 2 팀, 원작 게임 *Attack on Titan Tribute Game* 의 개발자 FengLee,
그리고 『진격의 거인』 권리자와 **무관하며, 이들의 후원이나 승인을 받지 않았습니다.**

## 서드파티 라이브러리

`ThirdParty/` 의 라이브러리(Dear ImGui, ImGuizmo, DirectXTK, Effects11, nlohmann/json, magic_enum, FMOD, Assimp 등)는
각자의 라이선스를 따릅니다.
