// MethodRegistry.h
#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

// 전방 선언
class MonoBehaviour;

class MethodRegistry
{
protected:
    MethodRegistry() {}
    virtual ~MethodRegistry() {}
public:
    using MethodType = std::function<void(MonoBehaviour*)>;

    MethodRegistry(const MethodRegistry&) = delete;
    static MethodRegistry& GetInstance()
    {
        static MethodRegistry instance;
        return instance;
    }

    void registerMethod(const std::string& key, MethodType method) {
        _methods[key] = method;
    }

    MethodType getMethod(const std::string& key) {
        auto it = _methods.find(key);
        if (it != _methods.end()) {
            return it->second;
        }
        return nullptr;
    }
    const std::unordered_map<std::string, MethodType>& GetAllMethods() const {
        return _methods;
    }
private:
    std::unordered_map<std::string, MethodType> _methods;
};
