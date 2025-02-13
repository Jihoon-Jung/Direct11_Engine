#pragma once

#include "Model.h"
struct Clip;
struct Transition;
class MonoBehaviour;


struct AvailableFunction
{
	MonoBehaviour* script;
	string functionKey;
};

struct Parameter
{
	enum class Type { Bool, Int, Float };
	string name;
	Type type;

	union Value
	{
		bool boolValue;
		int intValue;
		float floatValue;

		Value() : boolValue(false) {} // 기본 생성자
	} value;
};

struct Condition
{
	enum class CompareType { Equals, NotEqual, Greater, Less };
	string parameterName;
	Parameter::Type parameterType;
	CompareType compareType;

	union Value
	{
		bool boolValue;
		int intValue;
		float floatValue;
		Value() : boolValue(false) {}
	} value;
};

struct Transition
{
	weak_ptr<Clip> clipA;
	weak_ptr<Clip> clipB;
	bool flag = false;
	bool hasCondition = false;
	bool hasExitTime = false;
	float exitTime = 1.0f;
	float transitionOffset = 0.0f;
	float transitionDuration = 0.3f;
	vector<Condition> conditions;
};
struct AnimationEvent
{
	float time;              // 이벤트 발생 시간
	AvailableFunction function;     // 호출할 함수 이름
	bool isFuctionCalled = false;
	// (선택) std::string paramStr; 등 파라미터도 보낼 수 있음
};

struct Clip
{
	string name;
	bool isLoop = false;
	bool isEndFrame = false;
	int animIndex;
	float speed = 1.0f;
	float progressRatio = 0.0f;  // 0.0f ~ 1.0f 사이의 재생 진행률
	ImVec2 pos;  // Animator편집기의 노드 위치
	shared_ptr<Transition> transition;
	vector <shared_ptr<Transition>> transitions;
	vector<AnimationEvent> events;

};



class Animator : public Component
{
	using Super = Component;
public:
	Animator();
	virtual ~Animator();
	virtual void Update() override;
	virtual void Start() override;

	void AddClip(const string& name, int animIndex, bool isLoop = false);
	void AddTransition(const string& clipAName, const string& clipBName);
	void SetTransitionFlag(shared_ptr<Transition> transition, bool flag);
	void SetTransitionHasExit(shared_ptr<Transition> transition, bool flag);
	void SetTransitionExitTime(shared_ptr<Transition> transition, float exitTime);
	void SetTransitionOffset(shared_ptr<Transition> transition, float offset);
	void SetTransitionDuration(shared_ptr<Transition> transition, float duration);
	void SetEntryClip(const string& clipName);
	void SetCurrentClip(const string& name) { _currClip = GetClip(name); }
	void SetCurrentTransition();
	void RemoveParameter(const string& name);
	void CheckConditionsAndSetFlag();

	void UpdateBoneTransforms();

	void InvokeAnimationEvent(const std::string& functionName);
	void InvokeAnimationEvent(const AvailableFunction& function);

	void AddCondition(shared_ptr<Transition> transition, const string& paramName,
		Parameter::Type paramType, Condition::CompareType compareType);
	void RemoveCondition(shared_ptr<Transition> transition, int index);

	shared_ptr<Clip> GetClip(const string& name);

	void SetClipCurrentTransition(shared_ptr<Clip> clip);

	vector<AvailableFunction> GetAvailableFunctions();

	void HandleTransitionBlend(shared_ptr<Transition>& transition);
	BlendAnimDesc GetBlendAnimDesc() { return _blendAnimDesc; }

	shared_ptr<Clip> _entry;
	shared_ptr<Clip> _currClip;
	shared_ptr<Transition> _currTransition;
	map<string, shared_ptr<Clip>> _clips;
	vector<shared_ptr<Transition>> _transitions;

public:
	void AddParameter(const string& name, Parameter::Type type);
	Parameter* GetParameter(const string& name);
	vector<Parameter> _parameters;

	// 파라미터 값 설정 메서드들
	void SetBool(const string& name, bool value);
	void SetInt(const string& name, int value);
	void SetFloat(const string& name, float value);

	// 파라미터 값 가져오기 메서드들
	bool GetBool(const string& name);
	int GetInt(const string& name);
	float GetFloat(const string& name);
	void SetBoneObjects(const map<int32, shared_ptr<GameObject>>& boneObjects) { _boneObjects = boneObjects; }

	shared_ptr<GameObject> FindBoneObjectByIndex(int index);

private:

	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;
	shared_ptr<Model> _model;

	map<int32, shared_ptr<GameObject>> _boneObjects;
};

