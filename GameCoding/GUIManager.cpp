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

    // Ÿ��Ʋ�� ���� ����
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

    // ��Ʈ ����
    ImFont* font = io.Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 13.0f);
    io.FontDefault = font;

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(GP.GetWindowHandle());
	ImGui_ImplDX11_Init(GP.GetDevice().Get(), GP.GetDeviceContext().Get());

    // ImGuizmo �ʱ�ȭ
    ImGuizmo::Enable(true);  // ImGuizmo Ȱ��ȭ
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
            // �ε巯�� ������ ���� easeInOutCubic �Լ� ���
            float t = EaseInOutCubic(_cameraMoveTime);

            // ��ġ ����
            Vec3 newPos = Vec3::Lerp(_cameraStartPos, _cameraTargetPos, t);

            // ȸ�� ���� (���ʹϿ� ���)
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

            // ��ȯ�� �� ����
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

    ImGuizmo::BeginFrame();  // ImGuizmo ������ ����
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
        // â �׵θ� ��Ÿ�� ����
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

        // ��ư ũ��� ���� ����
        float buttonWidth = windowSizeX * (0.2f / 10.0f);
        float buttonHeight = windowSizeY * (6.0f / 10.0f);
        float spacing = windowSizeX * (0.1f / 100.0f);
        float totalWidth = (buttonWidth * 3) + (spacing * 2);

        // â�� �߾� ��ġ ���
        float windowWidth = ImGui::GetWindowSize().x;
        float windowHeight = ImGui::GetWindowSize().y;
        float startX = (windowWidth - totalWidth) * 0.5f;
        float startY = (windowHeight - buttonHeight) * 0.5f;

        // Ŀ�� ��ġ ����
        ImGui::SetCursorPos(ImVec2(startX, startY));

        // ��ư ��Ÿ�� ����
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

        // �̹��� ��ư ����
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

        // ��Ÿ�� ����
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();

        ImGui::End();

        // â �׵θ� ��Ÿ�� ����
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

        // Scene TreeNode�� �⺻������ ���� ���·� ����
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

                // ���� ���õ� ������Ʈ�� ��� Selected �÷��� �߰�
                if (_selectedObject == gameObjects[i])
                {
                    tree_flags |= ImGuiTreeNodeFlags_Selected;
                }

                ImGui::TreeNodeEx((void*)(intptr_t)i, tree_flags, objectName.c_str());

                // ���� Ŭ�� ó��
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
                {
                    _selectedObject = gameObjects[i];  // ���õ� ������Ʈ ����
                    SCENE.GetActiveScene()->AddPickedObject(_selectedObject);
                }

                // ���� Ŭ�� ó��
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
                    Vec3 targetPos = _selectedObject->transform()->GetWorldPosition();
                    Vec3 offset = Vec3(0.0f, 2.0f, -5.0f);
                    Vec3 cameraTargetPos = targetPos + offset;

                    // ī�޶� Ÿ���� �ٶ󺸵��� ȸ�� ���
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

                    // ���� ���� ����
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
                        // TODO: Delete GameObject ��� ����
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::TreePop();
        }

        // Hierarchy â �� ������ ���� ��Ŭ�� �޴�
        if (ImGui::BeginPopupContextWindow("HierarchyContextMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Object"))
            {
                // TODO: Create Empty Object ��� ����
            }
            if (ImGui::MenuItem("NULL1"))
            {
                // TODO: NULL1 ��� ����
            }
            if (ImGui::MenuItem("NULL2"))
            {
                // TODO: NULL2 ��� ����
            }
            if (ImGui::MenuItem("NULL3"))
            {
                // TODO: NULL3 ��� ����
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
            // ������Ʈ �̸� ǥ��
            ImGui::Text("Name: %s", WStringToString(_selectedObject->GetName()).c_str());
            ImGui::Separator();

            // Transform�� �׻� ���� ǥ��
            shared_ptr<Transform> transform = _selectedObject->GetComponent<Transform>();

            if (transform && ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                static Vec3 currentEulerAngles = Vec3::Zero;
                static bool initialized = false;
                static GameObject* lastObject = nullptr;
                static Vec3 lastRotation = Vec3::Zero;

                // ���� ȸ�� ���� ��������
                Matrix currentRotationMatrix = Matrix::CreateFromQuaternion(transform->GetQTRotation());

                // ���� ���� �� ���
                Vec3 localRight = Vec3::TransformNormal(Vec3::Right, currentRotationMatrix);
                Vec3 localUp = Vec3::TransformNormal(Vec3::Up, currentRotationMatrix);
                Vec3 localForward = Vec3::TransformNormal(Vec3::Forward, currentRotationMatrix);

                // ���� ȸ������ ���Ϸ������� ��ȯ
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

                // X�� ȸ��
                if (ImGui::DragFloat("Rotation X", &tempRotation.x, 0.1f))
                {
                    float angle = XMConvertToRadians(tempRotation.x - lastRotation.x);
                    Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localRight, angle);
                    Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                    transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                    lastRotation.x = tempRotation.x;
                    currentEulerAngles.x = tempRotation.x;
                }

                // Y�� ȸ��
                if (ImGui::DragFloat("Rotation Y", &tempRotation.y, 0.1f))
                {
                    float angle = XMConvertToRadians(tempRotation.y - lastRotation.y);
                    Matrix rotationMatrix = Matrix::CreateFromAxisAngle(localUp, angle);
                    Quaternion deltaRotation = Quaternion::CreateFromRotationMatrix(rotationMatrix);
                    transform->SetQTRotation(transform->GetQTRotation() * deltaRotation);
                    lastRotation.y = tempRotation.y;
                    currentEulerAngles.y = tempRotation.y;
                }

                // Z�� ȸ��
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
            // ������ ������Ʈ�� ǥ��
            const vector<shared_ptr<Component>>& components = _selectedObject->GetComponents();
            for (const auto& component : components)
            {
                // Transform�� �̹� ǥ�������Ƿ� ��ŵ
                if (dynamic_pointer_cast<Transform>(component))
                    continue;

                // ������Ʈ Ÿ�Կ� ���� �̸� ����
                string componentName;
                if (dynamic_pointer_cast<MeshRenderer>(component))
                    componentName = "Mesh Renderer";
                else if (dynamic_pointer_cast<Camera>(component))
                    componentName = "Camera";
                else if (dynamic_pointer_cast<Light>(component))
                    componentName = "Light";
                else if (dynamic_pointer_cast<BaseCollider>(component))
                    componentName = "Collider";
                // ... �ٸ� ������Ʈ Ÿ�Ե� �߰� ...

                if (!componentName.empty() && ImGui::CollapsingHeader(componentName.c_str()))
                {
                    // �� ������Ʈ Ÿ�Ժ� �Ӽ� ǥ��
                    // TODO: ������Ʈ�� ���� �Ӽ� ǥ�� ���� �߰�
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
            
            // Frustum ����
            BoundingFrustum frustum;
            BoundingFrustum::CreateFromMatrix(frustum, proj);

            // View ������ Frustum�� ���� �������� ��ȯ
            Matrix invView = view.Invert();
            frustum.Transform(frustum, invView);

            ImGuizmo::SetRect(viewportX, viewportY, viewportWidth, viewportHeight);
            ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

            // CŰ�� �ݶ��̴� ���� ��� ���
            if (INPUT.GetButtonDown(KEY_TYPE::C))
            {
                _isColliderEditMode = !_isColliderEditMode;
                // �ݶ��̴� ����� ���� ����(�ʷϻ� �迭)
                ImGuizmo::SetGizmoSizeClipSpace(0.08f);  // ����� ũ�⸦ �� �� ũ��
                ImGuizmo::AllowAxisFlip(false);  // �� ���� ��Ȱ��ȭ

                // ����� ���� Ŀ���͸���¡
                ImGuizmo::Style& style = ImGuizmo::GetStyle();
                style.Colors[ImGuizmo::TRANSLATE_X] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // X�� �ʷϻ�
                style.Colors[ImGuizmo::TRANSLATE_Y] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Y�� �ʷϻ�
                style.Colors[ImGuizmo::TRANSLATE_Z] = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Z�� �ʷϻ�
                style.Colors[ImGuizmo::BOUNDS] = ImVec4(0.0f, 0.6f, 0.0f, 0.3f);  // ��� �ʷϻ� (������)
            }
                

            // Transform Gizmo ���� (�ݶ��̴� ���� ��尡 �ƴ� ����)
            if (!_isColliderEditMode)
            {
                // �Ϲ� Transform ����� ��Ÿ�� ����
                ImGuizmo::SetGizmoSizeClipSpace(0.14f);  // �⺻ ũ��
                ImGuizmo::AllowAxisFlip(true);

                // �⺻ �������� ����
                ImGuizmo::Style& style = ImGuizmo::GetStyle();
                style.Colors[ImGuizmo::TRANSLATE_X] = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);  // X�� ������
                style.Colors[ImGuizmo::TRANSLATE_Y] = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);  // Y�� �ʷϻ�
                style.Colors[ImGuizmo::TRANSLATE_Z] = ImVec4(0.2f, 0.2f, 0.9f, 1.0f);  // Z�� �Ķ���

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
                            // �θ� �ִ� ��� ���� ��ȯ�� ���� ��ȯ���� ��ȯ
                            Matrix parentWorldMatrix = pickedObj->transform()->GetParent()->GetWorldMatrix();
                            Matrix parentWorldInverse = parentWorldMatrix.Invert();

                            if (currentGizmoOperation == ImGuizmo::TRANSLATE)
                            {
                                // �̵�: ���� ��ġ�� �θ� ���� �������� ��ȯ
                                Vec3 localPos = Vec3::Transform(position, parentWorldInverse);
                                pickedObj->transform()->SetLocalPosition(localPos);
                            }
                            else if (currentGizmoOperation == ImGuizmo::SCALE)
                            {
                                // ������: �θ��� �������� ����Ͽ� ���� ������ ���
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
                            // �θ� ���� ��� ���� ����
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

            // Collider �ð�ȭ �� Gizmo
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
                        // Collider Gizmo (�ݶ��̴� ���� ����� ����)
                        if (_isColliderEditMode)
                        {
                            // ������Ʈ�� ���� Transform ���� ��������
                            Vec3 worldPos = pickedObj->transform()->GetWorldPosition();
                            Quaternion worldRot = pickedObj->transform()->GetQTRotation();

                            // ���� �ݶ��̴��� ���� ũ�⸦ ������
                            Vec3 currentScale = boxCollider->GetScale();
                            Vec3 objectScale = pickedObj->transform()->GetWorldScale();

                            // Collider Matrix ����
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
                                // ������Ʈ�� ���� �������� ����Ͽ� �ݶ��̴� ������ ����
                                boxCollider->SetScale(scale / objectScale);
                            }
                        }


                        // ���̾������� ������
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

                        // �ڽ� ���� �׸���
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

                        // ���� �ݶ��̴��� ���� ũ�⸦ ������
                        Vec3 currentScale = sphereCollider->GetScale();
                        Vec3 objectScale = pickedObj->transform()->GetWorldScale();

                        // Collider Matrix ����
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
                            // ������Ʈ�� ���� �������� ����Ͽ� �ݶ��̴� ������ ����
                            sphereCollider->SetScale(scale / objectScale);
                        }
                    }
                    

                    ImGui::PopID();

                    // ���̾������� ������
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
