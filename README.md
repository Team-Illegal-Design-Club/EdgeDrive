# Edge Drive

Unreal Engine 5.4로 제작한 3D 액션 게임입니다. 4명으로 시작했고, 한 명이 빠진 뒤 3명이 작업했습니다.

[플레이 영상](https://www.youtube.com/watch?v=264Wfi25UOU) · [Windows 배포 빌드](https://drive.google.com/file/d/1XqZQiWAvwnG1kajS_e9Vc4t5MeMhALR_/view) · [포트폴리오 설명](https://haxx0rbunny.github.io/index.html#project-1)

## HaXX0rBunny 담당 작업

팀장으로 스테이지와 플레이어 캐릭터 전체, 시간 제어를 맡았습니다. 게임플레이는 Blueprint로 구현했습니다.

- 스테이지 제작, Motion Matching 기반 캐릭터 이동과 전투 동작 구현
- Time Dilation을 이용한 시간 제어와 Tick 의존 로직 분리
- 시간 제어와 비 파티클 연동, Niagara 스케일 파라미터 보간
- 패키징 로그와 프로젝트 참조 경로를 추적해 빌드 실패 복구
- 작업 우선순위와 진행 상황 조율, Git 기반 프로젝트 설정 관리

다른 두 팀원은 각각 VFX·SFX·UI와 Enemy AI를 담당했습니다.

주 사용 기술: Unreal Engine 5.4, Blueprint, Git, GitHub, Google Drive

[시간 제어와 비 파티클 Blueprint 구현 기록](https://github.com/HaXX0rBunny/HaXX0rBunny.github.io/blob/main/docs/edgedrive-blueprint.md)에서 로컬 에셋에 포함된 함수 호출을 확인할 수 있습니다.

## 이 저장소의 공개 범위

`main` 브랜치에는 프로젝트 설정, 기본 C++ 모듈, 외부 개발 플러그인이 있습니다. [Sanghyeon 브랜치의 Content](https://github.com/Team-Illegal-Design-Club/EdgeDrive/tree/Sanghyeon/Content)에는 일부 캐릭터 Blueprint와 입력 에셋, 테스트 레벨이 있습니다. 시간 제어와 비 파티클 Blueprint는 현재 위 공개 브랜치에 포함돼 있지 않습니다.

`main`의 `Source/`는 기본 모듈이며 위 담당 게임플레이 구현에 해당하지 않습니다. 공개 파일만으로 배포 영상과 같은 게임을 재현할 수 없으므로, 실행 결과는 영상과 배포 빌드에서 확인해 주세요.

| 경로 | 내용 |
|---|---|
| [EdgeDrive.uproject](EdgeDrive.uproject) | Unreal Engine 5.4 프로젝트 및 모듈 설정 |
| [Source/EdgeDrive](Source/EdgeDrive) | 기본 게임 모듈과 빌드 설정 |
| [Config](Config) | 프로젝트 환경 설정 |
| [Plugins](Plugins) | GitSourceControl, ProjectCleaner 등 외부 개발 도구 |

`Plugins/`의 외부 도구 소스는 각 플러그인 제작자의 작업이며, 위 담당 게임플레이 구현에 해당하지 않습니다.

## 플레이 확인

영상으로 캐릭터 움직임과 전투를 확인할 수 있습니다. 직접 실행하려면 Windows 배포 빌드를 내려받아 압축을 모두 해제한 뒤, 압축에 포함된 게임 실행 파일을 실행하세요. 실행 파일과 함께 제공되는 폴더를 같은 위치에 유지해야 합니다.
