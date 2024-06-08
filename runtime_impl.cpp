// Example Component Implementation
#include <iostream>
#include <string>
#include <unordered_map>
#include <typeindex>

#define DBGEXPR(expr)  std::cout << #expr << " = " << std::to_string(expr) << std::endl;

class IComponent
{
public:
    virtual ~IComponent() = default;
};

class RuntimeComponentContainer
{
public:
    template<class T>
    void AddComponent()
    {
        _map[typeid(T)] = std::make_shared<T>();
    }

    template<class T>
    T& GetComponent()
    {
        const auto& ptr = _map[typeid(T)].get();
        if (ptr) return *dynamic_cast<T*>(ptr);
        throw std::runtime_error("Failed to find component: " + GetComponentName<T>());
    }

    template<class T>
    void RemoveComponent()
    {
        _map.erase(typeid(T));
    }

    template<typename T>
    std::string GetComponentName()
    {
        return typeid(T).name();
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _map{};
};

class IntComponent : public IComponent
{
public:
    int& get_int() { return _i; }
private:
    int _i{};
};

class Entity : public RuntimeComponentContainer
{
};

int main()
{
    Entity ent{};

    ent.AddComponent<IntComponent>();
    auto& icmp = ent.GetComponent<IntComponent>();
    icmp.get_int() = 100;

    const auto& i = ent.GetComponent<IntComponent>().get_int();
    DBGEXPR(i)

    ent.RemoveComponent<IntComponent>();

    try
    {
        const auto& j = ent.GetComponent<IntComponent>().get_int();
        DBGEXPR(j)
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
