#pragma once
#include "MethodRegistry.h"
#include <typeinfo>
#include <string>

#define REGISTER_MONOBEHAVIOR_METHOD(ClassType, MethodName) \
    static bool s_register_##ClassType##_##MethodName = [](){ \
        std::string className = typeid(ClassType).name(); \
        std::string key = className + "::" + #MethodName; \
        MR.registerMethod(key, \
            [](MonoBehaviour* obj){ \
                auto derived = static_cast<ClassType*>(obj); \
                derived->MethodName(); \
            } \
        ); \
        return true; \
    }();
