#include "pch.h"
#include <string>
#include "GUIManager.h"
#include "MoveObject.h"
#include <algorithm>

void GUIManager::Init()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight();

    // 타이틀바 색상 설정
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

    // 폰트 설정
    ImFont* font = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 13.0f);
    io.FontDefault = font;

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(GP.GetWindowHandle());
	ImGui_ImplDX11_Init(GP.GetDevice().Get(), GP.GetDeviceContext().Get());

    // ImGuizmo 초기화
    ImGuizmo::Enable(true);  // ImGuizmo 활성화
    ImGuizmo::SetGizmoSizeClipSpace(0.14f);

    // 초기 선택 폴더를 Resource 폴더로 설정
    _selectedFolder = "Resource/";
    _isFirstFrame = true;

    // EmptyObject 관련 초기화
    _resourceListInitialized = false;
    _showEmptyObjectPopup = false;
    _newObjectUseMeshRenderer = false;
    _newObjectSelectedMesh = 0;
    _newObjectSelectedMaterial = 0;
    strcpy_s(_newObjectName, "NewObject");
    _newObjectPosition = Vec3::Zero;
    _newObjectRotation = Vec3::Zero;
    _newObjectScale = Vec3(1.0f, 1.0f, 1.0f);
}

float EaseInOutCubic(float t)
{
    return t < 0.5f ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
}

void GUIManager::Update()
{
    if (_isCameraMoving)
    {
        _cameraMoveTime += TIME.GetDeltaTime() * _cameraMoveSpeed;
        if (_cameraMoveTime >= 1.0f)
        {
            _cameraMoveTime = 1.0f;
            _isCameraMoving = false;
        }

        shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
        if (camera)
        {
            // 부드러운 보간을 위해 easeInOutCubic 함수 사용
            float t = EaseInOutCubic(_cameraMoveTime);

            // 위치 보간
            Vec3 newPos = Vec3::Lerp(_cameraStartPos, _cameraTargetPos, t);

            // 회전 보간 (쿼터니온 사용)
            Quaternion startRot = Quaternion::CreateFromYawPitchRoll(
                XMConvertToRadians(_cameraStartRot.y),
                XMConvertToRadians(_cameraStartRot.x),
                XMConvertToRadians(_cameraStartRot.z)
            );

            Quaternion targetRot = Quaternion::CreateFromYawPitchRoll(
                XMConvertToRadians(_cameraTargetRot.y),
                XMConvertToRadians(_cameraTargetRot.x),
                XMConvertToRadians(_cameraTargetRot.z)
            );

            Quaternion newRot = Quaternion::Slerp(startRot, targetRot, t);

            // 변환된 값 적용
            camera->transform()->SetLocalPosition(newPos);
            camera->transform()->SetQTRotation(newRot);
        }
    }

    RenderUI_Start();
    RenderUI();
    RenderUI_End();
}

void GUIManager::RenderUI_Start()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

    ImGuizmo::BeginFrame();  // ImGuizmo 프레임 시작
}

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
        return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}


void GUIManager::RenderUI()
{
    // 독립적인 팝업창 렌더링
    if (_showEmptyObjectPopup)
    {
        // 팝업 창 크기와 위치 설정
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300));

        if (ImGui::Begin("Create Empty Object", &_showEmptyObjectPopup, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
        {
            // Object Name
            ImGui::Text("Object Name:");
            ImGui::InputText("##ObjectName", _newObjectName, IM_ARRAYSIZE(_newObjectName));

            // Transform
            ImGui::Separator();
            ImGui::Text("Transform:");
            ImGui::DragFloat3("Position", &_newObjectPosition.x, 0.1f);
            ImGui::DragFloat3("Rotation", &_newObjectRotation.x, 0.1f);
            ImGui::DragFloat3("Scale", &_newObjectScale.x, 0.1f);

            // MeshRenderer
            ImGui::Separator();
            if (ImGui::Checkbox("Use MeshRenderer", &_newObjectUseMeshRenderer))
            {
                // Checkbox가 변경될 때 리소스 목록 초기화
                _resourceListInitialized = false;
            }

            if (_newObjectUseMeshRenderer)
            {
                // 리소스 목록 초기화
                if (!_resourceListInitialized)
                {
                    // Mesh 리스트 가져오기
                    _meshList.clear();
                    filesystem::path meshPath = "Resource/Mesh";
                    if (filesystem::exists(meshPath))
                    {
                        for (const auto& entry : filesystem::directory_iterator(meshPath))
                        {
                            if (entry.path().extension() == ".xml")
                            {
                                _meshList.push_back(entry.path().stem().wstring());
                            }
                        }
                    }

                    // Material 리스트 가져오기
                    _materialList.clear();
                    filesystem::path materialPath = "Resource/Material";
                    if (filesystem::exists(materialPath))
                    {
                        for (const auto& entry : filesystem::directory_iterator(materialPath))
                        {
                            if (entry.path().extension() == ".xml")
                            {
                                wstring materialName = entry.path().stem().wstring();
                                shared_ptr<Material> material = RESOURCE.GetResource<Material>(materialName);
                                if (material && material->GetShader())
                                {
                                    wstring shaderName = material->GetShader()->GetName();
                                    if (shaderName == L"Simple_Render_Shader" || shaderName == L"Default_Shader")
                                    {
                                        _materialList.push_back(materialName);
                                    }
                                }
                            }
                        }
                    }
                    _resourceListInitialized = true;
                }

                // Mesh 선택 콤보박스
                if (!_meshList.empty())
                {
                    ImGui::Text("Mesh:"); ImGui::SameLine();
                    string previewValue = _newObjectSelectedMesh < _meshList.size() ?
                        Utils::ToString(_meshList[_newObjectSelectedMesh]) : "Select Mesh";
                    if (ImGui::BeginCombo("##MeshCombo", previewValue.c_str()))
                    {
                        for (int i = 0; i < _meshList.size(); i++)
                        {
                            bool isSelected = (_newObjectSelectedMesh == i);
                            if (ImGui::Selectable(Utils::ToString(_meshList[i]).c_str(), isSelected))
                                _newObjectSelectedMesh = i;
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Mesh files found in Resource/Mesh");
                }

                // Material 선택 콤보박스
                if (!_materialList.empty())
                {
                    ImGui::Text("Material:"); ImGui::SameLine();
                    string previewValue = _newObjectSelectedMaterial < _materialList.size() ?
                        Utils::ToString(_materialList[_newObjectSelectedMaterial]) : "Select Material";
                    if (ImGui::BeginCombo("##MaterialCombo", previewValue.c_str()))
                    {
                        for (int i = 0; i < _materialList.size(); i++)
                        {
                            bool isSelected = (_newObjectSelectedMaterial == i);
                            if (ImGui::Selectable(Utils::ToString(_materialList[i]).c_str(), isSelected))
                                _newObjectSelectedMaterial = i;
                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                        "No Material files found with Simple_Render_Shader or Default_Shader");
                }
            }

            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                wstring objectName = Utils::ToWString(_newObjectName);
                wstring sceneName = SCENE.GetActiveScene()->GetSceneName();

                // 이름 중복 체크
                if (SCENE.GetActiveScene()->Find(objectName) != nullptr)
                {
                    // 경고 메시지 팝업
                    ImGui::OpenPopup("Name Exists");
                }
                else
                {
                    // GameObject 생성 및 저장
                    SCENE.SaveAndLoadGameObjectToXML(sceneName, objectName, _newObjectPosition, _newObjectRotation, _newObjectScale);

                    // MeshRenderer 추가
                    if (_newObjectUseMeshRenderer && _newObjectSelectedMesh < _meshList.size() && _newObjectSelectedMaterial < _materialList.size())
                    {
                        auto meshRenderer = make_shared<MeshRenderer>();
                        meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(_meshList[_newObjectSelectedMesh]));
                        meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(_materialList[_newObjectSelectedMaterial]));
                        meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
                        meshRenderer->AddRenderPass();
                        meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
                        meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
                        meshRenderer->GetRenderPasses()[0]->SetTransform(SCENE.GetActiveScene()->Find(objectName)->transform());
                        meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);

                        // MeshRenderer 컴포넌트를 GameObject에 추가하고 XML에 저장
                        SCENE.AddComponentToGameObjectAndSaveToXML(sceneName, objectName, meshRenderer,
                            _materialList[_newObjectSelectedMaterial], _meshList[_newObjectSelectedMesh]);
                    }

                    // 초기화
                    strcpy_s(_newObjectName, "NewObject");
                    _newObjectPosition = Vec3::Zero;
                    _newObjectRotation = Vec3::Zero;
                    _newObjectScale = Vec3(1.0f, 1.0f, 1.0f);
                    _newObjectUseMeshRenderer = false;
                    _newObjectSelectedMesh = 0;
                    _newObjectSelectedMaterial = 0;

                    _showEmptyObjectPopup = false;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                _showEmptyObjectPopup = false;
            }

            ImGui::End();
        }
    }
    // controller
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    {
        // 창 테두리 스타일 설정
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

        float windowSizeX = GP.GetViewWidth() * (7.0f / 10.0f);
        float windowSizeY = GP.GetViewHeight() * (3.0f / 100.0f);
        ImGui::SetNextWindowPos(ImVec2(GP.GetViewWidth() * (1.0f / 10.0f), 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowSizeX, windowSizeY));
        ImGui::Begin("Manual", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);

        // 버튼 크기와 간격 설정
        float buttonWidth = windowSizeX * (0.2f / 10.0f);
        float buttonHeight = windowSizeY * (6.0f / 10.0f);
        float spacing = windowSizeX * (0.1f / 100.0f);
        float totalWidth = (buttonWidth * 3) + (spacing * 2);

        // 창의 중앙 위치 계산
        float windowWidth = ImGui::GetWindowSize().x;
        float windowHeight = ImGui::GetWindowSize().y;
        float startX = (windowWidth - totalWidth) * 0.5f;
        float startY = (windowHeight - buttonHeight) * 0.5f;

        // 커서 위치 설정
        ImGui::SetCursorPos(ImVec2(startX, startY));

        // 버튼 스타일 설정
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

        // 이미지 버튼 생성
        if (ImGui::ImageButton("Play", (ImTextureID)(RESOURCE.GetResource<Texture>(L"startButton")->GetShaderResourceView().Get()), ImVec2(buttonWidth, buttonHeight)))
        {
            if (ENGINE.IsStopped() || ENGINE.IsPaused())
            {
                ENGINE.Play();
            }
        }
        ImGui::SameLine(0, spacing);

        if (ImGui::ImageButton("Pause", (ImTextureID)(RESOURCE.GetResource<Texture>(L"pauseButton")->GetShaderResourceView().Get()), ImVec2(buttonWidth, buttonHeight)))
        {
            if (ENGINE.IsPlaying())
            {
                ENGINE.Pause();
            }
            else if (ENGINE.IsPaused())
            {
                ENGINE.Resume();
            }
        }
        ImGui::SameLine(0, spacing);

        if (ImGui::ImageButton("Next", (ImTextureID)(RESOURCE.GetResource<Texture>(L"stopButton")->GetShaderResourceView().Get()), ImVec2(buttonWidth, buttonHeight)))
        {
            ENGINE.Stop();
        }

        // 스타일 복원
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        

        ImGui::End();

        // 창 테두리 스타일 복원
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

    }

    // Hierachy
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GP.GetViewWidth() * (1.0f / 10.0f), GP.GetViewHeight() * (63.0f / 100.0f)));
        ImGui::Begin("Hierachy", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse);

        vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();

        // Scene TreeNode를 기본적으로 열린 상태로 설정
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);

        if (ImGui::TreeNode("Scene"))
        {
            for (const auto& gameObject : gameObjects)
            {
                if (gameObject->GetParent() == nullptr)
                {
                    RenderGameObjectHierarchy(gameObject);
                }
            }
            ImGui::TreePop();
        }

        // Hierarchy 창 내 우클릭 메뉴
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::BeginMenu("Create"))
            {
                // EmptyObject 메뉴 아이템 클릭 처리 부분 수정
                if (ImGui::MenuItem("EmptyObject##CreateMenu1"))
                {
                    // 새로운 오브젝트 생성 시 기본 이름 설정
                    int count = 1;
                    string baseName = "NewObject";
                    string newName = baseName;

                    // 이미 존재하는 오브젝트 이름인지 확인
                    while (SCENE.GetActiveScene()->Find(Utils::ToWString(newName)) != nullptr)
                    {
                        newName = baseName + to_string(count);
                        count++;
                    }

                    strcpy_s(_newObjectName, newName.c_str());
                    _showEmptyObjectPopup = true;
                }

                if (ImGui::MenuItem("Cube##CreateMenu2"))
                {
                    SCENE.CreateCubeToScene(SCENE.GetActiveScene()->GetSceneName());
                    // TODO: NULL2 구현 예정
                }
                if (ImGui::MenuItem("Sphere##CreateMenu3"))
                {
                    SCENE.CreateSphereToScene(SCENE.GetActiveScene()->GetSceneName());
                    // TODO: NULL3 구현 예정
                }
                if (ImGui::MenuItem("Cylinder##CreateMenu4"))
                {
                    SCENE.CreateCylinderToScene(SCENE.GetActiveScene()->GetSceneName());
                    // TODO: NULL3 구현 예정
                }
                if (ImGui::MenuItem("Plane##CreateMenu5"))
                {
                    // TODO: NULL3 구현 예정
                }
                if (ImGui::MenuItem("Quad##CreateMenu6"))
                {
                    // TODO: NULL3 구현 예정
                }
                if (ImGui::MenuItem("Terrain##CreateMenu7"))
                {
                    // TODO: NULL3 구현 예정
                }
                if (ImGui::MenuItem("Particel System##CreateMenu8"))
                {
                    // TODO: NULL3 구현 예정
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    // Inspector
    {
        ImGui::SetNextWindowPos(ImVec2(GP.GetViewWidth()* (8.0f / 10.0f), 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GP.GetViewWidth()* (2.0f / 10.0f), GP.GetViewHeight()* (63.0f / 100.0f)));
        ImGui::Begin("Inspector", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse);

        if (_selectedObject != nullptr)
        {
            // 오브젝트 이름 표시
            ImGui::Text("Name: %s", WStringToString(_selectedObject->GetName()).c_str());
            ImGui::Separator();

            // Transform은 항상 먼저 표시
            shared_ptr<Transform> transform = _selectedObject->GetComponent<Transform>();

            if (transform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static Vec3 currentEulerAngles = Vec3::Zero;
                static bool initialized = false;
                static GameObject* lastObject = nullptr;
                static Vec3 lastRotation = Vec3::Zero;

                // 현재 회전 상태 가져오기
                Matrix currentRotationMatrix = Matrix::CreateFromQuaternion(transform->GetQTRotation());

                // 현재 로컬 축 계산
                Vec3 localRight = Vec3::TransformNormal(Vec3::Right, currentRotationMatrix);
                Vec3 localUp = Vec3::TransformNormal(Vec3::Up, currentRotationMatrix);
                Vec3 localForward = Vec3::TransformNormal(Vec3::Forward, currentRotationMatrix);

                // 현재 회전값을 오일러각으로 변환
                Vec3 angles = transform->ToEulerAngles(transform->GetQTRotation());
                currentEulerAngles.x = XMConvertToDegrees(angles.x);
                currentEulerAngles.y = XMConvertToDegrees(angles.y);
                currentEulerAngles.z = XMConvertToDegrees(angles.z);
                lastRotation = currentEulerAngles;
                initialized = true;
                lastObject = _selectedObject.get();
                _rotationUpdated = false;

                Vec3 position = transform->GetLocalPosition();
                Vec3 scale = transform->GetLocalScale();

                if (ImGui::DragFloat3("Position", &position.x, 0.1f))
                    transform->SetLocalPosition(position);

                Vec3 tempRotation = currentEulerAngles;

                // X축 회전
                if (ImGui::DragFloat("Rotation X", &tempRotation.x, 0.1f))
                {
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                    {
                        float deltaAngle = tempRotation.x - lastRotation.x;
                        // 델타 각도가 너무 크면 제한
                        if (abs(deltaAngle) > 10.0f) {
                            deltaAngle = (deltaAngle > 0) ? 10.0f : -10.0f;
                            tempRotation.x = lastRotation.x + deltaAngle;
                        }

                        float angle = XMConvertToRadians(deltaAngle);
                        Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localRight, angle);
                        Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                        transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                        lastRotation.x = tempRotation.x;
                        currentEulerAngles.x = tempRotation.x;
                    }
                    else if (!ImGui::IsItemActive() && ImGui::IsItemDeactivatedAfterEdit())  // 직접 입력 후 포커스를 잃었을 때
                    {
                        transform->SetLocalRotation(tempRotation);
                        lastRotation = tempRotation;
                        currentEulerAngles = tempRotation;
                    }
                }

                // Y축 회전
                if (ImGui::DragFloat("Rotation Y", &tempRotation.y, 0.1f))
                {
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                    {
                        float deltaAngle = tempRotation.y - lastRotation.y;
                        // 델타 각도가 너무 크면 제한
                        if (abs(deltaAngle) > 10.0f) {
                            deltaAngle = (deltaAngle > 0) ? 10.0f : -10.0f;
                            tempRotation.y = lastRotation.y + deltaAngle;
                        }

                        float angle = XMConvertToRadians(deltaAngle);
                        Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localUp, angle);
                        Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                        transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                        lastRotation.y = tempRotation.y;
                        currentEulerAngles.y = tempRotation.y;
                    }
                    else if (!ImGui::IsItemActive() && ImGui::IsItemDeactivatedAfterEdit())  // 직접 입력 후 포커스를 잃었을 때
                    {
                        transform->SetLocalRotation(tempRotation);
                        lastRotation = tempRotation;
                        currentEulerAngles = tempRotation;
                    }
                }

                // Z축 회전
                if (ImGui::DragFloat("Rotation Z", &tempRotation.z, 0.1f))
                {
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                    {
                        float deltaAngle = tempRotation.z - lastRotation.z;
                        // 델타 각도가 너무 크면 제한
                        if (abs(deltaAngle) > 10.0f) {
                            deltaAngle = (deltaAngle > 0) ? 10.0f : -10.0f;
                            tempRotation.z = lastRotation.z + deltaAngle;
                        }

                        float angle = XMConvertToRadians(deltaAngle);
                        Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localForward, angle);
                        Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                        transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                        lastRotation.z = tempRotation.z;
                        currentEulerAngles.z = tempRotation.z;
                    }
                    else if (!ImGui::IsItemActive() && ImGui::IsItemDeactivatedAfterEdit())  // 직접 입력 후 포커스를 잃었을 때
                    {
                        transform->SetLocalRotation(tempRotation);
                        lastRotation = tempRotation;
                        currentEulerAngles = tempRotation;
                    }
                }

                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                    transform->SetLocalScale(scale);
                SCENE.UpdateGameObjectTransformInXML(L"test_scene", _selectedObject->GetName(),
                    position, transform->GetLocalRotation(), transform->GetLocalScale());

                ImGui::Separator();
            }
            // 나머지 컴포넌트들 표시
            const vector<shared_ptr<Component>>& components = _selectedObject->GetComponents();
            for (const auto& component : components)
            {
                // Transform은 이미 표시했으므로 스킵
                if (dynamic_pointer_cast<Transform>(component))
                    continue;

                // 컴포넌트 타입에 따른 이름 설정
                string componentName;
                if (dynamic_pointer_cast<MeshRenderer>(component))
                    componentName = "Mesh Renderer";
                else if (dynamic_pointer_cast<Camera>(component))
                    componentName = "Camera";
                else if (dynamic_pointer_cast<Light>(component))
                    componentName = "Light";
                else if (dynamic_pointer_cast<BoxCollider>(component))
                    componentName = "BoxCollider";
                else if (dynamic_pointer_cast<SphereCollider>(component))
                    componentName = "SphereCollider";
                else if (dynamic_pointer_cast<MoveObject>(component))
                    componentName = "MoveObject";
                // ... 다른 컴포넌트 타입들 추가 ...

                if (!componentName.empty())
                {
                    if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        // 컴포넌트 제거 메뉴
                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Delete"))
                            {
                                // 오브젝트에서 컴포넌트 제거
                                _selectedObject->RemoveComponent(component);

                                // XML 업데이트
                                SCENE.RemoveComponentFromGameObjectInXML(
                                    SCENE.GetActiveScene()->GetSceneName(),
                                    _selectedObject->GetName(),
                                    component
                                );
                            }
                            ImGui::EndPopup();
                        }
                        // 각 컴포넌트 타입별 속성 표시
                        // TODO: 컴포넌트별 세부 속성 표시 로직 추가

                        if (auto boxCollider = dynamic_pointer_cast<BoxCollider>(component))
                        {
                            // Center
                            Vec3 center = boxCollider->GetCenter();
                            if (ImGui::DragFloat3("Center", &center.x, 0.1f))
                            {
                                boxCollider->SetCenter(center);
                                SCENE.UpdateGameObjectColliderInXML(L"test_scene", _selectedObject->GetName(),
                                    center, boxCollider->GetScale(), true);
                            }

                            // Size
                            Vec3 scale = boxCollider->GetScale();
                            if (ImGui::DragFloat3("Size", &scale.x, 0.1f))
                            {
                                boxCollider->SetScale(scale);
                                SCENE.UpdateGameObjectColliderInXML(L"test_scene", _selectedObject->GetName(),
                                    boxCollider->GetCenter(), scale, true);
                            }

                            // Edit Mode Toggle
                            if (ImGui::Button(_isColliderEditMode ? "Exit Edit Mode" : "Edit Mode"))
                            {
                                _isColliderEditMode = !_isColliderEditMode;
                            }
                        }
                        else if (auto sphereCollider = dynamic_pointer_cast<SphereCollider>(component))
                        {
                            // Center
                            Vec3 center = sphereCollider->GetCenter();
                            if (ImGui::DragFloat3("Center", &center.x, 0.1f))
                            {
                                sphereCollider->SetCenter(center);
                                wstring currentSceneName = SCENE.GetActiveScene()->GetSceneName();
                                SCENE.UpdateGameObjectSphereColliderInXML(currentSceneName, _selectedObject->GetName(),
                                    center, sphereCollider->GetRadius());
                            }

                            // Radius - 실제 크기 반영을 위해 수정
                            float radius = sphereCollider->GetRadius();
                            if (ImGui::DragFloat("Radius", &radius, 0.1f))
                            {
                                sphereCollider->SetRadius(radius);
                                sphereCollider->SetScale(Vec3(radius, radius, radius)); // Scale 설정 추가
                                wstring currentSceneName = SCENE.GetActiveScene()->GetSceneName();
                                SCENE.UpdateGameObjectSphereColliderInXML(currentSceneName, _selectedObject->GetName(),
                                    center, radius);
                            }

                            // Edit Mode Toggle
                            if (ImGui::Button(_isColliderEditMode ? "Exit Edit Mode" : "Edit Mode"))
                            {
                                _isColliderEditMode = !_isColliderEditMode;
                            }
                        }

                        else if (auto meshRenderer = dynamic_pointer_cast<MeshRenderer>(component))
                        {
                            // 스타일 설정
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

                            float labelWidth = 100.0f;
                            float valueWidth = ImGui::GetContentRegionAvail().x - labelWidth - 20.0f;

                            // 입력 필드 스타일 설정
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

                            // RenderPass
                            ImGui::Text("RenderPass"); ImGui::SameLine(labelWidth);
                            if (ImGui::Button("..##RenderPass", ImVec2(20, 0))) ImGui::OpenPopup("RenderPassSelectPopup");
                            ImGui::SameLine();
                            Pass passType = meshRenderer->GetRenderPasses()[0]->GetPass();
                            wstring passName = L"";
                            switch (passType)
                            {
                                case Pass::DEFAULT_RENDER:
                                    passName = L"DEFAULT_RENDER";
                                    break;
                                case Pass::OUTLINE_RENDER:
                                    passName = L"OUTLINE_RENDER";
                                    break;
                                case Pass::GAUSSIANBLUR_RENDER:
                                    passName = L"GAUSSIANBLUR_RENDER";
                                    break;
                                case Pass::SHADOWMAP_RENDER:
                                    passName = L"SHADOWMAP_RENDER";
                                    break;
                                case Pass::QUAD_RENDER:
                                    passName = L"QUAD_RENDER";
                                    break;
                                case Pass::TESSELLATION_RENDER:
                                    passName = L"TESSELLATION_RENDER";
                                    break;
                                case Pass::TERRAIN_RENDER:
                                    passName = L"TERRAIN_RENDER";
                                    break;
                                case Pass::ENVIRONMENTMAP_RENDER:
                                    passName = L"ENVIRONMENTMAP_RENDER";
                                    break;
                                case Pass::STATIC_MESH_RENDER:
                                    passName = L"STATIC_MESH_RENDER";
                                    break;
                                case Pass::ANIMATED_MESH_RENDER:
                                    passName = L"ANIMATED_MESH_RENDER";
                                    break;
                                case Pass::PARTICLE_RENDER:
                                    passName = L"PARTICLE_RENDER";
                                    break;
                                case Pass::DEBUG_2D_RENDER:
                                    passName = L"DEBUG_2D_RENDER";
                                    break;
                                default:
                                    break;
                            }
                            string passNameStr = WStringToString(passName);

                            ImGui::SetNextItemWidth(valueWidth);
                            ImGui::InputText("##RenderPassValue", (char*)passNameStr.c_str(), passNameStr.size(), ImGuiInputTextFlags_ReadOnly);

                            // Mesh
                            ImGui::Text("Mesh"); ImGui::SameLine(labelWidth);
                            if (ImGui::Button("..##Mesh", ImVec2(20, 0))) ImGui::OpenPopup("MeshSelectPopup");
                            ImGui::SameLine();

                            wstring meshName = L"None";
                            shared_ptr<Mesh> mesh = meshRenderer->GetMesh();
                            if (mesh != nullptr)
                                meshName = mesh->GetMeshName();
                            string meshNameStr = WStringToString(meshName);

                            ImGui::SetNextItemWidth(valueWidth);
                            ImGui::InputText("##MeshValue", (char*)meshNameStr.c_str(), meshNameStr.size(), ImGuiInputTextFlags_ReadOnly);

                            // Model
                            ImGui::Text("Model"); ImGui::SameLine(labelWidth);
                            if (ImGui::Button("..##Model", ImVec2(20, 0))) ImGui::OpenPopup("ModelSelectPopup");
                            ImGui::SameLine();

                            wstring modelName = L"None";
                            shared_ptr<Model> model = meshRenderer->GetModel();
                            if (model != nullptr)
                                modelName = model->GetModelName();
                            string modelNameStr = WStringToString(modelName);

                            ImGui::SetNextItemWidth(valueWidth);
                            ImGui::InputText("##ModelValue", (char*)modelNameStr.c_str(), modelNameStr.size(), ImGuiInputTextFlags_ReadOnly);

                            // Material
                            ImGui::Text("Material"); ImGui::SameLine(labelWidth);
                            if (ImGui::Button("..##Material", ImVec2(20, 0))) ImGui::OpenPopup("MaterialSelectPopup");
                            ImGui::SameLine();

                            wstring materialName = L"None";
                            shared_ptr<Material> material = meshRenderer->GetMaterial();
                            if (material != nullptr)
                                materialName = material->GetMaterialName();
                            string materialNameStr = WStringToString(materialName);

                            ImGui::SetNextItemWidth(valueWidth);
                            ImGui::InputText("##MaterialValue", (char*)materialNameStr.c_str(), materialNameStr.size(), ImGuiInputTextFlags_ReadOnly);

                            // 팝업 처리
                            if (ImGui::BeginPopup("MeshSelectPopup"))
                            {
                                ImGui::SetNextWindowSize(ImVec2(300, 400));
                                ImGui::Text("Mesh List");
                                ImGui::Separator();

                                // 검색창 추가
                                static char searchBuffer[128] = "";
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::InputText("##Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));
                                ImGui::Separator();

                                // Mesh 목록을 표시할 Child 윈도우
                                if (ImGui::BeginChild("MeshList", ImVec2(0, 0), true))
                                {
                                    string searchStr = string(searchBuffer);
                                    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                                    for (const auto& entry : filesystem::directory_iterator("Resource/Mesh"))
                                    {
                                        if (entry.path().extension() == ".xml")
                                        {
                                            string meshName = entry.path().stem().string();

                                            // 검색어 필터링
                                            string lowerMeshName = meshName;
                                            std::transform(lowerMeshName.begin(), lowerMeshName.end(), lowerMeshName.begin(), ::tolower);
                                            if (searchStr.empty() || lowerMeshName.find(searchStr) != string::npos)
                                            {
                                                if (ImGui::Selectable(meshName.c_str()))
                                                {
                                                    meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(Utils::ToWString(meshName)));
                                                    SCENE.UpdateMeshInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedObject->GetName(), meshName);
                                                    ImGui::CloseCurrentPopup();
                                                }
                                            }
                                        }
                                    }
                                    ImGui::EndChild();
                                }
                                ImGui::EndPopup();
                            }

                            if (ImGui::BeginPopup("MaterialSelectPopup"))
                            {
                                ImGui::SetNextWindowSize(ImVec2(300, 400));
                                ImGui::Text("Material List");
                                ImGui::Separator();

                                // 검색창 추가
                                static char materialSearchBuffer[128] = "";
                                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                                ImGui::InputText("##MaterialSearch", materialSearchBuffer, IM_ARRAYSIZE(materialSearchBuffer));
                                ImGui::Separator();

                                // Material 목록을 표시할 Child 윈도우
                                if (ImGui::BeginChild("MaterialList", ImVec2(0, 0), true))
                                {
                                    string searchStr = string(materialSearchBuffer);
                                    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                                    for (const auto& entry : filesystem::directory_iterator("Resource/Material"))
                                    {
                                        if (entry.path().extension() == ".xml")
                                        {
                                            // XML 파일 파싱
                                            tinyxml2::XMLDocument doc;
                                            if (doc.LoadFile(entry.path().string().c_str()) == tinyxml2::XML_SUCCESS)
                                            {
                                                tinyxml2::XMLElement* root = doc.FirstChildElement("Material");
                                                if (root)
                                                {
                                                    tinyxml2::XMLElement* shaderElement = root->FirstChildElement("Shader");
                                                    if (shaderElement)
                                                    {
                                                        const char* shaderType = shaderElement->GetText();
                                                        if (shaderType && (strcmp(shaderType, "Simple_Render_Shader") == 0 ||
                                                            strcmp(shaderType, "Default_Shader") == 0))
                                                        {
                                                            string materialName = entry.path().stem().string();

                                                            // 검색어 필터링
                                                            string lowerMaterialName = materialName;
                                                            std::transform(lowerMaterialName.begin(), lowerMaterialName.end(), lowerMaterialName.begin(), ::tolower);
                                                            if (searchStr.empty() || lowerMaterialName.find(searchStr) != string::npos)
                                                            {
                                                                if (ImGui::Selectable(materialName.c_str()))
                                                                {
                                                                    meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(Utils::ToWString(materialName)));
                                                                    SCENE.UpdateMaterialInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedObject->GetName(), materialName);
                                                                    ImGui::CloseCurrentPopup();
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    ImGui::EndChild();
                                }
                                ImGui::EndPopup();
                            }

                            // 스타일 복원
                            ImGui::PopStyleColor(2);
                            ImGui::PopStyleVar();
                            ImGui::Separator();
                        }

                        ImGui::Separator();
                    }
                    
                }
            }

            // 컴포넌트 추가 메뉴
            if (ImGui::BeginPopupContextWindow("AddComponentMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::BeginMenu("Add Component"))
                {
                    // 현재 오브젝트의 컴포넌트 목록 확인
                    const vector<shared_ptr<Component>>& components = _selectedObject->GetComponents();
                    bool hasBoxCollider = false;
                    bool hasSphereCollider = false;
                    bool hasMeshRenderer = false;

                    // 기존 컴포넌트 체크
                    for (const auto& component : components)
                    {
                        if (dynamic_pointer_cast<BoxCollider>(component))
                            hasBoxCollider = true;
                        else if (dynamic_pointer_cast<SphereCollider>(component))
                            hasSphereCollider = true;
                        else if (dynamic_pointer_cast<MeshRenderer>(component))
                            hasMeshRenderer = true;
                    }

                    // BoxCollider 메뉴 아이템
                    if (!hasBoxCollider && ImGui::MenuItem("Box Collider"))
                    {
                        auto boxCollider = make_shared<BoxCollider>();
                        boxCollider->SetCenter(Vec3::Zero);
                        boxCollider->SetScale(Vec3::One);
                        _selectedObject->AddComponent(boxCollider);

                        // XML 업데이트
                        SCENE.AddComponentToGameObjectAndSaveToXML(
                            SCENE.GetActiveScene()->GetSceneName(),
                            _selectedObject->GetName(),
                            boxCollider
                        );
                        ImGui::CloseCurrentPopup();
                    }

                    // SphereCollider 메뉴 아이템
                    if (!hasSphereCollider && ImGui::MenuItem("Sphere Collider"))
                    {
                        auto sphereCollider = make_shared<SphereCollider>();
                        sphereCollider->SetCenter(Vec3::Zero);
                        sphereCollider->SetRadius(0.5f);
                        _selectedObject->AddComponent(sphereCollider);

                        // XML 업데이트
                        SCENE.AddComponentToGameObjectAndSaveToXML(
                            SCENE.GetActiveScene()->GetSceneName(),
                            _selectedObject->GetName(),
                            sphereCollider
                        );
                        ImGui::CloseCurrentPopup();
                    }

                    // MeshRenderer 메뉴 아이템
                    if (!hasMeshRenderer && ImGui::MenuItem("Mesh Renderer"))
                    {
                        auto meshRenderer = make_shared<MeshRenderer>();
                        // 기본 메시와 머티리얼 설정
                        meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
                        meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
                        meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
                        meshRenderer->AddRenderPass();
                        meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
                        meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
                        meshRenderer->GetRenderPasses()[0]->SetTransform(_selectedObject->transform());
                        meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
                        _selectedObject->AddComponent(meshRenderer);

                        // XML 업데이트
                        SCENE.AddComponentToGameObjectAndSaveToXML(
                            SCENE.GetActiveScene()->GetSceneName(),
                            _selectedObject->GetName(),
                            meshRenderer,
                            L"DefaultMaterial",   // material name
                            L"Cube"       // mesh name
                        );
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    // project
    {
        ImGui::Begin("Project", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse);

        float windowWidth = ImGui::GetContentRegionAvail().x;
        float leftPanelWidth = windowWidth * 0.12f;

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(windowPos.x + leftPanelWidth, windowPos.y),
            ImVec2(windowPos.x + leftPanelWidth, windowPos.y + windowSize.y),
            ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.3f))
        );

        // 왼쪽 패널 (폴더 트리)
        {
            // 클리핑 영역 설정
            ImGui::PushClipRect(
                ImVec2(windowPos.x, windowPos.y),
                ImVec2(windowPos.x + leftPanelWidth - 1, windowPos.y + windowSize.y),
                true
            );

            ImGui::BeginChild("FolderTree", ImVec2(leftPanelWidth - 1, 0), false);

            // 트리 스타일 설정
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));

            static filesystem::path currentPath = "Resource/";
            RenderFolderTree(currentPath, _selectedFolder);

            ImGui::PopStyleVar(3);
            ImGui::EndChild();
            ImGui::PopClipRect();
        }

        ImGui::SameLine(0, 1);

        // 오른쪽 패널
        ImGui::SameLine(0, 1);
        ImGui::BeginChild("FilePanel", ImVec2(0, 0), false);

        // 경로 표시 바
        ImGui::BeginChild("PathBar", ImVec2(0, 25), false);
        string relativePath = _selectedFolder.string();
        if (relativePath.find("Resource/") == 0) {
            relativePath = relativePath.substr(9);
        }
        ImGui::Text(" %s", relativePath.c_str());
        ImGui::EndChild();

        // 가로 구분선
        ImVec2 panelPos = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(windowPos.x + leftPanelWidth, panelPos.y + 15),
            ImVec2(panelPos.x + ImGui::GetWindowWidth(), panelPos.y + 15),
            ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f))
        );

        // 세로 구분선
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(windowPos.x + leftPanelWidth, windowPos.y),
            ImVec2(windowPos.x + leftPanelWidth, windowPos.y + windowSize.y),
            ImGui::GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.3f))
        );

        // 파일 그리드
        ImGui::BeginChild("FileGrid", ImVec2(0, 0), false);
        RenderFileGrid(_selectedFolder);
        ImGui::EndChild();

        ImGui::EndChild();
        ImGui::End();
    }

    if (ImGui::Button("Test"))
    {
        GP.test = !GP.test;  // 버튼 클릭 시 GP.test 값을 토글
    }
    RenderGuizmo();
}

void GUIManager::RenderGameObjectHierarchy(shared_ptr<GameObject> gameObject)
{
    string objectName = WStringToString(gameObject->GetName());
    const vector<shared_ptr<GameObject>>& children = gameObject->GetChildren();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;

    // 자식이 있으면 OpenOnArrow, 없으면 Leaf 플래그 추가
    if (!children.empty())
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }
    else
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    // 현재 선택된 오브젝트면 Selected 플래그 추가
    if (_selectedObject == gameObject)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // 이전에 저장된 트리 노드 상태를 설정
    if (gameObject->IsTreeNodeOpen())
    {
        ImGui::SetNextItemOpen(true);
    }

    bool nodeOpen = ImGui::TreeNodeEx((void*)gameObject.get(), flags, objectName.c_str());

    // 트리 노드 상태 저장
    gameObject->SetTreeNodeOpen(nodeOpen);

    // 드래그 소스 설정
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("GAMEOBJECT_DRAG", &gameObject, sizeof(gameObject));
        ImGui::Text("Move %s", objectName.c_str());
        ImGui::EndDragDropSource();
    }

    // 드래그 타겟 설정
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GAMEOBJECT_DRAG"))
        {
            shared_ptr<GameObject>* sourceObj = (shared_ptr<GameObject>*)payload->Data;
            if (*sourceObj != gameObject) // 자기 자신에게 드롭하는 것 방지
            {
                // 현재 월드 위치/회전/스케일 저장
                Vec3 worldPos = (*sourceObj)->transform()->GetWorldPosition();
                Quaternion worldRot = (*sourceObj)->transform()->GetQTRotation();
                Vec3 worldScale = (*sourceObj)->transform()->GetWorldScale();

                // 부모 설정
                (*sourceObj)->SetParent(gameObject);

                // 새로운 부모의 역행렬로 로컬 변환 계산
                Matrix parentWorldMatrix = gameObject->transform()->GetWorldMatrix();
                Matrix parentWorldInverse = parentWorldMatrix.Invert();

                // 월드 위치를 새로운 부모의 로컬 공간으로 변환
                Vec3 newLocalPos = Vec3::Transform(worldPos, parentWorldInverse);
                (*sourceObj)->transform()->SetLocalPosition(newLocalPos);

                // 월드 회전을 새로운 부모의 로컬 공간으로 변환
                Quaternion parentRot = gameObject->transform()->GetQTRotation();
                XMVECTOR parentRotVec = XMLoadFloat4(&parentRot);
                XMVECTOR worldRotVec = XMLoadFloat4(&worldRot);
                XMVECTOR parentRotInvVec = XMQuaternionInverse(parentRotVec);
                XMVECTOR newLocalRotVec = XMQuaternionMultiply(parentRotInvVec, worldRotVec);

                Quaternion newLocalRot;
                XMStoreFloat4(&newLocalRot, newLocalRotVec);
                (*sourceObj)->transform()->SetQTRotation(newLocalRot);

                // 월드 스케일을 새로운 부모의 로컬 공간으로 변환
                Vec3 parentScale = gameObject->transform()->GetWorldScale();
                Vec3 newLocalScale = Vec3(
                    worldScale.x / parentScale.x,
                    worldScale.y / parentScale.y,
                    worldScale.z / parentScale.z
                );
                (*sourceObj)->transform()->SetLocalScale(newLocalScale);

                // 부모 노드를 자동으로 펼침
                gameObject->SetTreeNodeOpen(true);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // 클릭 처리
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
    {
        _selectedObject = gameObject;
        SCENE.GetActiveScene()->AddPickedObject(gameObject);
    }

    // 더블 클릭 처리 (카메라 포커스)
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
    {
        shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
        Vec3 targetPos = gameObject->transform()->GetWorldPosition();
        Vec3 offset = Vec3(0.0f, 2.0f, -5.0f);
        Vec3 cameraTargetPos = targetPos + offset;

        Vec3 dirToTarget = targetPos - cameraTargetPos;
        float length = dirToTarget.Length();
        dirToTarget.Normalize();

        float yaw = atan2(dirToTarget.x, dirToTarget.z);
        float pitch = -atan2(dirToTarget.y, sqrt(dirToTarget.x * dirToTarget.x + dirToTarget.z * dirToTarget.z));

        Vec3 targetRotation = Vec3(
            XMConvertToDegrees(pitch),
            XMConvertToDegrees(yaw),
            0.0f
        );

        _isCameraMoving = true;
        _cameraMoveTime = 0.0f;
        _cameraStartPos = camera->transform()->GetLocalPosition();
        _cameraTargetPos = cameraTargetPos;
        _cameraStartRot = camera->transform()->GetLocalRotation();
        _cameraTargetRot = targetRotation;
    }

    // 우클릭 메뉴
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete"))
        {
            wstring currentSceneName = SCENE.GetActiveScene()->GetSceneName();
            SCENE.RemoveGameObjectFromXML(currentSceneName, gameObject->GetName());
            SCENE.GetActiveScene()->RemoveGameObject(gameObject);
        }
        ImGui::EndPopup();
    }

    // 노드가 열려있으면 자식들도 재귀적으로 렌더링
    if (nodeOpen)
    {
        for (const auto& child : children)
        {
            RenderGameObjectHierarchy(child);
        }
        ImGui::TreePop();
    }
}

void GUIManager::RenderUI_End()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::RenderGuizmo()
{
    shared_ptr<GameObject> pickedObj = SCENE.GetActiveScene()->GetPickedObject();
    if (pickedObj != nullptr)
    {
        _selectedObject = pickedObj;
        shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
        if (camera && camera->GetComponent<Camera>())
        {
            Matrix view = camera->GetComponent<Camera>()->GetViewMatrix();
            Matrix proj = camera->GetComponent<Camera>()->GetProjectionMatrix();
            Matrix world = pickedObj->transform()->GetWorldMatrix();

            float viewportX = GP.GetViewWidth() * (1.0f / 10.0f);
            float viewportY = GP.GetViewHeight() * (3.0f / 100.0f);
            float viewportWidth = GP.GetViewWidth() * (7.0f / 10.0f);
            float viewportHeight = GP.GetViewHeight() * (6.0f / 10.0f);
            
            // Frustum 생성
            BoundingFrustum frustum;
            BoundingFrustum::CreateFromMatrix(frustum, proj);

            // View 공간의 Frustum을 월드 공간으로 변환
            Matrix invView = view.Invert();
            frustum.Transform(frustum, invView);

            ImGuizmo::SetRect(viewportX, viewportY, viewportWidth, viewportHeight);
            ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

            // 콜라이더 기즈모 색상 설정(초록색 계열)
            ImGuizmo::SetGizmoSizeClipSpace(0.08f);  // 기즈모 크기를 좀 더 크게
            ImGuizmo::AllowAxisFlip(false);  // 축 반전 비활성화

            // 기즈모 색상 커스터마이징
            ImGuizmo::Style& style = ImGuizmo::GetStyle();
            style.Colors[ImGuizmo::TRANSLATE_X] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // X축 초록색
            style.Colors[ImGuizmo::TRANSLATE_Y] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Y축 초록색
            style.Colors[ImGuizmo::TRANSLATE_Z] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Z축 초록색
            style.Colors[ImGuizmo::BOUNDS] = ImVec4(0.0f, 0.6f, 0.0f, 0.3f);  // 경계 초록색 (반투명)
                

            // Transform Gizmo 조작 (콜라이더 편집 모드가 아닐 때만)
            if (!_isColliderEditMode)
            {
                // 일반 Transform 기즈모 스타일 복원
                ImGuizmo::SetGizmoSizeClipSpace(0.14f);  // 기본 크기
                ImGuizmo::AllowAxisFlip(true);

                // 기본 색상으로 복원
                ImGuizmo::Style& style = ImGuizmo::GetStyle();
                style.Colors[ImGuizmo::TRANSLATE_X] = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);  // X축 빨간색
                style.Colors[ImGuizmo::TRANSLATE_Y] = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);  // Y축 초록색
                style.Colors[ImGuizmo::TRANSLATE_Z] = ImVec4(0.2f, 0.2f, 0.9f, 1.0f);  // Z축 파란색

                ImGui::PushID("TransformGizmo");

                if (INPUT.GetButtonDown(KEY_TYPE::W)) currentGizmoOperation = ImGuizmo::TRANSLATE;
                if (INPUT.GetButtonDown(KEY_TYPE::E)) currentGizmoOperation = ImGuizmo::ROTATE;
                if (INPUT.GetButtonDown(KEY_TYPE::R)) currentGizmoOperation = ImGuizmo::SCALE;

                Matrix deltaMatrix;
                currentGizmoMode = ImGuizmo::LOCAL;

                ImGuizmo::Manipulate(
                    view.m[0],
                    proj.m[0],
                    currentGizmoOperation,
                    currentGizmoMode,
                    world.m[0],
                    deltaMatrix.m[0]
                );

                if (ImGuizmo::IsUsing())
                {
                    if (currentGizmoOperation == ImGuizmo::ROTATE)
                    {
                        Matrix deltaRotationMatrix = Matrix(deltaMatrix);
                        Quaternion deltaRotation;
                        Vec3 deltaScale, deltaTranslation;
                        deltaRotationMatrix.Decompose(deltaScale, deltaRotation, deltaTranslation);

                        Quaternion currentRotation = pickedObj->transform()->GetQTRotation();
                        Quaternion newRotation = currentRotation * deltaRotation;

                        pickedObj->transform()->SetQTRotation(newRotation);
                        _rotationUpdated = true;
                    }
                    else
                    {
                        /*Vec3 position, rotation, scale;
                        ImGuizmo::DecomposeMatrixToComponents(world.m[0], &position.x, &rotation.x, &scale.x);

                        pickedObj->transform()->SetLocalPosition(position);
                        pickedObj->transform()->SetLocalScale(scale);*/
                        Vec3 position, rotation, scale;
                        ImGuizmo::DecomposeMatrixToComponents(world.m[0], &position.x, &rotation.x, &scale.x);

                        if (pickedObj->transform()->HasParent())
                        {
                            // 부모가 있는 경우 월드 변환을 로컬 변환으로 변환
                            Matrix parentWorldMatrix = pickedObj->transform()->GetParent()->GetWorldMatrix();
                            Matrix parentWorldInverse = parentWorldMatrix.Invert();

                            if (currentGizmoOperation == ImGuizmo::TRANSLATE)
                            {
                                // 이동: 월드 위치를 부모 로컬 공간으로 변환
                                Vec3 localPos = Vec3::Transform(position, parentWorldInverse);
                                pickedObj->transform()->SetLocalPosition(localPos);
                            }
                            else if (currentGizmoOperation == ImGuizmo::SCALE)
                            {
                                // 스케일: 부모의 스케일을 고려하여 로컬 스케일 계산
                                Vec3 parentScale = pickedObj->transform()->GetParent()->GetWorldScale();
                                Vec3 localScale = Vec3(
                                    scale.x / parentScale.x,
                                    scale.y / parentScale.y,
                                    scale.z / parentScale.z
                                );
                                pickedObj->transform()->SetLocalScale(localScale);
                            }
                        }
                        else
                        {
                            // 부모가 없는 경우 직접 적용
                            if (currentGizmoOperation == ImGuizmo::TRANSLATE)
                            {
                                pickedObj->transform()->SetLocalPosition(position);
                            }
                            else if (currentGizmoOperation == ImGuizmo::SCALE)
                            {
                                pickedObj->transform()->SetLocalScale(scale);
                            }
                        }
                    }
                }
                else
                {
                    currentRotationAxis = NONE;
                }

                ImGui::PopID();
            }

            // Collider 시각화 및 Gizmo
            shared_ptr<BaseCollider> collider = pickedObj->GetComponent<BaseCollider>();
            if (collider && _isColliderEditMode)
            {
                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                const ImU32 colliderColor = IM_COL32(0, 255, 0, 255);

                if (auto boxCollider = dynamic_pointer_cast<BoxCollider>(collider))
                {
                    ImGui::PushID("BoxColliderGizmo");
                    BoundingOrientedBox& box = boxCollider->GetBoundingBox();

                    if (frustum.Intersects(box))
                    {
                        // Collider Gizmo (콜라이더 편집 모드일 때만)
                        if (_isColliderEditMode)
                        {
                            // 오브젝트의 월드 Transform 정보 가져오기
                            Vec3 worldPos = pickedObj->transform()->GetWorldPosition();
                            Quaternion worldRot = pickedObj->transform()->GetQTRotation();

                            // 현재 콜라이더의 실제 크기를 가져옴
                            Vec3 currentScale = boxCollider->GetScale();
                            Vec3 objectScale = pickedObj->transform()->GetWorldScale();

                            // Collider Matrix 생성
                            Matrix colliderMatrix = Matrix::CreateScale(currentScale * objectScale);
                            colliderMatrix *= Matrix::CreateFromQuaternion(worldRot);
                            colliderMatrix.Translation(worldPos);

                            Matrix deltaMatrix;
                            ImGuizmo::Manipulate(
                                view.m[0],
                                proj.m[0],
                                ImGuizmo::SCALE,
                                ImGuizmo::LOCAL,
                                colliderMatrix.m[0],
                                deltaMatrix.m[0]
                            );

                            if (ImGuizmo::IsUsing())
                            {
                                Vec3 position, rotation, scale;
                                ImGuizmo::DecomposeMatrixToComponents(colliderMatrix.m[0], &position.x, &rotation.x, &scale.x);
                                // 오브젝트의 월드 스케일을 고려하여 콜라이더 스케일 설정
                                boxCollider->SetScale(scale / objectScale);

                                wstring currentSceneName = SCENE.GetActiveScene()->GetSceneName();
                                SCENE.UpdateGameObjectColliderInXML(currentSceneName, pickedObj->GetName(),
                                    boxCollider->GetCenter(), scale / objectScale, true);
                            }
                        }


                        // 와이어프레임 렌더링
                        Vec3 corners[8];
                        box.GetCorners(corners);
                        Matrix viewProj = view * proj;
                        ImVec2 screenPoints[8];

                        for (int i = 0; i < 8; i++)
                        {
                            Vec4 projected = Vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
                            projected = XMVector4Transform(projected, viewProj);

                            if (projected.w != 0.0f)
                            {
                                projected.x /= projected.w;
                                projected.y /= projected.w;
                            }

                            screenPoints[i] = ImVec2(
                                (projected.x * 0.5f + 0.5f) * viewportWidth + viewportX,
                                (-projected.y * 0.5f + 0.5f) * viewportHeight + viewportY
                            );
                        }

                        // 박스 라인 그리기
                        drawList->AddLine(screenPoints[0], screenPoints[1], colliderColor);
                        drawList->AddLine(screenPoints[1], screenPoints[2], colliderColor);
                        drawList->AddLine(screenPoints[2], screenPoints[3], colliderColor);
                        drawList->AddLine(screenPoints[3], screenPoints[0], colliderColor);

                        drawList->AddLine(screenPoints[4], screenPoints[5], colliderColor);
                        drawList->AddLine(screenPoints[5], screenPoints[6], colliderColor);
                        drawList->AddLine(screenPoints[6], screenPoints[7], colliderColor);
                        drawList->AddLine(screenPoints[7], screenPoints[4], colliderColor);

                        drawList->AddLine(screenPoints[0], screenPoints[4], colliderColor);
                        drawList->AddLine(screenPoints[1], screenPoints[5], colliderColor);
                        drawList->AddLine(screenPoints[2], screenPoints[6], colliderColor);
                        drawList->AddLine(screenPoints[3], screenPoints[7], colliderColor);
                    }
                    ImGui::PopID();
                }
                else if (auto sphereCollider = dynamic_pointer_cast<SphereCollider>(collider))
                {
                    ImGui::PushID("SphereColliderGizmo");
                    BoundingSphere& sphere = sphereCollider->GetBoundingSphere();

                    if (_isColliderEditMode)
                    {
                        Vec3 worldPos = pickedObj->transform()->GetWorldPosition();
                        Quaternion worldRot = pickedObj->transform()->GetQTRotation();

                        // 현재 콜라이더의 실제 크기를 가져옴
                        Vec3 currentScale = sphereCollider->GetScale();
                        Vec3 objectScale = pickedObj->transform()->GetWorldScale();

                        // Collider Matrix 생성
                        Matrix colliderMatrix = Matrix::CreateScale(currentScale * objectScale);
                        colliderMatrix *= Matrix::CreateFromQuaternion(worldRot);
                        colliderMatrix.Translation(worldPos);

                        Matrix deltaMatrix;
                        ImGuizmo::Manipulate(
                            view.m[0],
                            proj.m[0],
                            ImGuizmo::SCALE,
                            ImGuizmo::LOCAL,
                            colliderMatrix.m[0],
                            deltaMatrix.m[0]
                        );

                        if (ImGuizmo::IsUsing())
                        {
                            //Vec3 position, rotation, scale;
                            //ImGuizmo::DecomposeMatrixToComponents(colliderMatrix.m[0], &position.x, &rotation.x, &scale.x);
                            //// 오브젝트의 월드 스케일을 고려하여 콜라이더 스케일 설정
                            //sphereCollider->SetScale(scale / objectScale);
                            Vec3 position, rotation, scale;
                            ImGuizmo::DecomposeMatrixToComponents(colliderMatrix.m[0], &position.x, &rotation.x, &scale.x);

                            // 평균 스케일을 반지름으로 사용
                            float newRadius = (scale.x + scale.y + scale.z) / (3.0f * objectScale.x);
                            sphereCollider->SetRadius(newRadius);
                            sphereCollider->SetScale(Vec3(newRadius, newRadius, newRadius));

                            // XML 저장 추가
                            wstring currentSceneName = SCENE.GetActiveScene()->GetSceneName();
                            SCENE.UpdateGameObjectSphereColliderInXML(currentSceneName, pickedObj->GetName(),
                                sphereCollider->GetCenter(), newRadius);
                        }
                    }
                    

                    ImGui::PopID();

                    // 와이어프레임 렌더링
                    Vec4 center = Vec4(sphere.Center.x, sphere.Center.y, sphere.Center.z, 1.0f);
                    Matrix viewProj = view * proj;
                    center = XMVector4Transform(center, viewProj);

                    if (center.w != 0.0f)
                    {
                        center.x /= center.w;
                        center.y /= center.w;
                    }

                    ImVec2 screenCenter(
                        (center.x * 0.5f + 0.5f) * viewportWidth + viewportX,
                        (-center.y * 0.5f + 0.5f) * viewportHeight + viewportY
                    );

                    float screenRadius = sphere.Radius * viewportHeight / (center.w * 2.0f);
                    drawList->AddCircle(screenCenter, screenRadius, colliderColor);
                }
            }
        }
    }
    else
        _selectedObject = nullptr;
}

void GUIManager::RenderFolderTree(const filesystem::path& path, filesystem::path& selectedFolder)
{
    // 트리 항목의 배경색 설정
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.7f, 0.5f));  // 선택된 항목 색상
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.8f, 0.5f));  // 호버 색상

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Leaf;  // Leaf 플래그 추가하여 화살표 제거

    // 현재 폴더가 선택된 폴더인 경우 Selected 플래그 추가
    if (path == _selectedFolder)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool isDirectory = filesystem::is_directory(path);
    string folderName = path.filename().string();
    if (folderName.empty()) // root 폴더인 경우
    {
        folderName = "Resource";
        flags &= ~ImGuiTreeNodeFlags_Leaf;  // Resource 폴더만 화살표 표시
        flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    }

    bool opened = ImGui::TreeNodeEx(folderName.c_str(), flags);

    // 클릭 처리
    if (ImGui::IsItemClicked())
        _selectedFolder = path;

    if (opened && isDirectory)
    {
        for (const auto& entry : filesystem::directory_iterator(path))
        {
            if (filesystem::is_directory(entry))
            {
                RenderFolderTree(entry.path(), selectedFolder);
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopStyleColor(2);
}

void GUIManager::RenderFileGrid(const filesystem::path& path)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
    if (_isFirstFrame)
    {
        for (const auto& entry : filesystem::directory_iterator("Resource/"))
        {
            if (filesystem::is_directory(entry))
            {
                _selectedFolder = entry.path();
                break;
            }
        }
        _isFirstFrame = false;
    }

    float cellSize = 120.0f;  // 셀 크기를 더 늘림
    float iconSize = 50.0f;
    float padding = 10.0f;
    float maxTextWidth = cellSize - padding * 2;  // 최대 텍스트 너비

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = static_cast<int>(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));

    for (const auto& entry : filesystem::directory_iterator(path))
    {
        if (!filesystem::is_directory(entry))
        {
            string filename = entry.path().filename().string();
            string filenameWithoutExt = filename.substr(0, filename.find_last_of("."));
            string parentFolder = entry.path().parent_path().filename().string();
            shared_ptr<Texture> icon = nullptr;

            // 폴더별로 다른 아이콘 적용
            if (parentFolder == "Texture")
            {
                string nameWithoutExt = filename.substr(0, filename.find_last_of("."));
                wstring textureName(nameWithoutExt.begin(), nameWithoutExt.end());
                icon = RESOURCE.GetResource<Texture>(textureName);
            }
            else if (parentFolder == "Material")
            {
                icon = RESOURCE.GetResource<Texture>(L"Material_Icon");
            }
            else if (parentFolder == "Mesh")
            {
                icon = RESOURCE.GetResource<Texture>(L"Mesh_Icon");
            }
            else if (parentFolder == "Model")
            {
                icon = RESOURCE.GetResource<Texture>(L"Model_Icon");
            }
            else if (parentFolder == "Shader")
            {
                icon = RESOURCE.GetResource<Texture>(L"Shader_Icon");
            }

            if (icon)
            {
                // 아이콘 중앙 배치
                float cursorPosX = ImGui::GetCursorPosX();
                float iconPosX = cursorPosX + (cellSize - iconSize) * 0.5f;
                ImGui::SetCursorPosX(iconPosX);

                ImGui::ImageButton(filename.c_str(),
                    (ImTextureID)icon->GetShaderResourceView().Get(),
                    ImVec2(iconSize, iconSize));

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    string fullPath = entry.path().string();
                    ImGui::SetDragDropPayload("RESOURCE_FILE", fullPath.c_str(), fullPath.size() + 1);
                    ImGui::Text("%s", filenameWithoutExt.c_str());
                    ImGui::EndDragDropSource();
                }

                // 텍스트 처리 - 한 줄로 표시
                string displayText = filenameWithoutExt;
                float textWidth = ImGui::CalcTextSize(displayText.c_str()).x;

                // 텍스트가 너무 길 경우 줄임
                if (textWidth > maxTextWidth)
                {
                    // 텍스트와 "..." 길이를 고려하여 적절한 길이로 자름
                    int maxChars = 0;
                    string tempText;
                    for (int i = 0; i < displayText.length(); i++)
                    {
                        tempText = displayText.substr(0, i) + "...";
                        if (ImGui::CalcTextSize(tempText.c_str()).x > maxTextWidth)
                        {
                            maxChars = i - 1;
                            break;
                        }
                    }
                    displayText = displayText.substr(0, maxChars) + "...";
                    textWidth = ImGui::CalcTextSize(displayText.c_str()).x;
                }

                // 텍스트 중앙 정렬하여 한 줄로 표시
                float textPosX = cursorPosX + (cellSize - textWidth) * 0.5f;
                ImGui::SetCursorPosX(textPosX);
                ImGui::Text("%s", displayText.c_str());  // TextWrapped 대신 Text 사용

                // 항목 간 여백
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding);
            }

            ImGui::NextColumn();
        }
    }

    ImGui::PopStyleColor(3);
    ImGui::Columns(1);

    ImGui::PopStyleVar();
}