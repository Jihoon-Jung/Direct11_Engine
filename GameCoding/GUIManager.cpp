#include "pch.h"
#include <string>
#include "GUIManager.h"
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
            for (int i = 0; i < gameObjects.size(); i++)
            {
                std::string objectName;
                objectName.assign(gameObjects[i]->GetName().begin(), gameObjects[i]->GetName().end());

                ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth;

                // 현재 선택된 오브젝트인 경우 Selected 플래그 추가
                if (_selectedObject == gameObjects[i])
                {
                    tree_flags |= ImGuiTreeNodeFlags_Selected;
                }

                ImGui::TreeNodeEx((void*)(intptr_t)i, tree_flags, objectName.c_str());

                // 단일 클릭 처리
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
                {
                    _selectedObject = gameObjects[i];  // 선택된 오브젝트 저장
                    SCENE.GetActiveScene()->AddPickedObject(_selectedObject);
                }

                // 더블 클릭 처리
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
                    Vec3 targetPos = _selectedObject->transform()->GetWorldPosition();
                    Vec3 offset = Vec3(0.0f, 2.0f, -5.0f);
                    Vec3 cameraTargetPos = targetPos + offset;

                    // 카메라가 타겟을 바라보도록 회전 계산
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

                    // 보간 시작 설정
                    _isCameraMoving = true;
                    _cameraMoveTime = 0.0f;
                    _cameraStartPos = camera->transform()->GetLocalPosition();
                    _cameraTargetPos = cameraTargetPos;
                    _cameraStartRot = camera->transform()->GetLocalRotation();
                    _cameraTargetRot = targetRotation;
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Delete"))
                    {
                        char buffer[100];
                        sprintf_s(buffer, "Delete.\n");
                        OutputDebugStringA(buffer);
                        // TODO: Delete GameObject 기능 구현
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::TreePop();
        }

        // Hierarchy 창 빈 공간에 대한 우클릭 메뉴
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Object"))
            {
                // TODO: Create Empty Object 기능 구현
            }
            if (ImGui::MenuItem("NULL1"))
            {
                // TODO: NULL1 기능 구현
            }
            if (ImGui::MenuItem("NULL2"))
            {
                // TODO: NULL2 기능 구현
            }
            if (ImGui::MenuItem("NULL3"))
            {
                // TODO: NULL3 기능 구현
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
                    float angle = XMConvertToRadians(tempRotation.x - lastRotation.x);
                    Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localRight, angle);
                    Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                    transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                    lastRotation.x = tempRotation.x;
                    currentEulerAngles.x = tempRotation.x;
                }

                // Y축 회전
                if (ImGui::DragFloat("Rotation Y", &tempRotation.y, 0.1f))
                {
                    float angle = XMConvertToRadians(tempRotation.y - lastRotation.y);
                    Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localUp, angle);
                    Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                    transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                    lastRotation.y = tempRotation.y;
                    currentEulerAngles.y = tempRotation.y;
                }

                // Z축 회전
                if (ImGui::DragFloat("Rotation Z", &tempRotation.z, 0.1f))
                {
                    float angle = XMConvertToRadians(tempRotation.z - lastRotation.z);
                    Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localForward, angle);
                    Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                    transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                    lastRotation.z = tempRotation.z;
                    currentEulerAngles.z = tempRotation.z;
                }

                if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                    transform->SetLocalScale(scale);
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
                else if (dynamic_pointer_cast<BaseCollider>(component))
                    componentName = "Collider";
                // ... 다른 컴포넌트 타입들 추가 ...

                if (!componentName.empty() && ImGui::CollapsingHeader(componentName.c_str()))
                {
                    // 각 컴포넌트 타입별 속성 표시
                    // TODO: 컴포넌트별 세부 속성 표시 로직 추가
                }
            }
        }

        ImGui::End();
    }

    // project
    {
        ImGui::SetNextWindowPos(ImVec2(0, GP.GetViewHeight() * (63.0f / 100.0f)), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(GP.GetViewWidth(), GP.GetViewHeight() * (37.0f / 100.0f)));
        ImGui::Begin("Project", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse);
        ImGui::End();
    }
    RenderGuizmo();
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

            // C키로 콜라이더 편집 모드 토글
            if (INPUT.GetButtonDown(KEY_TYPE::C))
            {
                _isColliderEditMode = !_isColliderEditMode;
                // 콜라이더 기즈모 색상 설정(초록색 계열)
                ImGuizmo::SetGizmoSizeClipSpace(0.08f);  // 기즈모 크기를 좀 더 크게
                ImGuizmo::AllowAxisFlip(false);  // 축 반전 비활성화

                // 기즈모 색상 커스터마이징
                ImGuizmo::Style& style = ImGuizmo::GetStyle();
                style.Colors[ImGuizmo::TRANSLATE_X] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // X축 초록색
                style.Colors[ImGuizmo::TRANSLATE_Y] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Y축 초록색
                style.Colors[ImGuizmo::TRANSLATE_Z] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Z축 초록색
                style.Colors[ImGuizmo::BOUNDS] = ImVec4(0.0f, 0.6f, 0.0f, 0.3f);  // 경계 초록색 (반투명)
            }
                

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
            if (collider)
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
                            Vec3 position, rotation, scale;
                            ImGuizmo::DecomposeMatrixToComponents(colliderMatrix.m[0], &position.x, &rotation.x, &scale.x);
                            // 오브젝트의 월드 스케일을 고려하여 콜라이더 스케일 설정
                            sphereCollider->SetScale(scale / objectScale);
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
