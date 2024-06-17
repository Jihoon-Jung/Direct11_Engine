#include "pch.h"
#include "ModelAnimation.h"

std::shared_ptr<ModelKeyframe> ModelAnimation::GetKeyframe(const wstring& name)
{
	auto findIt = keyframes.find(name);
	if (findIt == keyframes.end())
		return nullptr;

	return findIt->second;
}