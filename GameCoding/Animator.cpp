#include "pch.h"
#include "Animator.h"

Animator::Animator()
	:Super(ComponentType::Animator)
{

}

Animator::~Animator()
{
}

void Animator::Start()
{
	/*AddClip("Clip1", 2, false);
	AddClip("Clip2", 0, false);
	AddClip("Clip3", 1, false);

	SetEntryClip("Clip1");

	AddTransition("Clip1", "Clip2");
	AddTransition("Clip2", "Clip3");
	AddTransition("Clip3", "Clip1");
	SetTransitionFlag(GetClip("Clip1")->transition, false);
	SetTransitionFlag(GetClip("Clip2")->transition, false);

	SetCurrentTransition();*/
}

void Animator::Update()
{
	/*SetTransitionFlag(GetClip("Clip1")->transition, GP.test);
	SetTransitionFlag(GetClip("Clip2")->transition, GP.test2);*/
}

void Animator::AddClip(const string& name, int animIndex, bool isLoop)
{
	string uniqueName = name;

	// 이름이 이미 존재하는 경우 처리
	if (_clips.find(uniqueName) != _clips.end())
	{
		// 기본 이름과 숫자 부분 분리
		size_t underscorePos = name.rfind('_');
		string baseName = name;
		int number = 1;

		// 이미 _숫자 형식이 있는지 확인
		if (underscorePos != string::npos)
		{
			string numberStr = name.substr(underscorePos + 1);
			// 숫자로만 이루어져 있는지 확인
			bool isNumber = !numberStr.empty() &&
				std::all_of(numberStr.begin(), numberStr.end(), ::isdigit);

			if (isNumber)
			{
				baseName = name.substr(0, underscorePos);
				number = std::stoi(numberStr) + 1;
			}
		}

		// 사용 가능한 이름을 찾을 때까지 반복
		do {
			uniqueName = baseName + "_" + std::to_string(number);
			number++;
		} while (_clips.find(uniqueName) != _clips.end());
	}

	shared_ptr<Clip> clip = make_shared<Clip>();
	clip->name = uniqueName;
	clip->animIndex = animIndex;
	clip->isLoop = isLoop;

	_clips[uniqueName] = clip;
}

void Animator::AddTransition(const string& clipAName, const string& clipBName)
{
	shared_ptr<Clip> clipA = GetClip(clipAName);
	shared_ptr<Clip> clipB = GetClip(clipBName);

	if (clipA && clipB)
	{
		shared_ptr<Transition> transition = make_shared<Transition>();
		transition->clipA = clipA;
		transition->clipB = clipB;
		transition->flag = false;
		transition->hasExitTime = true;
		transition->hasCondition = false;

		//clipA->transition = transition;
		clipA->transitions.push_back(transition);
		if (_currClip->name == clipAName || _currClip->name == clipBName)
			SetCurrentTransition();

		_transitions.push_back(transition);
	}
	SetClipCurrentTransition(clipA);
}
void Animator::SetTransitionFlag(shared_ptr<Transition> transition, bool flag)
{
	transition->flag = flag;
	transition->hasCondition = true;
}
void Animator::SetEntryClip(const string& clipName)
{
	_entry = GetClip(clipName);
	SetCurrentClip(clipName);
}

void Animator::SetCurrentTransition()
{
	_currTransition = _currClip->transition;
}

void Animator::SetTransitionHasExit(shared_ptr<Transition> transition, bool flag)
{
	transition->hasExitTime = flag;
}

void Animator::SetTransitionExitTime(shared_ptr<Transition> transition, float exitTime)
{
	transition->exitTime = exitTime;
}

void Animator::SetTransitionOffset(shared_ptr<Transition> transition, float offset)
{
	transition->transitionOffset = offset;
}

void Animator::SetTransitionDuration(shared_ptr<Transition> transition, float duration)
{
	transition->transitionDuration = duration;
}

void Animator::AddParameter(const string& name, Parameter::Type type)
{
	Parameter param;
	param.name = name;
	param.type = type;

	// 기본값 설정
	switch (type)
	{
	case Parameter::Type::Bool:
		param.value.boolValue = false;
		break;
	case Parameter::Type::Int:
		param.value.intValue = 0;
		break;
	case Parameter::Type::Float:
		param.value.floatValue = 0.0f;
		break;
	}

	_parameters.push_back(param);
}

Parameter* Animator::GetParameter(const string& name)
{
	auto it = std::find_if(_parameters.begin(), _parameters.end(),
		[&name](const Parameter& param) { return param.name == name; });

	return it != _parameters.end() ? &(*it) : nullptr;
}

shared_ptr<Clip> Animator::GetClip(const string& name)
{
    auto it = _clips.find(name);
    if (it != _clips.end())
        return it->second;
    return nullptr;
}
void Animator::SetBool(const string& name, bool value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Bool)
		{
			param->value.boolValue = value;
			CheckConditionsAndSetFlag();
		}
			
	}
}

void Animator::SetInt(const string& name, int value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Int)
		{
			param->value.intValue = value;
			CheckConditionsAndSetFlag();
		}
	}
}

void Animator::SetFloat(const string& name, float value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Float)
		{
			param->value.floatValue = value;
			CheckConditionsAndSetFlag();
		}
	}
}

bool Animator::GetBool(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Bool)
			return param->value.boolValue;
	}
	return false;
}

void Animator::CheckConditionsAndSetFlag()
{
	for (shared_ptr<Transition> transition : _transitions)
	{
		bool prevFlag = transition->flag;
		bool isAllConditionSatisfy = true;

		for (const Condition& condition : transition->conditions)
		{
			bool conditionSatisfied = false;

			// 파라미터 값 가져오기
			switch (condition.parameterType)
			{
			case Parameter::Type::Bool:
			{
				bool paramValue = GetBool(condition.parameterName);
				bool conditionValue = condition.value.boolValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (paramValue == conditionValue);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (paramValue != conditionValue);
					break;
				}
				break;
			}
			case Parameter::Type::Int:
			{
				int paramValue = GetInt(condition.parameterName);
				int conditionValue = condition.value.intValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (paramValue == conditionValue);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (paramValue != conditionValue);
					break;
				case Condition::CompareType::Greater:
					conditionSatisfied = (paramValue > conditionValue);
					break;
				case Condition::CompareType::Less:
					conditionSatisfied = (paramValue < conditionValue);
					break;
				}
				break;
			}
			case Parameter::Type::Float:
			{
				float paramValue = GetFloat(condition.parameterName);
				float conditionValue = condition.value.floatValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (abs(paramValue - conditionValue) < 0.0001f);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (abs(paramValue - conditionValue) >= 0.0001f);
					break;
				case Condition::CompareType::Greater:
					conditionSatisfied = (paramValue > conditionValue);
					break;
				case Condition::CompareType::Less:
					conditionSatisfied = (paramValue < conditionValue);
					break;
				}
				break;
			}
			}

			// 하나의 조건이라도 만족하지 않으면 전체 조건은 실패
			if (!conditionSatisfied)
			{
				isAllConditionSatisfy = false;
				break;
			}
		}

		if (isAllConditionSatisfy)
		{
			transition->flag = true;
		}
		else
			transition->flag = false;

		SetClipCurrentTransition(transition->clipA.lock());
		
		if (prevFlag != transition->flag)
		{
			if (auto clipA = transition->clipA.lock())
				if (auto clipB = transition->clipB.lock())
					SCENE.UpdateAnimatorTransitionFlagInXML(
						SCENE.GetActiveScene()->GetSceneName(),
						GetGameObject()->GetName(),
						clipA->name, clipB->name,
						transition->flag, transition->hasCondition);
		}
	}
}

void Animator::InvokeAnimationEvent(const std::string& functionName)
{
	// Animator가 달려있는 GameObject 취득
	auto owner = GetGameObject();
	if (!owner)
		return;

	// 1) owner->GetComponents() 순회
	for (auto& comp : owner->GetComponents())
	{
		// 2) MonoBehaviour* 캐스팅
		MonoBehaviour* mb = dynamic_cast<MonoBehaviour*>(comp.get());
		if (mb == nullptr)
			continue;

		// 3) 현재 MonoBehaviour의 C++ RTTI 이름
		std::string className = typeid(*mb).name();
		// 4) 레지스트리에서 조회할 키를 만든다: "MoveObject::TestLog"
		std::string key = className + "::" + functionName;

		// 5) 실제 함수 호출 람다를 구해 옴
		auto method = MR.getMethod(key);
		if (method)
		{
			// 6) 함수 호출
			method(mb);
		}
	}
}

int Animator::GetInt(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Int)
			return param->value.intValue;
	}
	return 0;
}

float Animator::GetFloat(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Float)
			return param->value.floatValue;
	}
	return 0.0f;
}

void Animator::AddCondition(shared_ptr<Transition> transition, const string& paramName,
	Parameter::Type paramType, Condition::CompareType compareType)
{
	if (!transition)
		return;

	Condition newCondition;
	newCondition.parameterName = paramName;
	newCondition.parameterType = paramType;
	newCondition.compareType = compareType;
	transition->conditions.push_back(newCondition);
	transition->hasCondition = true;

}

void Animator::RemoveCondition(shared_ptr<Transition> transition, int index)
{
	if (!transition || index < 0 || index >= transition->conditions.size())
		return;

	transition->conditions.erase(transition->conditions.begin() + index);

	if (transition->conditions.size() == 0)
		transition->hasCondition = false;
}

void Animator::RemoveParameter(const string& name)
{
	auto it = std::find_if(_parameters.begin(), _parameters.end(),
		[&name](const Parameter& param) { return param.name == name; });

	if (it != _parameters.end())
	{
		// 파라미터와 관련된 모든 트랜지션의 조건들도 제거
		for (auto& transition : _transitions)
		{
			transition->conditions.erase(
				std::remove_if(transition->conditions.begin(), transition->conditions.end(),
					[&name](const Condition& condition) {
						return condition.parameterName == name;
					}
				),
				transition->conditions.end()
						);

			// 조건이 모두 제거되었다면 hasCondition 플래그 업데이트
			if (transition->conditions.empty())
				transition->hasCondition = false;
		}

		// 파라미터 제거
		_parameters.erase(it);
	}
}

void Animator::SetClipCurrentTransition(shared_ptr<Clip> clip)
{
	if (!clip)
		return;

	// 1. hasCondition이 true인 트랜지션들 찾기
	vector<shared_ptr<Transition>> conditionTransitions;
	for (const auto& transition : clip->transitions)
	{
		if (transition->hasCondition)
			conditionTransitions.push_back(transition);
	}

	// 조건이 있는 트랜지션이 존재하는 경우
	if (!conditionTransitions.empty())
	{
		if (conditionTransitions.size() > 1)
		{
			// HasExitTime이 false인 트랜지션들 찾기
			vector<shared_ptr<Transition>> noExitTransitions;
			for (const auto& transition : conditionTransitions)
			{
				if (!transition->hasExitTime)
					noExitTransitions.push_back(transition);
			}

			if (!noExitTransitions.empty())
			{
				// HasExitTime이 false인 트랜지션 중 flag가 true인 것들 찾기
				vector<shared_ptr<Transition>> flaggedTransitions;
				for (const auto& transition : noExitTransitions)
				{
					if (transition->flag)
						flaggedTransitions.push_back(transition);
				}

				if (flaggedTransitions.size() > 1)
				{
					clip->transition = flaggedTransitions[0]; // 먼저 넣은 것 사용
				}
				else if (flaggedTransitions.size() == 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else
				{
					clip->transition = nullptr;  // flag가 true인 것이 없음
				}
			}
			else
			{
				// HasExitTime이 true인 트랜지션들 중 flag가 true인 것들 찾기
				vector<shared_ptr<Transition>> flaggedTransitions;
				for (const auto& transition : conditionTransitions)
				{
					if (transition->flag)
						flaggedTransitions.push_back(transition);
				}

				if (flaggedTransitions.size() > 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else if (flaggedTransitions.size() == 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else
				{
					clip->transition = nullptr;
				}
			}
		}
		else
		{
			// 트랜지션이 1개인 경우 flag가 true일 때만 설정
			if (conditionTransitions[0]->flag)
				clip->transition = conditionTransitions[0];
			else
				clip->transition = nullptr;
		}
	}
	else
	{
		// 조건이 없는 트랜지션들 중 hasExitTime이 true인 것들 찾기
		vector<shared_ptr<Transition>> exitTimeTransitions;
		for (const auto& transition : clip->transitions)
		{
			if (transition->hasExitTime)
				exitTimeTransitions.push_back(transition);
		}

		if (exitTimeTransitions.size() > 1)
		{
			clip->transition = exitTimeTransitions[0];
		}
		else if (exitTimeTransitions.size() == 1)
		{
			clip->transition = exitTimeTransitions[0];
		}
		else
		{
			clip->transition = nullptr;
		}
	}
}