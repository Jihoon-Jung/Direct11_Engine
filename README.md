# Jihoon Engine

Direct3D 11 기반의 3D 게임 엔진으로, 컴포넌트 기반 아키텍처를 통해 GameObject에 다양한 컴포넌트를 부착하여 유연하고 직관적인 게임 로직 구성이 가능하도록 설계되었습니다.

## 사용한 라이브러리

* **Direct11** 
* **DirectXMath**
* **SimpleMath**
* **DirectXTex**
* **ImGUI**
* **ImGuizmo**

## 주요 기능

### 통합 에디터 인터페이스
* ImGUI를 활용하여 **직관적인 인터페이스**로 코드 작성 없이 게임 요소를 시각적으로 구성 및 편집
* Inspector, Hierarchy, Scene View 등을 통해 게임 객체를 직관적으로 편집 가능

### 컴포넌트 기반 설계
* Inspector 창을 통해 게임 오브젝트의 속성과 컴포넌트를 실시간으로 조작 및 수정
* 기본 컴포넌트(Transform, MeshRenderer, Collider, Animator, Camera, Light) 지원
* 개발자가 직접 만든 컴포넌트를 엔진에 등록하고, 컴포넌트 팩토리에서 관리하여 UI에서 쉽게 추가할 수 있도록 함

### 동적 함수 등록 및 이벤트 처리
* 컴포넌트 내의 함수를 이벤트 처리용으로 등록하여 애니메이션 이벤트나 UI 버튼 등에서 드롭다운으로 선택하고 동적 호출 가능

### Transform
* 부모-자식 관계 설정(유니티 유사), UI 상에서 직관적인 오브젝트 배치 및 수정

### 씬 관리
* **XML 포맷**으로 씬 데이터를 저장·로드하여 다양한 씬 구성을 간편하게 관리

### Model & Animation
* Assimp 라이브러리를 통해 Mesh와 애니메이션 정보를 **Binary파일**로 추출하여 게임 내에서 효율적으로 사용 가능하도록 지원
* 노드 기반 Animator로 클립 연결, Loop·Speed·Duration 설정, Event 처리 지원
* 본 계층 구조에 외부 오브젝트 부착 및 최적화된 업데이트 시스템

### 그래픽스 기능
* **Terrain**: LOD와 Frustum Culling을 적용해 최적화된 지형 렌더링
* **Particle System**: 입자의 텍스처, 속도, 형태 등을 제어하여 다양한 파티클 효과 연출
* **Shadow Map**: 빛 방향을 유지하며 카메라를 추적해 항상 안정적인 그림자 표시
* Lighting, Skybox, Environment Map 지원

### 모드 전환
* Edit, Play, Pause 모드를 제공해 자유롭게 개발 환경을 전환

### 물리 시스템
* Collider를 사용한 충돌 처리 지원
* **Sort and Sweep 알고리즘**을 활용한 효율적인 충돌 감지

### UI 시스템
* 이미지, 버튼 등 기본 UI 요소 제공
* 좌표계 변환을 통한 정확한 UI 피킹 구현

---

## 엔진 레이아웃 및 기능

### Scene View
* 게임에 등장하는 각종 게임 오브젝트가 배치된 씬을 시각적으로 편집하고 확인할 수 있는 공간
* Scene View 내에서 마우스로 오브젝트 피킹 가능, Guizmo를 통한 Transform 변경 가능

### Hierachy창
* Scene에 포함된 모든 게임 오브젝트를 계층 구조(부모-자식 관계)로 표시
* 오브젝트를 선택, 새로 생성하거나, 오브젝트를 끌어 다른 오브젝트의 자식으로 만들 수도 있음

### Inspector 창
* 선택된 오브젝트와 연결된 다양한 컴포넌트의 속성을 상세하게 편집 가능
* 변경한 속성은 즉시 Scene에 반영되며, 관련 기능이나 UI에도 반영되어 작업 상황을 바로 확인할 수 있음
* Component 추가, 삭제 기능 지원

### Project창
* 프로젝트에서 사용하는 모든 에셋을 폴더 구조로 정리해 표시 및 관리하는 창
* 이미지, 스크립트, Material 파일 등 미리보기 및 편집 지원
* **Drag & Drop**으로 Scene에 에셋 배치 가능

### Tool Bar
* View 전환 버튼 (Scene/Game View)
* 플레이 모드 제어 (Play, Pause, Stop)
* SkyBox 변경 등 환경 설정

---

## 시스템 구조

### 주요 관리 클래스
* **EngineBody**: 전체 엔진 시스템 통합 관리
* **SceneManager**: Scene 생명주기 및 데이터 관리
* **RenderManager**: 렌더링 시스템 총괄
* **ResourceManager**: 모든 리소스(Mesh, Texture, Shader 등) 관리

### 컴포넌트 시스템
* 모든 컴포넌트는 `Component` 클래스를 상속
* Transform, MeshRenderer, Camera, Light 등 기본 컴포넌트 제공
* `MonoBehaviour`를 상속한 사용자 정의 스크립트 컴포넌트 지원

### 렌더링 파이프라인
* Material, Shader, Buffer, RenderPass 등으로 구성된 유연한 렌더링 시스템
* 셰이더 리소스 관리를 위한 ShaderSlot 시스템
* 다양한 렌더링 기법(Shadow Mapping, Skybox, Particle) 지원

### 애니메이션 시스템
* 여러 애니메이션 클립 관리 및 블렌딩
* 트랜지션 시스템으로 자연스러운 애니메이션 전환
* 파라미터 기반 애니메이션 제어
* GPU 기반 스켈레탈 애니메이션 최적화

### 물리 및 충돌 시스템
* Box, Sphere 등 기본 Collider 제공
* Sort and Sweep 알고리즘 기반 효율적인 충돌 감지
* 계층적 바운딩 볼륨 구조로 최적화

### 파일 형식 및 에셋 관리
* XML 기반 Scene, Material 데이터 관리
* 바이너리 파일 형식(`.mesh`, `.clip`)으로 모델 및 애니메이션 데이터 저장
* Assimp 라이브러리를 이용한 외부 3D 모델 변환
