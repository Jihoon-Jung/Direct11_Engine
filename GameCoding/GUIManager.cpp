#include "pch.h"
#include <string>
#include "GUIManager.h"
#include "MoveObject.h"
#include "TestEvent.h"
#include <algorithm>
#include <fstream>

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

        shared_ptr<GameObject> camera = SCENE.GetActiveScene()->GetMainCamera();
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
    // 전역 빈 공간 클릭 처리
    if (ImGui::GetIO().MouseClicked[ImGuiMouseButton_Left])
    {
        bool clickedWindow = false;
        bool isAnyPopupOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);  // 열린 팝업 확인

        // 현재 마우스가 어떤 윈도우 위에 있는지 확인
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
            ImGui::IsAnyItemHovered() ||
            isAnyPopupOpen)  // 수정: 팝업창이 열려있으면 클릭된 것으로 처리
        {
            clickedWindow = true;
        }

        // 뷰포트 영역을 클릭했을 때 Inspector 창 초기화
        if (IsViewportHovered() && !isAnyPopupOpen)
        {
            _selectedShaderFile.clear();
            _selectedScriptFile.clear();
            _selectedMaterialFile.clear();
            _selectedTextureFile.clear();
            _selectedFileType = FileType::NONE;
        }
        // 빈 공간 클릭 시 초기화
        else if (!clickedWindow && !isAnyPopupOpen)
        {
            _selectedShaderFile.clear();
            _selectedScriptFile.clear();
            _selectedMaterialFile.clear();
            _selectedTextureFile.clear();
            _selectedFileType = FileType::NONE;
            _selectedObject = nullptr;
        }
    }

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

    ShowAnimatorEditor();

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

        // 선택된 셰이더 파일이 있는 경우
        if (_selectedFileType == FileType::SHADER && !_selectedShaderFile.empty())
        {
            _selectedObject = nullptr;  // 오브젝트 선택 해제
            ShowShaderInspector(_selectedShaderFile);
        }
        // 선택된 스크립트 파일이 있는 경우
        else if (_selectedFileType == FileType::Script && !_selectedScriptFile.empty())
        {
            _selectedObject = nullptr;  // 오브젝트 선택 해제
            ShowScriptInspector(_selectedScriptFile);
        }
        
        if (_selectedFileType == FileType::MATERIAL && !_selectedMaterialFile.empty())
        {
            _selectedObject = nullptr;  // 오브젝트 선택 해제
            ShowMaterialInspector(_selectedMaterialFile);
        }
        if (_selectedFileType == FileType::TEXTURE && !_selectedTextureFile.empty())
        {
            _selectedObject = nullptr;  // 오브젝트 선택 해제
            ShowTextureInspector(_selectedTextureFile);
        }
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
                {
                    transform->SetLocalPosition(position);
                    _isTransformChanged = true;
                }
                    

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
                    _isTransformChanged = true;
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
                    _isTransformChanged = true;
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
                    _isTransformChanged = true;
                }

                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                {
                    transform->SetLocalScale(scale);
                    _isTransformChanged = true;
                }
                
                if (_isTransformChanged && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    SCENE.UpdateGameObjectTransformInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedObject->GetName(),
                        position, transform->GetLocalRotation(), transform->GetLocalScale());
                    _isTransformChanged = false;
                }
                

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

                // MonoBehaviour 스크립트 컴포넌트 처리
                if (auto script = dynamic_pointer_cast<MonoBehaviour>(component))
                {
                    // ComponentFactory에서 등록된 스크립트 정보 찾기
                    const auto& scripts = CF.GetRegisteredScripts();
                    string currentTypeName = typeid(*script).name();

                    // "class " 접두사 제거
                    if (currentTypeName.substr(0, 6) == "class ")
                    {
                        currentTypeName = currentTypeName.substr(6);
                    }

                    auto it = scripts.find(currentTypeName);
                    if (it != scripts.end())
                    {
                        componentName = it->second.displayName;
                    }
                }

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
                /*else if (dynamic_pointer_cast<MoveObject>(component))
                    componentName = "MoveObject";*/
                else if (dynamic_pointer_cast<Animator>(component))
                    componentName = "Animator";

                if (!componentName.empty())
                {
                    if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        // 컴포넌트 제거 메뉴
                        if (ImGui::BeginPopupContextItem())
                        {
                            if (ImGui::MenuItem("Delete"))
                            {
                                // XML 업데이트
                                SCENE.RemoveComponentFromGameObjectInXML(
                                    SCENE.GetActiveScene()->GetSceneName(),
                                    _selectedObject->GetName(),
                                    component
                                );

                                // 오브젝트에서 컴포넌트 제거
                                _selectedObject->RemoveComponent(component);

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
                                SCENE.UpdateGameObjectColliderInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedObject->GetName(),
                                    center, boxCollider->GetScale(), true);
                            }

                            // Size
                            Vec3 scale = boxCollider->GetScale();
                            if (ImGui::DragFloat3("Size", &scale.x, 0.1f))
                            {
                                boxCollider->SetScale(scale);
                                SCENE.UpdateGameObjectColliderInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedObject->GetName(),
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
                        else if (auto animator = dynamic_pointer_cast<Animator>(component))
                        {
                            if (ImGui::Button("Edit##Animator", ImVec2(60, 25)))
                            {
                                _showAnimatorEditor = true;
                                _selectedAnimator = animator;

                                // 노드 데이터 초기화
                                _nodes.clear();

                                // Entry 노드 추가
                                NodeData entryNode;
                                entryNode.name = "Entry";
                                entryNode.pos = ImVec2(50, 100);  // Entry 노드의 시작 위치 수정
                                entryNode.isEntry = true;
                                _nodes.push_back(entryNode);

                                // Clip 노드들 추가
                                float xOffset = 250;  // Entry 노드와의 간격
                                float yPos = 100;     // 시작 y 위치
                                float xSpacing = 200; // 노드 간 가로 간격

                                for (const auto& clipPair : animator->_clips)
                                {
                                    NodeData node;
                                    node.name = clipPair.first;
                                    node.pos = ImVec2(xOffset, yPos);
                                    node.clip = clipPair.second;
                                    _nodes.push_back(node);
                                    xOffset += xSpacing;
                                }
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
                                                    RENDER.GetRenderableObject();
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
                                                                    RENDER.GetRenderableObject();
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

            // 드래그 앤 드롭 영역 생성
            ImGui::InvisibleButton("##dropzone", ImGui::GetContentRegionAvail());

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE"))
                {
                    const char* fullPath = (const char*)payload->Data;
                    filesystem::path path(fullPath);

                    // Script 폴더의 XML 파일인지 확인
                    if (path.parent_path().filename() == "Script" && path.extension() == ".xml")
                    {
                        string className = path.stem().string();  // XML 파일명에서 확장자를 제외한 클래스 이름

                        // ComponentFactory에서 스크립트 정보 찾기
                        const auto& scripts = CF.GetRegisteredScripts();
                        for (const auto& [typeName, info] : scripts)
                        {
                            if (info.displayName == className)
                            {
                                // 이미 추가된 스크립트인지 확인
                                bool hasScript = false;
                                for (const auto& component : _selectedObject->GetComponents())
                                {
                                    if (auto script = dynamic_pointer_cast<MonoBehaviour>(component))
                                    {
                                        if (typeid(*script).name() == typeName)
                                        {
                                            hasScript = true;
                                            break;
                                        }
                                    }
                                }

                                // 스크립트가 아직 추가되지 않았다면 추가
                                if (!hasScript)
                                {
                                    auto script = info.createFunc();
                                    _selectedObject->AddComponent(script);

                                    // XML 업데이트
                                    SCENE.AddComponentToGameObjectAndSaveToXML(
                                        SCENE.GetActiveScene()->GetSceneName(),
                                        _selectedObject->GetName(),
                                        script
                                    );
                                }
                                break;
                            }
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // 컴포넌트 추가 메뉴
            if (ImGui::BeginPopupContextWindow("AddComponentMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::BeginMenu("Add Component"))
                {
                    // Scripts 서브메뉴
                    if (ImGui::BeginMenu("Scripts"))
                    {
                        const auto& scripts = CF.GetRegisteredScripts();
                        for (const auto& [typeName, info] : scripts)
                        {
                            // 이미 추가된 스크립트인지 확인
                            bool hasScript = false;
                            for (const auto& component : _selectedObject->GetComponents())
                            {
                                if (auto script = dynamic_pointer_cast<MonoBehaviour>(component))
                                {
                                    if (typeid(*script).name() == typeName)
                                    {
                                        hasScript = true;
                                        break;
                                    }
                                }
                            }

                            if (!hasScript && ImGui::MenuItem(info.displayName.c_str()))
                            {
                                auto script = info.createFunc();
                                _selectedObject->AddComponent(script);

                                // XML 업데이트
                                SCENE.AddComponentToGameObjectAndSaveToXML(
                                    SCENE.GetActiveScene()->GetSceneName(),
                                    _selectedObject->GetName(),
                                    script
                                );
                                ImGui::CloseCurrentPopup();
                            }
                        }
                        ImGui::EndMenu();
                    }

                    // 현재 오브젝트의 컴포넌트 목록 확인
                    const vector<shared_ptr<Component>>& components = _selectedObject->GetComponents();
                    bool hasBoxCollider = false;
                    bool hasSphereCollider = false;
                    bool hasMeshRenderer = false;
                    bool hasAnimator = false;

                    // 기존 컴포넌트 체크
                    for (const auto& component : components)
                    {
                        if (dynamic_pointer_cast<BoxCollider>(component))
                            hasBoxCollider = true;
                        else if (dynamic_pointer_cast<SphereCollider>(component))
                            hasSphereCollider = true;
                        else if (dynamic_pointer_cast<MeshRenderer>(component))
                            hasMeshRenderer = true;
                        else if (dynamic_pointer_cast<Animator>(component))
                            hasAnimator = true;
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
                    if (!hasAnimator && ImGui::MenuItem("Animator"))
                    {
                        auto meshRenderer = _selectedObject->GetComponent<MeshRenderer>();
                        if (!meshRenderer || meshRenderer->GetModel()->GetAnimations().empty())
                        {
                            MessageBoxA(nullptr, "No animations in this model!", "Error", MB_ICONERROR);
                            abort();
                        }
                        else
                        {
                            auto animator = make_shared<Animator>();
                            vector<shared_ptr<ModelAnimation>> animations = meshRenderer->GetModel()->GetAnimations();
                            int animIndex = 0;
                            for (shared_ptr<ModelAnimation> animation : animations)
                            {
                                string name = Utils::ToString(animation->clipName);
                                animator->AddClip(name, animIndex, false);

                                if (animIndex == 0)
                                    animator->SetEntryClip(name);

                                animIndex++;
                            }

                            _selectedObject->AddComponent(animator);
                            SCENE.AddComponentToGameObjectAndSaveToXML(
                                SCENE.GetActiveScene()->GetSceneName(),
                                _selectedObject->GetName(),
                                animator);
                        }
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

            shared_ptr<Animator> animator = _selectedObject->GetComponent<Animator>();

            if (animator != nullptr)
            {
                shared_ptr<MeshRenderer> meshRenderer = _selectedObject->GetComponent<MeshRenderer>();
                vector<shared_ptr<ModelAnimation>> animations = meshRenderer->GetModel()->GetAnimations();
                for (shared_ptr<ModelAnimation> animation : animations)
                {
                    wstring name = animation->name;
                    int a = 1;
                }

            }

        }
        
        ImGui::End();
    }

    // project
    {

        ImGui::SetNextWindowPos(ImVec2(0, GP.GetViewHeight()* (63.0f / 100.0f)), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GP.GetViewWidth(), GP.GetViewHeight()* (37.0f / 100.0f)));
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


    if (!_showAnimatorEditor)
    {
        // 뷰포트 영역 계산
        float viewportX = GP.GetViewWidth() * (1.0f / 10.0f);
        float viewportY = GP.GetViewHeight() * (3.0f / 100.0f);
        float viewportWidth = GP.GetViewWidth() * (7.0f / 10.0f);
        float viewportHeight = GP.GetViewHeight() * (6.0f / 10.0f);

        // 드래그 중일 때만 드롭 영역 활성화
        bool isDragging = ImGui::GetDragDropPayload() != nullptr;

        if (isDragging)
        {
            ImGui::SetNextWindowPos(ImVec2(viewportX, viewportY));
            ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));

            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav;

            ImGui::SetNextWindowBgAlpha(0.0f);

            if (ImGui::Begin("##ViewportDropTarget", nullptr, window_flags))
            {
                ImGui::SetCursorPos(ImVec2(0, 0));
                ImGui::InvisibleButton("##dropzone", ImGui::GetContentRegionAvail(), ImGuiButtonFlags_MouseButtonLeft);

                // 마우스 위치를 직접 체크하여 뷰포트 영역 안에 있는지 확인
                ImVec2 mousePos = ImGui::GetMousePos();
                if (mousePos.x >= viewportX && mousePos.x <= (viewportX + viewportWidth) &&
                    mousePos.y >= viewportY && mousePos.y <= (viewportY + viewportHeight))
                {
                    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                    if (payload && payload->IsDataType("RESOURCE_FILE"))  // 타입 체크는 IsDataType 사용
                    {
                        const ImGuiPayload* payload = ImGui::GetDragDropPayload();
                        if (payload && payload->IsDataType("RESOURCE_FILE"))
                        {
                            const char* fullPath = (const char*)payload->Data;
                            OnResourceDroppedToViewport(fullPath);
                        }
                    }
                }

                // 실제 드롭 처리
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_FILE"))
                    {
                        _droppedObject = nullptr;
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            ImGui::End();
        }

        RenderGuizmo();
    }
        
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
        // Project 창의 선택 상태 초기화
        _selectedShaderFile.clear();
        _selectedScriptFile.clear();
        _selectedMaterialFile.clear();
        _selectedTextureFile.clear();
        _selectedFileType = FileType::NONE;

        _selectedObject = gameObject;
        SCENE.GetActiveScene()->AddPickedObject(gameObject);

        // Animator 업데이트
        _selectedAnimator = gameObject->GetComponent<Animator>();
        if (_selectedAnimator)
        {
            // 노드 데이터 초기화
            _nodes.clear();

            // Entry 노드 추가
            NodeData entryNode;
            entryNode.name = "Entry";
            entryNode.isEntry = true;
            entryNode.pos = ImVec2(50, 50);  // 적절한 초기 위치 설정
            _nodes.push_back(entryNode);

            // 애니메이션 클립 노드들 추가
            for (const auto& clipPair : _selectedAnimator->_clips)  // pair<string, shared_ptr<Clip>>
            {
                NodeData clipNode;
                clipNode.name = clipPair.second->name;
                clipNode.clip = clipPair.second;
                clipNode.isEntry = false;
                clipNode.pos = clipPair.second->pos;  // 저장된 위치 사용
                _nodes.push_back(clipNode);
            }
        }
        else
        {
            // Animator가 없는 경우 노드 데이터 초기화
            _nodes.clear();
            _selectedTransition = nullptr;
        }
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
                    _isTransformChanged = true;

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

    // 드래그 드롭 영역 설정
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXTERNAL_FILE"))
        {
            wchar_t* path = (wchar_t*)payload->Data;
            HandleExternalFilesDrop(path);
        }
        ImGui::EndDragDropTarget();
    }

    static bool clickedItem = false;
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        clickedItem = false;
    }

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
            // Script 폴더에서는 XML 파일만 처리하고 각각 cpp와 h 파일 아이콘으로 표시
            if (parentFolder == "Script")
            {
                if (entry.path().extension() == ".xml")
                {
                    // Header 파일 아이콘
                    icon = RESOURCE.GetResource<Texture>(L"H_Icon");
                    RenderScriptIcon(icon, filenameWithoutExt + ".h", entry.path(), cellSize, iconSize, padding, maxTextWidth);
                    ImGui::NextColumn();

                    // CPP 파일 아이콘
                    icon = RESOURCE.GetResource<Texture>(L"CPP_Icon");
                    RenderScriptIcon(icon, filenameWithoutExt + ".cpp", entry.path(), cellSize, iconSize, padding, maxTextWidth);
                    ImGui::NextColumn();
                }
                continue;
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

                // 파일 클릭 처리
                if (ImGui::IsItemClicked())
                {
                    clickedItem = true;

                    _selectedObject = nullptr;
                    SCENE.GetActiveScene()->AddPickedObject(nullptr);

                    if (parentFolder == "Shader")
                    {
                        _selectedShaderFile = entry.path();
                        _selectedFileType = FileType::SHADER;

                        // 셰이더 코드 읽기
                        ifstream file(entry.path());
                        if (file.is_open())
                        {
                            stringstream buffer;
                            buffer << file.rdbuf();
                            _shaderCode = buffer.str();
                        }
                    }
                    else if (parentFolder == "Script")
                    {
                        // Hierarchy 창의 선택 상태 초기화
                        _selectedObject = nullptr;
                        SCENE.GetActiveScene()->AddPickedObject(nullptr);

                        _selectedScriptFile = entry.path();
                        _selectedFileType = FileType::Script;
                    }
                    // Material 파일 클릭 처리 추가
                    else if (parentFolder == "Material")
                    {
                        _selectedMaterialFile = entry.path();
                        _selectedFileType = FileType::MATERIAL;
                    }
                    else if (parentFolder == "Texture")  // 텍스처 파일 선택 처리 추가
                    {
                        _selectedObject = nullptr;
                        _selectedTextureFile = entry.path();
                        _selectedFileType = FileType::TEXTURE;
                    }
                }

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
    // 모든 아이템 렌더링 후 빈 공간 클릭 처리
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !clickedItem)
    {
        _selectedShaderFile.clear();
        _selectedScriptFile.clear();
        _selectedMaterialFile.clear();
        _selectedTextureFile.clear();
        _selectedFileType = FileType::NONE;
    }
    

    ImGui::PopStyleColor(3);
    ImGui::Columns(1);

    ImGui::PopStyleVar();
}


void GUIManager::ShowAnimatorEditor()
{
    if (!_showAnimatorEditor)
        return;
        

    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Animator", &_showAnimatorEditor))
    {
        if (_selectedAnimator)
        {
            // 좌측 Parameters 패널
            ImGui::BeginChild("Parameters", ImVec2(200, 0), true);
            RenderParametersPanel();
            ImGui::EndChild();

            ImGui::SameLine();

            // 중앙 Graph 패널
            ImGui::BeginChild("Graph", ImVec2(-200, 0), true);
            RenderGraphPanel();
            ImGui::EndChild();

            ImGui::SameLine();

            // 우측 Inspector 패널
            ImGui::BeginChild("Inspector", ImVec2(200, 0), true);
            RenderInspectorPanel();  // 여기서 호출되는지 확인
            ImGui::EndChild();
        }
    }
    ImGui::End();

    if (!_showAnimatorEditor)
    {
        ClearAnimatorEditorData();
    }
}

void GUIManager::RenderParametersPanel()
{
    if (!_selectedAnimator)
        return;

    ImGui::Text("Parameters");
    ImGui::Separator();

    if (ImGui::Button("+"))
        ImGui::OpenPopup("AddParameter");

    if (ImGui::BeginPopup("AddParameter"))
    {
        // 먼저 타입 선택 버튼들을 표시
        if (ImGui::Button("Bool"))
        {
            _showAddParameterPopup = true;
            _selectedParameterType = ParameterType::Bool;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Int"))
        {
            _showAddParameterPopup = true;
            _selectedParameterType = ParameterType::Int;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Float"))
        {
            _showAddParameterPopup = true;
            _selectedParameterType = ParameterType::Float;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // 타입 선택 후 이름 입력 팝업
    if (_showAddParameterPopup)
    {
        ImGui::OpenPopup("ParameterName");
        // 팝업 중앙 정렬
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("ParameterName", &_showAddParameterPopup))
    {
        static char paramName[128] = "";
        ImGui::Text("Enter Parameter Name:");
        ImGui::InputText("##Name", paramName, sizeof(paramName));

        if (ImGui::Button("OK") || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            if (strlen(paramName) > 0)
            {
                // 파라미터 추가
                _selectedAnimator->AddParameter(paramName, static_cast<Parameter::Type>(_selectedParameterType));

                // XML에도 저장
                SCENE.AddAnimatorParameterToXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    paramName,
                    static_cast<Parameter::Type>(_selectedParameterType));

                _showAddParameterPopup = false;
                ImGui::CloseCurrentPopup();
                memset(paramName, 0, sizeof(paramName));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            _showAddParameterPopup = false;
            ImGui::CloseCurrentPopup();
            memset(paramName, 0, sizeof(paramName));
        }
        ImGui::EndPopup();
    }

    // 파라미터 목록 표시
    auto& parameters = _selectedAnimator->_parameters;
    for (size_t i = 0; i < parameters.size(); i++)
    {
        auto& param = parameters[i];

        // 파라미터 이름과 삭제 버튼을 한 줄에 표시하기 위한 그룹
        ImGui::PushID(i);
        ImGui::BeginGroup();

        bool valueChanged = false;
        switch (param.type)
        {
        case Parameter::Type::Bool:
        {
            bool value = param.value.boolValue;
            if (ImGui::Checkbox(param.name.c_str(), &value))
            {
                _selectedAnimator->SetBool(param.name, value);
                valueChanged = true;
            }
        }
        break;
        case Parameter::Type::Int:
        {
            int value = param.value.intValue;
            if (ImGui::DragInt(param.name.c_str(), &value))
            {
                _selectedAnimator->SetInt(param.name, value);
                valueChanged = true;
            }
        }
        break;
        case Parameter::Type::Float:
        {
            float value = param.value.floatValue;
            if (ImGui::DragFloat(param.name.c_str(), &value, 0.1f))
            {
                _selectedAnimator->SetFloat(param.name, value);
                valueChanged = true;
            }
        }
        break;
        }

        // 삭제 버튼 추가
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Button("X##"))
        {
            // XML에서 먼저 삭제
            if (auto gameObj = _selectedObject)
            {
                SCENE.RemoveAnimatorParameterFromXML(
                    SCENE.GetActiveScene()->GetSceneName(),
                    gameObj->GetName(),
                    param.name
                );
            }

            // Animator에서 삭제
            _selectedAnimator->RemoveParameter(param.name);

            ImGui::PopStyleColor(3);
            ImGui::EndGroup();
            ImGui::PopID();
            break;  // 파라미터가 삭제되었으므로 루프 종료
        }

        ImGui::PopStyleColor(3);
        ImGui::EndGroup();
        ImGui::PopID();

        if (valueChanged)
        {
            SCENE.UpdateAnimatorParameterInXML(SCENE.GetActiveScene()->GetSceneName(),
                _selectedAnimator->GetGameObject()->GetName(),
                param.name, param);
        }
    }
}

void GUIManager::RenderGraphPanel()
{
    // 매 프레임 시작시 호버 상태 초기화
    _isAnyArrowHovered = false;

    // 회색 배경으로 설정
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
    ImGui::BeginChild("canvas", ImVec2(0, 0), true);

    // 캔버스의 시작 위치를 저장
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    // 먼저 트랜지션(화살표)을 그림
    RenderTransitions();

    // 노드들을 그릴 때 저장된 캔버스 위치를 사용
    for (auto& node : _nodes)
    {
        ImGui::SetCursorScreenPos(canvasPos);
        RenderNode(node);
    }

    // 빈 공간 클릭 감지
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        ImVec2 mousePos = ImGui::GetMousePos();
        bool clickedEmpty = true;

        // 노드 클릭 체크
        for (auto& node : _nodes)
        {
            ImVec2 nodePos = ImGui::GetCursorScreenPos();
            nodePos.x += node.pos.x;
            nodePos.y += node.pos.y;

            if (ImGui::IsMouseHoveringRect(
                nodePos,
                ImVec2(nodePos.x + NODE_WIDTH, nodePos.y + NODE_HEIGHT)))
            {
                clickedEmpty = false;
                break;
            }
        }
        // 트랜지션 클릭 체크
        for (const auto& transition : _selectedAnimator->_transitions)
        {
            auto clipA = transition->clipA.lock();
            auto clipB = transition->clipB.lock();
            if (!clipA || !clipB)
                continue;

            // 시작점과 끝점 노드 찾기
            NodeData* startNode = nullptr;
            NodeData* endNode = nullptr;
            for (auto& node : _nodes)
            {
                if (node.clip == clipA) startNode = &node;
                if (node.clip == clipB) endNode = &node;
            }

            if (startNode && endNode)
            {
                ImVec2 start(
                    canvasPos.x + startNode->pos.x + (NODE_WIDTH * 0.5f),
                    canvasPos.y + startNode->pos.y + (NODE_HEIGHT * 0.5f)
                );
                ImVec2 end(
                    canvasPos.x + endNode->pos.x + (NODE_WIDTH * 0.5f),
                    canvasPos.y + endNode->pos.y + (NODE_HEIGHT * 0.5f)
                );

                if (DistancePointToLineSegment(mousePos, start, end) < 5.0f)
                {
                    clickedEmpty = false;
                    break;
                }
            }
        }

        if (_isAnyArrowHovered)
        {
            clickedEmpty = false;
        }
        // 빈 공간을 클릭했을 때만 선택 해제
        if (clickedEmpty)
        {
            _selectedNode = nullptr;
            _selectedTransition = nullptr;
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

ImVec4 HSVtoRGB(float h, float s, float v)
{
    float r, g, b;

    int hi = static_cast<int>(h * 6.0f) % 6;
    float f = h * 6.0f - static_cast<float>(hi);
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (hi)
    {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
    }

    return ImVec4(r, g, b, 1.0f);

}
void GUIManager::RenderInspectorPanel()
{
    if (!_selectedAnimator)
    {
        return;
    }

    // 선택된 트랜지션이 있는 경우
    if (_selectedTransition)
    {
        ImGui::Text("Transition Settings");
        ImGui::Separator();

        auto clipA = _selectedTransition->clipA.lock();
        auto clipB = _selectedTransition->clipB.lock();

        // Has Exit Time 설정
        bool hasExitTime = _selectedTransition->hasExitTime;
        if (ImGui::Checkbox("Has Exit Time", &hasExitTime))
        {
            _selectedAnimator->SetTransitionHasExit(_selectedTransition, hasExitTime);
            if (clipA && clipB)
            {
                SCENE.UpdateAnimatorTransitionInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedAnimator->GetGameObject()->GetName(),
                    clipA->name, clipB->name, _selectedTransition->transitionDuration,
                    _selectedTransition->transitionOffset, _selectedTransition->exitTime, hasExitTime);
            }
        }

        // Exit Time 설정
        float exitTime = _selectedTransition->exitTime;
        if (ImGui::DragFloat("ExitTime", &exitTime, 0.1f, 0.0f, 5.0f))
        {
            _selectedAnimator->SetTransitionExitTime(_selectedTransition, exitTime);
            if (clipA && clipB)
            {
                SCENE.UpdateAnimatorTransitionInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedAnimator->GetGameObject()->GetName(),
                    clipA->name, clipB->name, _selectedTransition->transitionDuration,
                    _selectedTransition->transitionOffset, exitTime,  hasExitTime);
            }
        }

        // Duration 설정
        float duration = _selectedTransition->transitionDuration;
        if (ImGui::DragFloat("Duration", &duration, 0.1f, 0.0f, 5.0f))
        {
            _selectedAnimator->SetTransitionDuration(_selectedTransition, duration);
            
            if (clipA && clipB)
            {
                SCENE.UpdateAnimatorTransitionInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedAnimator->GetGameObject()->GetName(),
                    clipA->name, clipB->name, duration, _selectedTransition->transitionOffset,
                    _selectedTransition->exitTime, _selectedTransition->hasExitTime);
            }
        }

        // Offset 설정
        float offset = _selectedTransition->transitionOffset;
        if (ImGui::DragFloat("Offset", &offset, 0.1f, 0.0f, 1.0f))
        {
            _selectedAnimator->SetTransitionOffset(_selectedTransition, offset);
            if (clipA && clipB)
            {
                SCENE.UpdateAnimatorTransitionInXML(SCENE.GetActiveScene()->GetSceneName(), _selectedAnimator->GetGameObject()->GetName(),
                    clipA->name, clipB->name, _selectedTransition->transitionDuration,
                    offset, _selectedTransition->exitTime, _selectedTransition->hasExitTime);
            }
        }

        // Conditions 섹션
        ImGui::Spacing();
        ImGui::Text("Conditions");
        ImGui::Separator();

        // 조건 목록 표시
        for (size_t i = 0; i < _selectedTransition->conditions.size(); i++)
        {
            auto& condition = _selectedTransition->conditions[i];
            ImGui::PushID(static_cast<int>(i));

            bool conditionChanged = false;

            // Parameter 선택 콤보박스 (첫 번째 줄)
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 20.0f); // X 버튼 공간 확보
            if (ImGui::BeginCombo("##param", condition.parameterName.c_str()))
            {
                for (const auto& param : _selectedAnimator->_parameters)
                {
                    bool isSelected = (condition.parameterName == param.name);
                    if (ImGui::Selectable(param.name.c_str(), isSelected))
                    {
                        condition.parameterName = param.name;
                        condition.parameterType = param.type;
                        conditionChanged = true;
                    }
                }
                ImGui::EndCombo();
            }

            // 삭제 버튼을 오른쪽에 배치
            ImGui::SameLine();
            if (ImGui::Button("X##remove"))
            {
                _selectedAnimator->RemoveCondition(_selectedTransition, i);

                // Condition 삭제 후 XML 업데이트 추가
                SCENE.UpdateAnimatorTransitionConditionInXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _selectedTransition->clipA.lock()->name,
                    _selectedTransition->clipB.lock()->name,
                    _selectedTransition->conditions);

                ImGui::PopID();
                break;
            }

            // 값 설정 UI (두 번째 줄)
            switch (condition.parameterType)
            {
            case Parameter::Type::Bool:
            {
                int value = condition.value.boolValue ? 1 : 0;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::Combo("##boolValue", &value, "false\0true\0"))
                {
                    condition.value.boolValue = (value == 1);
                    conditionChanged = true;
                }
            }
            break;

            case Parameter::Type::Int:
            case Parameter::Type::Float:
            {
                // 비교 연산자 콤보박스
                const char* compareTypes[] = { "==", "!=", ">", "<" };
                int currentType = static_cast<int>(condition.compareType);
                float totalWidth = ImGui::GetContentRegionAvail().x;
                float compareWidth = 60.0f;
                float valueWidth = totalWidth - compareWidth;

                ImGui::SetNextItemWidth(compareWidth);
                if (ImGui::Combo("##compare", &currentType, compareTypes, 4))
                {
                    condition.compareType = static_cast<Condition::CompareType>(currentType);
                    conditionChanged = true;
                }

                ImGui::SameLine();

                // 값 입력
                ImGui::SetNextItemWidth(valueWidth);
                if (condition.parameterType == Parameter::Type::Int)
                {
                    int value = condition.value.intValue;
                    if (ImGui::DragInt("##value", &value))
                    {
                        condition.value.intValue = value;
                        conditionChanged = true;
                    }
                }
                else
                {
                    float value = condition.value.floatValue;
                    if (ImGui::DragFloat("##value", &value, 0.1f))
                    {
                        condition.value.floatValue = value;
                        conditionChanged = true;
                    }
                }
            }
            break;
            }

            if (conditionChanged)
            {
                _selectedAnimator->CheckConditionsAndSetFlag();

                SCENE.UpdateAnimatorTransitionConditionInXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _selectedTransition->clipA.lock()->name,
                    _selectedTransition->clipB.lock()->name,
                    _selectedTransition->conditions);
            }

            ImGui::PopID();
            ImGui::Spacing();
        }

        // Add Condition 버튼
        if (ImGui::Button("Add Condition", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
        {
            if (!_selectedAnimator->_parameters.empty())
            {
                const auto& firstParam = _selectedAnimator->_parameters[0];
                _selectedAnimator->AddCondition(_selectedTransition,
                    firstParam.name, firstParam.type, Condition::CompareType::Equals);

                _selectedAnimator->CheckConditionsAndSetFlag();

                SCENE.UpdateAnimatorTransitionConditionInXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _selectedTransition->clipA.lock()->name,
                    _selectedTransition->clipB.lock()->name,
                    _selectedTransition->conditions);
            }
        }
    }
    // 선택된 노드가 있고 Entry가 아닌 경우
    else if (_selectedNode && !_selectedNode->isEntry)
    {
        ImGui::Text("Clip Settings");
        ImGui::Separator();

        auto clip = _selectedNode->clip;
        if (clip)
        {
            // Speed 설정
            float speed = clip->speed;
            if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 10.0f))
            {
                clip->speed = speed;
                SCENE.UpdateAnimatorClipInXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _selectedNode->clip->name, speed, _selectedNode->clip->isLoop);
            }

            // Loop 설정
            bool isLoop = clip->isLoop;
            if (ImGui::Checkbox("Loop", &isLoop))
            {
                clip->isLoop = isLoop;
                SCENE.UpdateAnimatorClipInXML(SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _selectedNode->clip->name, speed, isLoop);
            }

            // Events 섹션 추가
            ImGui::Separator();
            ImGui::Text("Events");

            // 이벤트 추가 버튼
            if (ImGui::Button("+", ImVec2(30, 20)))
            {
                auto availableFunctions = _selectedAnimator->GetAvailableFunctions();
                if (!availableFunctions.empty())
                {
                    AnimationEvent newEvent;
                    newEvent.time = 0.0f;
                    newEvent.function = availableFunctions[0];  // 기본값으로 첫 번째 함수 선택
                    clip->events.push_back(newEvent);

                    // XML 업데이트 추가
                    SCENE.UpdateAnimatorClipEventsInXML(
                        SCENE.GetActiveScene()->GetSceneName(),
                        _selectedAnimator->GetGameObject()->GetName(),
                        clip->name,
                        clip->events
                    );
                }
            }

            shared_ptr<Model> model = _selectedAnimator->GetGameObject()->GetComponent<MeshRenderer>()->GetModel();
            shared_ptr<ModelAnimation> anim = model->GetAnimationByIndex(clip->animIndex);

            // 타임라인 영역
            float timelineWidth = ImGui::GetContentRegionAvail().x * 0.9f;
            float timelineHeight = 20.0f;

            // 가운데 정렬을 위한 좌측 여백 계산
            float leftPadding = (ImGui::GetContentRegionAvail().x - timelineWidth) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + leftPadding);

            ImVec2 timelineStart = ImGui::GetCursorScreenPos();
            ImVec2 timelineEnd = ImVec2(timelineStart.x + timelineWidth, timelineStart.y + timelineHeight);

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // 타임라인 배경 (밝은 회색)
            drawList->AddRectFilled(timelineStart, timelineEnd, IM_COL32(80, 80, 80, 255));

            // 눈금 그리기
            int numDivisions = 6; // 0.0 ~ 1.0을 6등분
            float divisionWidth = timelineWidth / numDivisions;
            for (int i = 0; i <= numDivisions; i++)
            {
                float x = timelineStart.x + (i * divisionWidth);

                // 세로 눈금선
                drawList->AddLine(
                    ImVec2(x, timelineStart.y),
                    ImVec2(x, timelineEnd.y),
                    IM_COL32(120, 120, 120, 255)
                );

                // 시간 텍스트
                char text[32];
                sprintf_s(text, "%.1f", static_cast<float>(i) / numDivisions);
                float textWidth = ImGui::CalcTextSize(text).x;
                drawList->AddText(
                    ImVec2(x - (textWidth * 0.5f), timelineEnd.y + 2),  // 텍스트 중앙 정렬
                    IM_COL32(200, 200, 200, 255),
                    text
                );
            }

            // 가로 경계선
            drawList->AddLine(
                timelineStart,
                ImVec2(timelineEnd.x, timelineStart.y),
                IM_COL32(120, 120, 120, 255)
            );
            drawList->AddLine(
                ImVec2(timelineStart.x, timelineEnd.y),
                timelineEnd,
                IM_COL32(120, 120, 120, 255)
            );

            vector<ImVec4> markerColors(0);

            // 이벤트 마커와 설정 UI 개선
            for (int i = 0; i < clip->events.size(); i++)
            {
                auto& event = clip->events[i];
                float markerX = timelineStart.x + (event.time * timelineWidth);

                // 각 이벤트마다 다른 색상 생성
                float hue = (i * 0.618034f);  // 황금비를 사용하여 균일한 색상 분포
                hue = fmodf(hue, 1.0f);
                ImVec4 color = HSVtoRGB(hue, 0.8f, 0.9f);
                ImU32 markerColor = ImGui::ColorConvertFloat4ToU32(color);
                markerColors.push_back(color);

                // 마커 드래그 로직
                ImVec2 markerPos(markerX, timelineStart.y);
                ImVec2 markerSize(5, timelineHeight);

                // 마커 그리기
                drawList->AddRectFilled(
                    markerPos,
                    ImVec2(markerPos.x + markerSize.x, markerPos.y + markerSize.y),
                    markerColor
                );

                // 마커 드래그 처리
                ImGui::SetCursorScreenPos(markerPos);
                ImGui::InvisibleButton(("##marker" + std::to_string(i)).c_str(), markerSize);

                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    float newX = ImGui::GetIO().MousePos.x - timelineStart.x;
                    event.time = std::clamp(newX / timelineWidth, 0.0f, 1.0f);

                    // XML 업데이트
                    SCENE.UpdateAnimatorClipEventsInXML(
                        SCENE.GetActiveScene()->GetSceneName(),
                        _selectedAnimator->GetGameObject()->GetName(),
                        clip->name,
                        clip->events
                    );
                }
            }

            // 타임라인 아래에 이벤트 설정 UI 배치
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + timelineHeight + 20);

            // 이벤트 목록 헤더
            ImGui::Text("Function");
            ImGui::Separator();

            for (int i = 0; i < clip->events.size(); i++)
            {
                auto& event = clip->events[i];
                ImGui::PushID(i);
                char timeText[16];
                sprintf_s(timeText, "(%.2f)", event.time);
                ImVec2 textSize = ImGui::CalcTextSize(timeText);

                // 같은 색상을 시간값 배경에도 사용
                ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(markerColors[i].x, markerColors[i].y, markerColors[i].z, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));

                // 배경 사각형 그리기
                ImVec2 textPos = ImGui::GetCursorScreenPos();
                drawList->AddRectFilled(
                    textPos,
                    ImVec2(textPos.x + textSize.x, textPos.y + textSize.y),
                    bgColor,
                    4.0f  // 모서리 둥글게
                );

                ImGui::Text(timeText);  // 시간값 표시
                ImGui::PopStyleColor();

                // 함수 선택 콤보박스
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo(("##Function" + std::to_string(i)).c_str(),
                    event.function.functionKey.c_str()))
                {
                    auto availableFunctions = _selectedAnimator->GetAvailableFunctions();
                    for (const auto& availableFunc : availableFunctions)
                    {
                        if (ImGui::Selectable(availableFunc.functionKey.c_str()))
                        {
                            event.function = availableFunc;
                            SCENE.UpdateAnimatorClipEventsInXML(
                                SCENE.GetActiveScene()->GetSceneName(),
                                _selectedAnimator->GetGameObject()->GetName(),
                                clip->name,
                                clip->events
                            );
                        }
                    }
                    ImGui::EndCombo();
                }

                // 삭제 버튼 (오른쪽 정렬, 작은 크기)
                float windowWidth = ImGui::GetContentRegionAvail().x;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + windowWidth - 30.0f);  // 오른쪽에서 30픽셀 위치
                if (ImGui::Button("X##delete", ImVec2(25.0f, 0)))  // 25픽셀 너비
                {
                    clip->events.erase(clip->events.begin() + i);
                    i--;
                    SCENE.UpdateAnimatorClipEventsInXML(
                        SCENE.GetActiveScene()->GetSceneName(),
                        _selectedAnimator->GetGameObject()->GetName(),
                        clip->name,
                        clip->events
                    );
                }

                ImGui::PopID();
            }
        }
    }
}

void GUIManager::RenderNode(NodeData& node)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    // 노드의 실제 화면 위치 계산
    ImVec2 nodePos = ImVec2(
        canvasPos.x + node.pos.x,
        canvasPos.y + node.pos.y
    );
    
    if (node.name != "Entry")
    {
        if (node.clip->pos.x != 0 && node.clip->pos.y != 0)
        {
            node.pos = node.clip->pos;
            nodePos = ImVec2(
                canvasPos.x + node.pos.x,
                canvasPos.y + node.pos.y
            );
        }
    }
    
    // 노드 색상 설정
    ImU32 nodeColor;
    if (node.isEntry)
    {
        nodeColor = IM_COL32(76, 156, 76, 255);  // 초록색
    }
    else if (_selectedAnimator && _selectedAnimator->_entry && node.clip == _selectedAnimator->_entry)
    {
        nodeColor = IM_COL32(255, 140, 0, 255);  // 주황색
    }
    else
    {
        nodeColor = IM_COL32(51, 122, 183, 255);  // 파란색
    }

    // 노드 그리기
    drawList->AddRectFilled(
        nodePos,
        ImVec2(nodePos.x + NODE_WIDTH, nodePos.y + NODE_HEIGHT),
        nodeColor,
        4.0f
    );

    // 프로그레스 바 그리기
    bool isCurrentNode = (_selectedAnimator && _selectedAnimator->_currClip && _selectedAnimator->_currClip == node.clip);
    bool isBlendingNode = (_selectedAnimator && _selectedAnimator->_currTransition &&
        (_selectedAnimator->_currTransition->clipA.lock() == node.clip ||
            _selectedAnimator->_currTransition->clipB.lock() == node.clip));

    if (isCurrentNode || isBlendingNode)
    {
        float progress = node.clip->progressRatio;
        ImVec2 progressBarMin = nodePos;
        ImVec2 progressBarMax = ImVec2(nodePos.x + (NODE_WIDTH * progress), nodePos.y + NODE_HEIGHT);

        ImU32 progressColor = isCurrentNode ?
            IM_COL32(100, 200, 255, 128) :  // 현재 노드는 밝은 파란색
            IM_COL32(100, 255, 200, 128);   // 블렌딩 노드는 밝은 초록색

        drawList->AddRectFilled(progressBarMin, progressBarMax, progressColor, 4.0f);
    }

    // 선택된 노드는 테두리 강조
    if (&node == _selectedNode)
    {
        drawList->AddRect(
            nodePos,
            ImVec2(nodePos.x + NODE_WIDTH, nodePos.y + NODE_HEIGHT),
            IM_COL32(255, 255, 255, 255),  // 흰색 테두리
            4.0f,
            0,
            2.0f
        );
    }

    // 노드 제목
    ImVec2 textPos(
        nodePos.x + (NODE_WIDTH * 0.5f),
        nodePos.y + (NODE_HEIGHT * 0.5f)
    );

    ImVec2 textSize = ImGui::CalcTextSize(node.name.c_str());
    drawList->AddText(
        ImVec2(textPos.x - textSize.x * 0.5f, textPos.y - textSize.y * 0.5f),
        IM_COL32(255, 255, 255, 255),
        node.name.c_str()
    );

    // 드래그 앤 드롭을 위한 투명 버튼
    ImGui::SetCursorScreenPos(nodePos);

    string buttonId = "##" + node.name;
    if (ImGui::InvisibleButton(buttonId.c_str(), ImVec2(NODE_WIDTH, NODE_HEIGHT)))
    {
        // 클릭 처리 (필요한 경우)
    }

    // 드래그 처리
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        node.pos.x += ImGui::GetIO().MouseDelta.x;
        node.pos.y += ImGui::GetIO().MouseDelta.y;

        if (node.name != "Entry")
            node.clip->pos = node.pos;

    }

    // 드래그가 끝났을 때만 XML 저장
    if (ImGui::IsItemDeactivated() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        // 노드 위치 XML에 저장
        SCENE.UpdateAnimatorNodePositionInXML(
            SCENE.GetActiveScene()->GetSceneName(),
            _selectedAnimator->GetGameObject()->GetName(),
            node.name,
            node.pos
        );
    }

    // 호버 효과
    if (ImGui::IsItemHovered())
    {
        drawList->AddRect(
            nodePos,
            ImVec2(nodePos.x + NODE_WIDTH, nodePos.y + NODE_HEIGHT),
            IM_COL32(255, 255, 255, 100),
            4.0f,
            0,
            2.0f
        );
    }

    if (ImGui::IsItemClicked())
    {
        _selectedNode = &node;
        _selectedTransition = nullptr;
    }
}

void GUIManager::RenderTransitions()
{
    if (!_selectedAnimator)
        return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();

    // Entry 노드와 Entry Clip 사이의 연결
    if (_selectedAnimator->_entry)
    {
        NodeData* entryNode = nullptr;
        NodeData* entryClipNode = nullptr;

        for (auto& node : _nodes)
        {
            if (node.isEntry)
                entryNode = &node;
            else if (_selectedAnimator->_entry && node.clip == _selectedAnimator->_entry)
                entryClipNode = &node;
        }

        if (entryNode && entryClipNode)
        {
            ImVec2 start(
                canvasPos.x + entryNode->pos.x + (NODE_WIDTH * 0.5f),
                canvasPos.y + entryNode->pos.y + (NODE_HEIGHT * 0.5f)
            );

            // 끝점: 노드의 중앙
            ImVec2 end(
                canvasPos.x + entryClipNode->pos.x + (NODE_WIDTH * 0.5f),
                canvasPos.y + entryClipNode->pos.y + (NODE_HEIGHT * 0.5f)
            );

            DrawConnection(drawList, start, end);
        }
    }

    // 일반 트랜지션 연결
    // 실제 존재하는 트랜지션에 대해서만 화살표를 그림
    for (const auto& transition : _selectedAnimator->_transitions)
    {
        auto clipA = transition->clipA.lock();
        auto clipB = transition->clipB.lock();
        if (!clipA || !clipB)
            continue;

        // 시작점과 끝점 노드 찾기
        NodeData* startNode = nullptr;
        NodeData* endNode = nullptr;
        for (auto& node : _nodes)
        {
            if (node.clip == clipA) startNode = &node;
            if (node.clip == clipB) endNode = &node;
        }

        if (!startNode || !endNode)
            continue;

        ImVec2 start(
            canvasPos.x + startNode->pos.x + (NODE_WIDTH * 0.5f),
            canvasPos.y + startNode->pos.y + (NODE_HEIGHT * 0.5f)
        );

        // 끝점: 노드의 중앙
        ImVec2 end(
            canvasPos.x + endNode->pos.x + (NODE_WIDTH * 0.5f),
            canvasPos.y + endNode->pos.y + (NODE_HEIGHT * 0.5f)
        );

        DrawConnection(drawList, start, end);

        // 화살표 클릭 감지
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            float dist = DistancePointToLineSegment(mousePos, start, end);
            if (dist < 5.0f)  // 클릭 허용 범위
            {
                _rightClickedTransition = transition;
                ImGui::OpenPopup("TransitionContextMenu");
            }
        }
    }

    // 트랜지션 우클릭 메뉴
    if (ImGui::BeginPopup("TransitionContextMenu"))
    {
        if (ImGui::MenuItem("Delete"))
        {
            if (_rightClickedTransition)
            {
                auto clipA = _rightClickedTransition->clipA.lock();
                auto clipB = _rightClickedTransition->clipB.lock();
                if (clipA && clipB)
                {
                    // XML에서 삭제
                    SCENE.RemoveAnimatorTransitionFromXML(
                        SCENE.GetActiveScene()->GetSceneName(),
                        _selectedAnimator->GetGameObject()->GetName(),
                        clipA->name, clipB->name);

                    // 현재 트랜지션인 경우 초기화
                    if (_selectedAnimator->_currTransition == _rightClickedTransition)
                    {
                        _selectedAnimator->_currTransition = nullptr;
                    }

                    // clipA의 transitions 목록에서 삭제
                    clipA->transitions.erase(
                        std::remove(clipA->transitions.begin(), clipA->transitions.end(), _rightClickedTransition),
                        clipA->transitions.end()
                    );

                    // clipA의 현재 transition이 삭제되는 트랜지션인 경우 초기화
                    if (clipA->transition == _rightClickedTransition)
                    {
                        clipA->transition = nullptr;
                    }

                    // Animator의 transitions 목록에서 삭제
                    auto& transitions = _selectedAnimator->_transitions;
                    transitions.erase(
                        std::remove(transitions.begin(), transitions.end(), _rightClickedTransition),
                        transitions.end()
                    );
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // 노드 우클릭 메뉴
    for (auto& node : _nodes)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 nodeMin(
                canvasPos.x + node.pos.x,
                canvasPos.y + node.pos.y
            );
            ImVec2 nodeMax(
                nodeMin.x + NODE_WIDTH,
                nodeMin.y + NODE_HEIGHT
            );

            if (mousePos.x >= nodeMin.x && mousePos.x <= nodeMax.x &&
                mousePos.y >= nodeMin.y && mousePos.y <= nodeMax.y)
            {
                _rightClickedNode = &node;
                ImGui::OpenPopup("NodeContextMenu");
            }
        }
    }

    if (ImGui::BeginPopup("NodeContextMenu"))
    {
        if (_rightClickedNode && !_rightClickedNode->isEntry)
        {
            if (ImGui::MenuItem("Set as Default State"))
            {
                // Entry 노드와의 기존 연결 해제
                auto& transitions = _selectedAnimator->_transitions;
                for (auto it = transitions.begin(); it != transitions.end();)
                {
                    auto clipA = (*it)->clipA.lock();
                    if (clipA && clipA == _selectedAnimator->_entry)
                    {
                        it = transitions.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }

                // 새로운 Entry 설정
                _selectedAnimator->SetEntryClip(_rightClickedNode->clip->name);

                // XML 업데이트
                SCENE.UpdateAnimatorEntryClipInXML(
                    SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _rightClickedNode->clip->name);
            }

            if (ImGui::MenuItem("Make Transition"))
            {
                _isCreatingTransition = true;
                _transitionStartNode = _rightClickedNode;
                _transitionEndPos = ImGui::GetMousePos();
            }
        }
        ImGui::EndPopup();
    }

    // 트랜지션 생성 중일 때 화살표 그리기
    if (_isCreatingTransition)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();

        // 시작점 계산
        ImVec2 startPos(
            canvasPos.x + _transitionStartNode->pos.x + (NODE_WIDTH * 0.5f),
            canvasPos.y + _transitionStartNode->pos.y + (NODE_HEIGHT * 0.5f)
        );

        // 마우스 위치 업데이트
        ImVec2 mousePos = ImGui::GetMousePos();
        _transitionEndPos = mousePos;

        // 다른 노드 위에 있는지 확인
        NodeData* targetNode = nullptr;
        for (auto& node : _nodes)
        {
            if (&node != _transitionStartNode && !node.isEntry)
            {
                ImVec2 nodeMin(
                    canvasPos.x + node.pos.x,
                    canvasPos.y + node.pos.y
                );
                ImVec2 nodeMax(
                    nodeMin.x + NODE_WIDTH,
                    nodeMin.y + NODE_HEIGHT
                );

                if (mousePos.x >= nodeMin.x && mousePos.x <= nodeMax.x &&
                    mousePos.y >= nodeMin.y && mousePos.y <= nodeMax.y)
                {
                    targetNode = &node;
                    _transitionEndPos = ImVec2(
                        canvasPos.x + node.pos.x + (NODE_WIDTH * 0.5f),
                        canvasPos.y + node.pos.y + (NODE_HEIGHT * 0.5f)
                    );
                    break;
                }
            }
        }

        DrawConnection(drawList, startPos, _transitionEndPos);

        // 마우스 좌클릭 감지
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (targetNode)
            {
                // 새 트랜지션 생성
                _selectedAnimator->AddTransition(_transitionStartNode->clip->name, targetNode->clip->name);

                // XML에 새 트랜지션 추가
                SCENE.AddAnimatorTransitionToXML(
                    SCENE.GetActiveScene()->GetSceneName(),
                    _selectedAnimator->GetGameObject()->GetName(),
                    _transitionStartNode->clip->name,
                    targetNode->clip->name);
            }
            _isCreatingTransition = false;
        }
    }
    
}
void GUIManager::DrawConnection(ImDrawList* drawList, const ImVec2& start, const ImVec2& end)
{
    if (!_showAnimatorEditor) 
        return;

    ImU32 arrowColor = IM_COL32(255, 255, 255, 255);
    ImU32 highlightColor = IM_COL32(135, 195, 255, 255);

    // 현재 라인이 양방향 연결인지 확인
    bool isBidirectional = false;
    shared_ptr<Transition> oppositeTransition = nullptr;

    // 현재 연결의 반대 방향 트랜지션이 존재하는지 확인
    if (_selectedAnimator)
    {
        for (const auto& transition : _selectedAnimator->_transitions)
        {
            auto clipA = transition->clipA.lock();
            auto clipB = transition->clipB.lock();
            if (!clipA || !clipB)
                continue;

            // 반대 방향의 트랜지션 찾기
            if (IsOppositeTransition(start, end,
                GetNodeCenterPos(clipA), GetNodeCenterPos(clipB)))
            {
                isBidirectional = true;
                oppositeTransition = transition;
                break;
            }
        }
    }

    // 마우스 위치 가져오기
    ImVec2 mousePos = ImGui::GetMousePos();
    bool isHovered = false;

    // 양방향일 경우 라인을 약간 이동
    ImVec2 adjustedStart = start;
    ImVec2 adjustedEnd = end;

    float cashedNormalX;
    float cashedNormalY;

    if (isBidirectional)
    {
        const float offset = 8.0f;
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        float length = sqrt(dx * dx + dy * dy);

        if (length > 0)
        {
            // 항상 위/아래로 일관되게 offset 적용
            float normalX = -dy / length * offset;
            float normalY = dx / length * offset;


            adjustedStart.x += normalX;
            adjustedStart.y += normalY;
            adjustedEnd.x += normalX;
            adjustedEnd.y += normalY;

            cashedNormalX = normalX;
            cashedNormalY = normalY;
        }
    }

    // 직선 그리기 및 클릭 감지
    float distanceToLine = DistancePointToLineSegment(mousePos, adjustedStart, adjustedEnd);
    if (distanceToLine < 5.0f)
    {
        isHovered = true;
        _isAnyArrowHovered = true;
    }

    ImU32 currentColor = (isHovered ||
        (_selectedTransition && _selectedTransition == GetTransitionFromPoints(start, end)))
        ? highlightColor : arrowColor;

    // 조정된 위치로 선 그리기
    drawList->AddLine(adjustedStart, adjustedEnd, currentColor, 2.0f);

    // 선의 중앙점 계산 (조정된 위치 사용)
    ImVec2 midPoint(
        adjustedStart.x + (adjustedEnd.x - adjustedStart.x) * 0.5f,
        adjustedStart.y + (adjustedEnd.y - adjustedStart.y) * 0.5f
    );

    // 화살표 머리 그리기
    float angle = atan2(adjustedEnd.y - adjustedStart.y, adjustedEnd.x - adjustedStart.x);

    DrawArrowHead(drawList, midPoint, angle, currentColor);

    if (isHovered)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            NodeData* clickedStartNode = nullptr;
            NodeData* clickedEndNode = nullptr;

            for (auto& node : _nodes)
            {
                ImVec2 nodeCenter(
                    canvasPos.x + node.pos.x + (NODE_WIDTH * 0.5f),
                    canvasPos.y + node.pos.y + (NODE_HEIGHT * 0.5f)
                );

                // 현재 노드가 시작점인지 확인
                for (const auto& transition : _selectedAnimator->_transitions)
                {
                    auto clipA = transition->clipA.lock();
                    if (clipA && clipA == node.clip)
                    {
                        // 시작점에 대한 offset 계산
                        ImVec2 adjustedNodeCenter = nodeCenter;
                        if (isBidirectional)
                        {
                            adjustedNodeCenter.x += cashedNormalX;  // normalX는 이미 계산된 offset 값
                            adjustedNodeCenter.y += cashedNormalY;  // normalY는 이미 계산된 offset 값
                        }

                        const float threshold = 1.0f;
                        if (abs(adjustedNodeCenter.x - adjustedStart.x) < threshold &&
                            abs(adjustedNodeCenter.y - adjustedStart.y) < threshold)
                        {
                            clickedStartNode = &node;
                            break;
                        }
                    }
                }

                // 현재 노드가 끝점인지 확인
                for (const auto& transition : _selectedAnimator->_transitions)
                {
                    auto clipB = transition->clipB.lock();
                    if (clipB && clipB == node.clip)
                    {
                        // 끝점에 대한 offset 계산
                        ImVec2 adjustedNodeCenter = nodeCenter;
                        if (isBidirectional)
                        {
                            adjustedNodeCenter.x += cashedNormalX;
                            adjustedNodeCenter.y += cashedNormalY;
                        }

                        const float threshold = 1.0f;
                        if (abs(adjustedNodeCenter.x - adjustedEnd.x) < threshold &&
                            abs(adjustedNodeCenter.y - adjustedEnd.y) < threshold)
                        {
                            clickedEndNode = &node;
                            break;
                        }
                    }
                }
            }

            // Entry 노드에서 시작하는 화살표는 무시
            if (clickedStartNode && clickedStartNode->isEntry)
                return;

            // 찾은 노드들로 해당하는 트랜지션 찾기
            if (clickedStartNode && clickedEndNode)
            {
                for (const auto& transition : _selectedAnimator->_transitions)
                {
                    auto clipA = transition->clipA.lock();
                    auto clipB = transition->clipB.lock();
                    if (!clipA || !clipB)
                        continue;

                    if (clipA == clickedStartNode->clip && clipB == clickedEndNode->clip)
                    {
                        _selectedTransition = transition;
                        _selectedNode = nullptr;  // 노드 선택 해제
                        break;
                    }
                }
            }
        }
    }
}

void GUIManager::DrawArrowHead(ImDrawList* drawList, const ImVec2& pos, float angle, ImU32 color)
{
    float arrowSize = 10.0f;
    ImVec2 arrowP1(
        pos.x - arrowSize * cos(angle - 0.5f),
        pos.y - arrowSize * sin(angle - 0.5f)
    );
    ImVec2 arrowP2(
        pos.x - arrowSize * cos(angle + 0.5f),
        pos.y - arrowSize * sin(angle + 0.5f)
    );

    drawList->AddTriangleFilled(pos, arrowP1, arrowP2, color);
}

void GUIManager::DrawArrow(ImDrawList* drawList, const ImVec2& start, const ImVec2& end,
    ImU32 color, float thickness)
{
    // 메인 라인 그리기
    drawList->AddLine(start, end, color, thickness);

    // 화살표 머리 그리기
    float angle = atan2(end.y - start.y, end.x - start.x);
    float arrowSize = 10.0f;

    ImVec2 arrowP1(
        end.x - arrowSize * cos(angle - 0.5f),
        end.y - arrowSize * sin(angle - 0.5f)
    );
    ImVec2 arrowP2(
        end.x - arrowSize * cos(angle + 0.5f),
        end.y - arrowSize * sin(angle + 0.5f)
    );

    drawList->AddTriangleFilled(end, arrowP1, arrowP2, color);
}

// 점과 선분 사이의 거리를 계산하는 헬퍼 함수
float GUIManager::DistancePointToLineSegment(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd)
{
    float A = point.x - lineStart.x;
    float B = point.y - lineStart.y;
    float C = lineEnd.x - lineStart.x;
    float D = lineEnd.y - lineStart.y;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = -1;

    if (len_sq != 0)
        param = dot / len_sq;

    float xx, yy;

    if (param < 0) {
        xx = lineStart.x;
        yy = lineStart.y;
    }
    else if (param > 1) {
        xx = lineEnd.x;
        yy = lineEnd.y;
    }
    else {
        xx = lineStart.x + param * C;
        yy = lineStart.y + param * D;
    }

    float dx = point.x - xx;
    float dy = point.y - yy;
    return sqrt(dx * dx + dy * dy);
}

ImVec2 GUIManager::GetNodeCenterPos(shared_ptr<Clip> clip)
{
    for (const auto& node : _nodes)
    {
        if (node.clip == clip)
        {
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            return ImVec2(
                canvasPos.x + node.pos.x + (NODE_WIDTH * 0.5f),
                canvasPos.y + node.pos.y + (NODE_HEIGHT * 0.5f)
            );
        }
    }
    return ImVec2(0, 0);
}

bool GUIManager::IsOppositeTransition(const ImVec2& start1, const ImVec2& end1, const ImVec2& start2, const ImVec2& end2)
{
    const float threshold = 1.0f;
    return (abs(start1.x - end2.x) < threshold && abs(start1.y - end2.y) < threshold &&
        abs(end1.x - start2.x) < threshold && abs(end1.y - start2.y) < threshold);
}

bool GUIManager::IsCurrentTransitionNewer(shared_ptr<Transition> current, shared_ptr<Transition> other)
{
    if (!current || !other)
        return true;

    // 트랜지션 리스트에서 위치를 비교하여 나중에 추가된 것 확인
    auto& transitions = _selectedAnimator->_transitions;
    auto currentIt = std::find(transitions.begin(), transitions.end(), current);
    auto otherIt = std::find(transitions.begin(), transitions.end(), other);

    if (currentIt == transitions.end() || otherIt == transitions.end())
        return true;

    return std::distance(transitions.begin(), currentIt) >
        std::distance(transitions.begin(), otherIt);
}

void GUIManager::ShowShaderInspector(const filesystem::path& xmlPath)
{
    // XML 파일 로드
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        return;

    auto root = doc.FirstChildElement("Shader");
    if (!root)
        return;

    // 셰이더 정보 표시
    if (auto nameElem = root->FirstChildElement("Name"))
        ImGui::Text("Shader Name: %s", nameElem->GetText());

    if (auto pathElem = root->FirstChildElement("Path"))
    {
        string shaderPath = pathElem->GetText();
        ImGui::Text("Path: %s", shaderPath.c_str());

        // XML 파일이 있는 Resource/Shader 폴더의 상위 폴더들로 이동
        filesystem::path hlslPath = xmlPath.parent_path().parent_path().parent_path();
        // HLSL 파일 경로 구성 (GameCoding/Shader/xxx.hlsl)
        hlslPath /= shaderPath;

        // HLSL 파일 읽기
        ifstream file(hlslPath);
        if (file.is_open())
        {
            stringstream buffer;
            buffer << file.rdbuf();
            _shaderCode = buffer.str();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Shader Code:");

            ImGui::BeginChild("ShaderCode", ImVec2(0, 0), true,
                ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.3f));
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

            ImGui::TextUnformatted(_shaderCode.c_str());

            ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::EndChild();
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                "Failed to open shader file: %s", hlslPath.string().c_str());
        }
    }
}

void GUIManager::ShowScriptInspector(const filesystem::path& xmlPath)
{
    // XML 파일 로드
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        return;

    auto root = doc.FirstChildElement("Script");
    if (!root)
        return;

    // 스크립트 정보 표시
    if (auto classNameElem = root->FirstChildElement("ClassName"))
        ImGui::Text("Class Name: %s", classNameElem->GetText());

    if (auto displayNameElem = root->FirstChildElement("DisplayName"))
        ImGui::Text("Display Name: %s", displayNameElem->GetText());

    ImGui::Spacing();
    ImGui::Separator();

    // 헤더 파일 내용 표시
    if (auto headerElem = root->FirstChildElement("HeaderContent"))
    {
        ImGui::Text("Header File (.h):");
        ImGui::BeginChild("HeaderCode", ImVec2(0, ImGui::GetWindowHeight() * 0.4f), true,
            ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.3f));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

        ImGui::TextUnformatted(headerElem->GetText());

        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::EndChild();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // CPP 파일 내용 표시
    if (auto cppElem = root->FirstChildElement("CppContent"))
    {
        ImGui::Text("Source File (.cpp):");
        ImGui::BeginChild("CppCode", ImVec2(0, ImGui::GetWindowHeight() * 0.4f), true,
            ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.3f));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

        ImGui::TextUnformatted(cppElem->GetText());

        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::EndChild();
    }
}

void GUIManager::ShowMaterialInspector(const filesystem::path& xmlPath)
{
    // XML 파일 로드
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        return;

    auto root = doc.FirstChildElement("Material");
    if (!root)
        return;

    // Material 이름 먼저 가져오기
    auto nameElem = root->FirstChildElement("Name");
    if (!nameElem)
        return;

    // 메모리에 로드된 Material 가져오기
    auto material = RESOURCE.GetResource<Material>(Utils::ToWString(nameElem->GetText()));
    if (!material)
        return;

    bool isModified = false;

    // Material 이름 표시
    if (auto nameElem = root->FirstChildElement("Name"))
        ImGui::Text("Material Name: %s", nameElem->GetText());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 스타일 설정
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 10));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

    // Texture 선택
    if (auto textureElem = root->FirstChildElement("Texture"))
    {
        string currentTexture = textureElem->GetText();
        ImGui::Text("Texture");
        ImGui::SameLine(100);

        // 텍스처 선택 버튼 스타일
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
        if (ImGui::Button(currentTexture.c_str(), ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("Select Texture");
        }
        ImGui::PopStyleVar();

        // 팝업 스타일 설정
        ImGui::SetNextWindowSize(ImVec2(500, 600));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.94f, 0.94f, 0.94f, 1.0f));

        if (ImGui::BeginPopupModal("Select Texture", nullptr, ImGuiWindowFlags_NoResize))
        {
            // 검색창 스타일 및 구현
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            static char searchBuffer[128] = "";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search textures...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 그리드 영역
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::BeginChild("TextureGrid", ImVec2(0, -40), true))
            {
                float cellSize = 100.0f;
                float iconSize = 80.0f;
                float padding = 10.0f;

                // 그리드 레이아웃 계산
                float availWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = static_cast<int>(availWidth / cellSize);
                if (columnCount < 1) columnCount = 1;

                ImGui::Columns(columnCount, nullptr, false);

                // None 옵션
                RenderTextureOption("None", nullptr, cellSize, iconSize, padding, textureElem, isModified);

                // 텍스처 목록
                string searchStr = ToLower(string(searchBuffer));
                for (const auto& entry : filesystem::directory_iterator("Resource/Texture"))
                {
                    if (entry.path().extension() == ".xml")
                    {
                        string textureName = entry.path().stem().string();
                        if (searchStr.empty() || ToLower(textureName).find(searchStr) != string::npos)
                        {
                            shared_ptr<Texture> icon = RESOURCE.GetResource<Texture>(Utils::ToWString(textureName));
                            RenderTextureOption(textureName, icon, cellSize, iconSize, padding, textureElem, isModified);
                        }
                    }
                }

                ImGui::Columns(1);
                ImGui::EndChild();
            }
            ImGui::PopStyleColor();

            // 하단 버튼
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    ImGui::Spacing();

    // NormalMap 선택
    if (auto normalMapElem = root->FirstChildElement("NormalMap"))
    {
        string currentNormalMap = normalMapElem->GetText();
        ImGui::Text("NormalMap");
        ImGui::SameLine(100);

        // 텍스처 선택 버튼 스타일
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
        if (ImGui::Button(currentNormalMap.c_str(), ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("Select NormalMap");
        }
        ImGui::PopStyleVar();

        // 팝업 스타일 설정
        ImGui::SetNextWindowSize(ImVec2(500, 600));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.94f, 0.94f, 0.94f, 1.0f));

        if (ImGui::BeginPopupModal("Select NormalMap", nullptr, ImGuiWindowFlags_NoResize))
        {
            // 검색창 스타일 및 구현
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            static char searchBuffer[128] = "";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search normal maps...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 그리드 영역
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::BeginChild("NormalMapGrid", ImVec2(0, -40), true))
            {
                float cellSize = 100.0f;
                float iconSize = 80.0f;
                float padding = 10.0f;

                // 그리드 레이아웃 계산
                float availWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = static_cast<int>(availWidth / cellSize);
                if (columnCount < 1) columnCount = 1;

                ImGui::Columns(columnCount, nullptr, false);

                // None 옵션
                RenderTextureOption("None", nullptr, cellSize, iconSize, padding, normalMapElem, isModified);

                // NormalMap 목록
                string searchStr = ToLower(string(searchBuffer));
                for (const auto& entry : filesystem::directory_iterator("Resource/Texture"))
                {
                    if (entry.path().extension() == ".xml")
                    {
                        string textureName = entry.path().stem().string();
                        if (searchStr.empty() || ToLower(textureName).find(searchStr) != string::npos)
                        {
                            shared_ptr<Texture> icon = RESOURCE.GetResource<Texture>(Utils::ToWString(textureName));
                            RenderTextureOption(textureName, icon, cellSize, iconSize, padding, normalMapElem, isModified);
                        }
                    }
                }

                ImGui::Columns(1);
                ImGui::EndChild();
            }
            ImGui::PopStyleColor();

            // 하단 버튼
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    ImGui::Spacing();

    // Shader 선택
    if (auto shaderElem = root->FirstChildElement("Shader"))
    {
        string currentShader = shaderElem->GetText();
        ImGui::Text("Shader");
        ImGui::SameLine(100);

        // 셰이더 선택 버튼 스타일
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
        if (ImGui::Button(currentShader.c_str(), ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("Select Shader");
        }
        ImGui::PopStyleVar();

        // 팝업 스타일 설정
        ImGui::SetNextWindowSize(ImVec2(500, 600));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.94f, 0.94f, 0.94f, 1.0f));

        if (ImGui::BeginPopupModal("Select Shader", nullptr, ImGuiWindowFlags_NoResize))
        {
            // 검색창 스타일 및 구현
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

            static char searchBuffer[128] = "";
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search shaders...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 그리드 영역
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::BeginChild("ShaderGrid", ImVec2(0, -40), true))
            {
                float cellSize = 100.0f;
                float iconSize = 80.0f;
                float padding = 10.0f;

                // 그리드 레이아웃 계산
                float availWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = static_cast<int>(availWidth / cellSize);
                if (columnCount < 1) columnCount = 1;

                ImGui::Columns(columnCount, nullptr, false);

                // Shader_Icon 로드
                shared_ptr<Texture> shaderIcon = RESOURCE.GetResource<Texture>(L"Shader_Icon");

                // Shader 목록
                string searchStr = ToLower(string(searchBuffer));
                for (const auto& entry : filesystem::directory_iterator("Resource/Shader"))
                {
                    if (entry.path().extension() == ".xml")
                    {
                        string shaderName = entry.path().stem().string();
                        if (searchStr.empty() || ToLower(shaderName).find(searchStr) != string::npos)
                        {
                            RenderTextureOption(shaderName, shaderIcon, cellSize, iconSize, padding, shaderElem, isModified);
                        }
                    }
                }

                ImGui::Columns(1);
                ImGui::EndChild();
            }
            ImGui::PopStyleColor();

            // 하단 버튼
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleVar();

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    // 첫 번째 스타일 세트 Pop
    ImGui::PopStyleColor(3);  // 버튼 관련 스타일
    ImGui::PopStyleVar(2);    // FramePadding, ItemSpacing

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Material Properties 섹션
    ImGui::Text("Material Properties");
    ImGui::Spacing();

    if (auto descElem = root->FirstChildElement("MaterialDesc"))
    {
        // 새로운 스타일 Push
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));

        // Ambient, Diffuse, Specular 색상 편집 (더 세련된 레이아웃)
        const float colorEditWidth = 180.0f;
        const char* colorLabels[] = { "Ambient", "Diffuse", "Specular" };
        const char* elemNames[] = { "Ambient", "Diffuse", "Specular" };

        for (int i = 0; i < 3; i++)
        {
            if (auto elem = descElem->FirstChildElement(elemNames[i]))
            {
                float color[3] = {
                    elem->FloatAttribute("r"),
                    elem->FloatAttribute("g"),
                    elem->FloatAttribute("b")
                };

                ImGui::Text(colorLabels[i]);
                ImGui::SameLine(100);
                ImGui::SetNextItemWidth(colorEditWidth);
                if (ImGui::ColorEdit3(("##" + string(colorLabels[i])).c_str(), color))
                {
                    elem->SetAttribute("r", color[0]);
                    elem->SetAttribute("g", color[1]);
                    elem->SetAttribute("b", color[2]);
                    isModified = true;
                }
                ImGui::Spacing();
            }
        }

        // 두 번째 스타일 세트 Pop
        ImGui::PopStyleColor(3);  // FrameBg 관련 스타일
        ImGui::PopStyleVar();     // FramePadding
    }

    // 변경사항이 있으면 파일 저장
    if (isModified)
    {
        doc.SaveFile(xmlPath.string().c_str());
        // Material 리소스 다시 로드
        RESOURCE.LoadMaterialData(xmlPath.wstring());

        // 메모리에 로드된 Material 업데이트
        auto material = RESOURCE.GetResource<Material>(Utils::ToWString(nameElem->GetText()));
        if (material)
        {
            // Shader 업데이트
            if (auto shaderElem = root->FirstChildElement("Shader"))
            {
                material->SetShader(RESOURCE.GetResource<Shader>(Utils::ToWString(shaderElem->GetText())));
            }

            // Texture 업데이트
            if (auto textureElem = root->FirstChildElement("Texture"))
            {
                material->SetTexture(RESOURCE.GetResource<Texture>(Utils::ToWString(textureElem->GetText())));
            }

            // NormalMap 업데이트
            if (auto normalMapElem = root->FirstChildElement("NormalMap"))
            {
                material->SetNormalMap(RESOURCE.GetResource<Texture>(Utils::ToWString(normalMapElem->GetText())));
            }
            // MaterialDesc 업데이트
            if (auto descElem = root->FirstChildElement("MaterialDesc"))
            {
                MaterialDesc matDesc;

                // Ambient 업데이트
                if (auto ambientElem = descElem->FirstChildElement("Ambient"))
                {
                    matDesc.ambient.x = ambientElem->FloatAttribute("r");
                    matDesc.ambient.y = ambientElem->FloatAttribute("g");
                    matDesc.ambient.z = ambientElem->FloatAttribute("b");
                    matDesc.ambient.w = ambientElem->FloatAttribute("a");
                }

                // Diffuse 업데이트
                if (auto diffuseElem = descElem->FirstChildElement("Diffuse"))
                {
                    matDesc.diffuse.x = diffuseElem->FloatAttribute("r");
                    matDesc.diffuse.y = diffuseElem->FloatAttribute("g");
                    matDesc.diffuse.z = diffuseElem->FloatAttribute("b");
                    matDesc.diffuse.w = diffuseElem->FloatAttribute("a");
                }

                // Specular 업데이트
                if (auto specularElem = descElem->FirstChildElement("Specular"))
                {
                    matDesc.specular.x = specularElem->FloatAttribute("r");
                    matDesc.specular.y = specularElem->FloatAttribute("g");
                    matDesc.specular.z = specularElem->FloatAttribute("b");
                    matDesc.specular.w = specularElem->FloatAttribute("a");
                }

                // Material의 MaterialDesc 업데이트
                material->SetMaterialDesc(matDesc);
            }
        
        }
    }
}

void GUIManager::ShowTextureInspector(const filesystem::path& xmlPath)
{
    // XML 파일 로드
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.string().c_str()) != tinyxml2::XML_SUCCESS)
        return;

    auto root = doc.FirstChildElement("Texture");
    if (!root)
        return;

    // 텍스처 정보 표시
    if (auto nameElem = root->FirstChildElement("Name"))
    {
        ImGui::Text("Texture Name: %s", nameElem->GetText());

        // 메모리에 로드된 텍스처 가져오기
        auto texture = RESOURCE.GetResource<Texture>(Utils::ToWString(nameElem->GetText()));
        if (texture)
        {
            ImGui::Spacing();
            ImGui::Separator();

            // 텍스처 미리보기
            float previewSize = ImGui::GetContentRegionAvail().x;
            ImGui::Image((ImTextureID)texture->GetShaderResourceView().Get(),
                ImVec2(previewSize, previewSize));

            ImGui::Spacing();
            ImGui::Separator();

            // 해상도 정보
            Vec2 size = texture->GetSize();
            ImGui::Text("Resolution: %.0f x %.0f", size.x, size.y);

            // 파일 경로 표시
            if (auto fileElem = root->FirstChildElement("File"))
            {
                ImGui::Text("File Path: %s", fileElem->GetText());
            }
        }
    }
}

void GUIManager::HandleExternalFilesDrop(const filesystem::path& sourcePath)
{
    // 이미지 파일인지 확인
    if (!IsImageFile(sourcePath))
        return;

    // GameCoding 폴더 경로 구성 (현재 경로 사용)
    filesystem::path basePath = filesystem::current_path();

    // 대상 파일 경로 구성
    filesystem::path destPath = basePath / sourcePath.filename();

    // 파일 복사
    CopyFileToResourceFolder(sourcePath, destPath);

    // XML 생성을 위한 이름 구성 (확장자 제외)
    wstring textureName = sourcePath.stem().wstring();
    filesystem::path resourcePath = basePath / "Resource" / "Texture";
    filesystem::path xmlPath = resourcePath / (textureName + L".xml");

    // XML 파일 생성
    RESOURCE.WriteTextureToXML(
        sourcePath.filename().wstring(),  // 이미지 파일명
        textureName,                      // 텍스처 이름
        xmlPath.wstring()                 // XML 저장 경로
    );

    // 새로운 리소스만 로드
    RESOURCE.LoadResourcesByType(resourcePath, ".xml", [](const wstring& path) {
        RESOURCE.LoadTextureData(path);
    });
}

bool GUIManager::IsViewportHovered()
{
    float viewportX = GP.GetViewWidth() * (1.0f / 10.0f);
    float viewportY = GP.GetViewHeight() * (3.0f / 100.0f);
    float viewportWidth = GP.GetViewWidth() * (7.0f / 10.0f);
    float viewportHeight = GP.GetViewHeight() * (6.0f / 10.0f);

    ImVec2 mousePos = ImGui::GetMousePos();
    return (mousePos.x >= viewportX && mousePos.x <= (viewportX + viewportWidth) &&
        mousePos.y >= viewportY && mousePos.y <= (viewportY + viewportHeight));
}

bool GUIManager::IsImageFile(const filesystem::path& path)
{
    wstring ext = path.extension().wstring();
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return ext == L".png" || ext == L".jpg" || ext == L".jpeg"
        || ext == L".bmp" || ext == L".tga" || ext == L".dds";
}

void GUIManager::CopyFileToResourceFolder(const filesystem::path& sourcePath, const filesystem::path& destPath)
{
    try {
        // 대상 폴더가 없으면 생성
        filesystem::create_directories(destPath.parent_path());

        // 파일 복사
        filesystem::copy_file(sourcePath, destPath,
            filesystem::copy_options::overwrite_existing);
    }
    catch (const filesystem::filesystem_error& e) {
        // 에러 처리
        OutputDebugStringA(e.what());
    }
}

void GUIManager::RenderScriptIcon(shared_ptr<Texture> icon, const string& filename, const filesystem::path& path, float cellSize, float iconSize, float padding, float maxTextWidth)
{
    float cursorPosX = ImGui::GetCursorPosX();
    float iconPosX = cursorPosX + (cellSize - iconSize) * 0.5f;
    ImGui::SetCursorPosX(iconPosX);

    ImGui::ImageButton(filename.c_str(),
        (ImTextureID)icon->GetShaderResourceView().Get(),
        ImVec2(iconSize, iconSize));

    // 드래그 중이 아닐 때만 스크립트 선택 처리
    if (ImGui::IsItemClicked() && !ImGui::IsMouseDragging(0))
    {
        _selectedScriptFile = path;
        _selectedFileType = FileType::Script;
    }

    // 드래그 소스 설정
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        // 드래그가 시작되면 스크립트 선택 해제
        _selectedScriptFile.clear();
        _selectedFileType = FileType::NONE;

        string fullPath = path.string();
        ImGui::SetDragDropPayload("RESOURCE_FILE", fullPath.c_str(), fullPath.size() + 1);
        ImGui::Text("%s", filename.c_str());
        ImGui::EndDragDropSource();
    }

    // 텍스트 처리 로직
    string displayText = filename;
    float textWidth = ImGui::CalcTextSize(displayText.c_str()).x;

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

    float textPosX = cursorPosX + (cellSize - textWidth) * 0.5f;
    ImGui::SetCursorPosX(textPosX);
    ImGui::Text("%s", displayText.c_str());

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding);
}

shared_ptr<Transition> GUIManager::GetTransitionFromPoints(const ImVec2& start, const ImVec2& end)
{
    if (!_selectedAnimator)
        return nullptr;

    // 모든 트랜지션을 순회하면서 시작점과 끝점이 일치하는 트랜지션 찾기
    for (const auto& transition : _selectedAnimator->_transitions)
    {
        auto clipA = transition->clipA.lock();
        auto clipB = transition->clipB.lock();
        if (!clipA || !clipB)
            continue;

        ImVec2 transStart = GetNodeCenterPos(clipA);
        ImVec2 transEnd = GetNodeCenterPos(clipB);

        // 위치 비교 시 약간의 오차 허용
        const float threshold = 10.0f;
        float startDist = sqrt(pow(transStart.x - start.x, 2) + pow(transStart.y - start.y, 2));
        float endDist = sqrt(pow(transEnd.x - end.x, 2) + pow(transEnd.y - end.y, 2));

        if (startDist < threshold && endDist < threshold)
        {
            return transition;
        }
    }

    return nullptr;
}

void GUIManager::OnResourceDroppedToViewport(const std::string& fullPath)
{
    // 경로에서 폴더명 추출
    const char* start = strstr(fullPath.c_str(), "Resource/");
    if (!start) return;

    start += strlen("Resource/");
    string folderName = string(start, strcspn(start, "\\"));

    // 파일명 추출 (확장자 포함)
    const char* lastSlash = strrchr(start, '\\');
    if (!lastSlash) return;
    string fileName = string(lastSlash + 1);

    // 레이캐스팅
    Matrix worldMatrix;
    shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
    shared_ptr<Camera> cameraComponent = camera->GetComponent<Camera>();
    Matrix projectionMatrix = cameraComponent->GetProjectionMatrix();
    Matrix viewMatrix = cameraComponent->GetViewMatrix();

    int32 mouseX = INPUT.GetMousePos().x;
    int32 mouseY = INPUT.GetMousePos().y;
    Ray ray = GP.GetViewport().GetRayFromScreenPoint(mouseX, mouseY, worldMatrix, viewMatrix, projectionMatrix, camera->transform()->GetWorldPosition());

    // Y=0 평면과의 교차점 계산
    Vec3 planeNormal(0, 1, 0);
    Vec3 planePoint(0, 0, 0);
    float d = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ray.direction, planeNormal));

    if (abs(d) > 0.0001f)
    {
        float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(planePoint - ray.position, planeNormal)) / d;
        Vec3 intersectionPoint = ray.position + ray.direction * t;

        if (_droppedObject == nullptr)
        {
            // Mesh 폴더 처리
            if (folderName == "Mesh")
            {
                string meshName = fileName.substr(0, fileName.find_last_of("."));

                if (meshName == "Cube")
                {
                    _droppedObject = SCENE.CreateCubeToScene(SCENE.GetActiveScene()->GetSceneName());
                }
                else if (meshName == "Sphere")
                {
                    _droppedObject = SCENE.CreateSphereToScene(SCENE.GetActiveScene()->GetSceneName());
                }
                else if (meshName == "Cylinder")
                {
                    _droppedObject = SCENE.CreateCylinderToScene(SCENE.GetActiveScene()->GetSceneName());
                }
                else if (meshName == "Grid")
                {
                    OutputDebugStringA("Grid");
                }
                else if (meshName == "Quad")
                {
                    OutputDebugStringA("Quad");
                }
                else if (meshName == "Terrain")
                {
                    OutputDebugStringA("Terrain");
                }
            }
            // Model 폴더 처리
            else if (folderName == "Model")
            {
                // XML 파일 파싱
                tinyxml2::XMLDocument doc;
                if (doc.LoadFile(fullPath.c_str()) == tinyxml2::XML_SUCCESS)
                {
                    tinyxml2::XMLElement* root = doc.FirstChildElement("Model");
                    if (root)
                    {
                        // Name 요소 추출
                        wstring modelName;
                        if (auto nameElement = root->FirstChildElement("Name"))
                        {
                            const char* nameStr = nameElement->GetText();
                            if (nameStr)
                            {
                                modelName = Utils::ToWString(nameStr);
                            }
                        }

                        tinyxml2::XMLElement* shaderElement = root->FirstChildElement("Shader");
                        if (shaderElement)
                        {
                            const char* shaderType = shaderElement->GetText();
                            if (strcmp(shaderType, "AnimatedMesh_Shader") == 0)
                            {
                                _droppedObject = SCENE.CreateAnimatedMeshToScene(SCENE.GetActiveScene()->GetSceneName(), modelName);
                            }
                            else if (strcmp(shaderType, "StaticMesh_Shader") == 0)
                            {
                                _droppedObject = SCENE.CreateStaticMeshToScene(SCENE.GetActiveScene()->GetSceneName(), modelName);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            _droppedObject->transform()->SetPosition(intersectionPoint);
        }
    }
}