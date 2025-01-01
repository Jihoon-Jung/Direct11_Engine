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

	// �̸��� �̹� �����ϴ� ��� ó��
	if (_clips.find(uniqueName) != _clips.end())
	{
		// �⺻ �̸��� ���� �κ� �и�
		size_t underscorePos = name.rfind('_');
		string baseName = name;
		int number = 1;

		// �̹� _���� ������ �ִ��� Ȯ��
		if (underscorePos != string::npos)
		{
			string numberStr = name.substr(underscorePos + 1);
			// ���ڷθ� �̷���� �ִ��� Ȯ��
			bool isNumber = !numberStr.empty() &&
				std::all_of(numberStr.begin(), numberStr.end(), ::isdigit);

			if (isNumber)
			{
				baseName = name.substr(0, underscorePos);
				number = std::stoi(numberStr) + 1;
			}
		}

		// ��� ������ �̸��� ã�� ������ �ݺ�
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

	// �⺻�� ����
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

			// �Ķ���� �� ��������
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

			// �ϳ��� �����̶� �������� ������ ��ü ������ ����
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
	// Animator�� �޷��ִ� GameObject ���
	auto owner = GetGameObject();
	if (!owner)
		return;

	// 1) owner->GetComponents() ��ȸ
	for (auto& comp : owner->GetComponents())
	{
		// 2) MonoBehaviour* ĳ����
		MonoBehaviour* mb = dynamic_cast<MonoBehaviour*>(comp.get());
		if (mb == nullptr)
			continue;

		// 3) ���� MonoBehaviour�� C++ RTTI �̸�
		std::string className = typeid(*mb).name();
		// 4) ������Ʈ������ ��ȸ�� Ű�� �����: "MoveObject::TestLog"
		std::string key = className + "::" + functionName;

		// 5) ���� �Լ� ȣ�� ���ٸ� ���� ��
		auto method = MR.getMethod(key);
		if (method)
		{
			// 6) �Լ� ȣ��
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
		// �Ķ���Ϳ� ���õ� ��� Ʈ�������� ���ǵ鵵 ����
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

			// ������ ��� ���ŵǾ��ٸ� hasCondition �÷��� ������Ʈ
			if (transition->conditions.empty())
				transition->hasCondition = false;
		}

		// �Ķ���� ����
		_parameters.erase(it);
	}
}

void Animator::SetClipCurrentTransition(shared_ptr<Clip> clip)
{
	if (!clip)
		return;

	// 1. hasCondition�� true�� Ʈ�����ǵ� ã��
	vector<shared_ptr<Transition>> conditionTransitions;
	for (const auto& transition : clip->transitions)
	{
		if (transition->hasCondition)
			conditionTransitions.push_back(transition);
	}

	// ������ �ִ� Ʈ�������� �����ϴ� ���
	if (!conditionTransitions.empty())
	{
		if (conditionTransitions.size() > 1)
		{
			// HasExitTime�� false�� Ʈ�����ǵ� ã��
			vector<shared_ptr<Transition>> noExitTransitions;
			for (const auto& transition : conditionTransitions)
			{
				if (!transition->hasExitTime)
					noExitTransitions.push_back(transition);
			}

			if (!noExitTransitions.empty())
			{
				// HasExitTime�� false�� Ʈ������ �� flag�� true�� �͵� ã��
				vector<shared_ptr<Transition>> flaggedTransitions;
				for (const auto& transition : noExitTransitions)
				{
					if (transition->flag)
						flaggedTransitions.push_back(transition);
				}

				if (flaggedTransitions.size() > 1)
				{
					clip->transition = flaggedTransitions[0]; // ���� ���� �� ���
				}
				else if (flaggedTransitions.size() == 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else
				{
					clip->transition = nullptr;  // flag�� true�� ���� ����
				}
			}
			else
			{
				// HasExitTime�� true�� Ʈ�����ǵ� �� flag�� true�� �͵� ã��
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
			// Ʈ�������� 1���� ��� flag�� true�� ���� ����
			if (conditionTransitions[0]->flag)
				clip->transition = conditionTransitions[0];
			else
				clip->transition = nullptr;
		}
	}
	else
	{
		// ������ ���� Ʈ�����ǵ� �� hasExitTime�� true�� �͵� ã��
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