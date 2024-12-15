#pragma once
#include <filesystem>

class GUIManager
{
protected:
	GUIManager() {}
	virtual ~GUIManager() {}
public:
	GUIManager(const GUIManager&) = delete;
	static GUIManager& GetInstance()
	{
		static GUIManager instance;
		return instance;
	}
	void Init();
	void Update();
	void RenderUI_Start();
	void RenderUI();
	void RenderUI_End();
	void RenderGuizmo();
	void RenderGameObjectHierarchy(shared_ptr<GameObject> gameObject);
	
	void RenderFolderTree(const filesystem::path& path, filesystem::path& selectedFolder);
	void RenderFileGrid(const filesystem::path& path);

private:
	shared_ptr<GameObject> _selectedObject = nullptr;
	bool _isColliderEditMode = false;  // �ݶ��̴� ���� ������� ����
	bool _isCameraMoving = false;
	Vec3 _cameraStartPos;
	Vec3 _cameraTargetPos;
	Vec3 _cameraStartRot;
	Vec3 _cameraTargetRot;
	float _cameraMoveTime = 0.0f;
	const float _cameraMoveSpeed = 3.0f;  // �̵� �ӵ� (���� Ŭ���� ����)

private:
	// EmptyObject ���� ���� ������
	bool _showEmptyObjectPopup = false;
	char _newObjectName[128] = "NewObject";
	Vec3 _newObjectPosition = Vec3::Zero;
	Vec3 _newObjectRotation = Vec3::Zero;
	Vec3 _newObjectScale = Vec3(1.0f, 1.0f, 1.0f);
	bool _newObjectUseMeshRenderer = false;

	// Resource ����Ʈ ���� ������
	bool _resourceListInitialized = false;
	vector<wstring> _meshList;
	vector<wstring> _materialList;
	int _newObjectSelectedMesh = 0;
	int _newObjectSelectedMaterial = 0;

	// ���� ȸ�� ���� ���� ����
	enum RotationAxis
	{
		NONE = -1,
		X_AXIS = 0,
		Y_AXIS = 1,
		Z_AXIS = 2
	};
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentGizmoMode = ImGuizmo::LOCAL;
	RotationAxis currentRotationAxis = NONE;

	bool _rotationUpdated = false;
	filesystem::path _selectedFolder;
	bool _isFirstFrame = true;  // ù ������ üũ�� ���� ����

};

