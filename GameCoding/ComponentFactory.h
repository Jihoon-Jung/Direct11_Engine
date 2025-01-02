#pragma once
class MonoBehaviour;

class ComponentFactory {
protected:
    ComponentFactory() {}
public:
    static ComponentFactory& GetInstance() {
        static ComponentFactory instance;
        return instance;
    }

    using CreateScriptFunc = std::function<shared_ptr<MonoBehaviour>()>;
    struct ScriptInfo {
        string displayName;
        CreateScriptFunc createFunc;
    };

    void RegisterScript(const string& typeName, const ScriptInfo& info) {
        _scriptTypes[typeName] = info;
    }

    const map<string, ScriptInfo>& GetRegisteredScripts() const { return _scriptTypes; }

private:
    map<string, ScriptInfo> _scriptTypes;
};

#define REGISTER_SCRIPT(Type, DisplayName) \
    namespace { \
        struct Type##Registrar { \
            Type##Registrar() { \
                ComponentFactory::ScriptInfo info; \
                info.displayName = DisplayName; \
                info.createFunc = []() { return make_shared<Type>(); }; \
                ComponentFactory::GetInstance().RegisterScript(#Type, info); \
            } \
        }; \
        static Type##Registrar Type##AutoRegister; \
    }