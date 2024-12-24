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
	SetTransitionFlag(GetClip("Clip1")->transition, GP.test);
	SetTransitionFlag(GetClip("Clip2")->transition, GP.test2);
}

void Animator::AddClip(const string& name, int animIndex, bool isLoop)
{
	shared_ptr<Clip> clip = make_shared<Clip>();
	clip->name = name;
	clip->animIndex = animIndex;
	clip->isLoop = isLoop;

	_clips[name] = clip;
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

		clipA->transition = transition;
		_transitions.push_back(transition);
	}
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
			param->value.boolValue = value;
	}
}

void Animator::SetInt(const string& name, int value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Int)
			param->value.intValue = value;
	}
}

void Animator::SetFloat(const string& name, float value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Float)
			param->value.floatValue = value;
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

void Animator::CheckConditionsAndSetFlag(shared_ptr<Transition> transition)
{
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
		transition->flag = true;
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