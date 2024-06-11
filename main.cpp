// Example Component Implementation
#include <iostream>
#include <string>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <vector>
#include <memory>

#define DBGSTR(expr)  std::cout << #expr << " = " << expr << std::endl << std::endl;
#define DBGEXPR(expr)  std::cout << #expr << " = " << std::to_string(expr) << std::endl << std::endl;

template<typename T, typename... Ts>
struct is_in_pack : std::false_type {};

template<typename T, typename U, typename... Ts>
struct is_in_pack<T, U, Ts...> : std::conditional_t<std::is_same_v<T, U>, std::true_type, is_in_pack<T, Ts...>> {};

class IComponent
{
public:
    virtual ~IComponent() = default;
};

template<class T>
class ComponentImpl : public IComponent
{
public:
    template<class... TArgs>
    explicit ComponentImpl(TArgs... args) :
            _t{ args... }
    {
    }

    T& get() { return _t; }

private:
    T _t;
};

class RuntimeComponentContainer
{
protected:
    template<class T, class... TArgs>
    void AddComponentToMap(TArgs... args)
    {
        _map[typeid(T)] = std::make_shared<T>(args...);
    }

    template<class T>
    void RemoveComponentFromMap()
    {
        _map.erase(typeid(T));
    }
    
    template<class T>
    T& ReturnComponent()
    {
        const auto& ptr = _map[typeid(T)].get();
        if (ptr) return *reinterpret_cast<T*>(ptr);
        throw std::runtime_error("Failed to find component: " + std::string(typeid(T).name()));
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _map{};
};

template<class T>
class ComponentProvider
{
public:
    static_assert(std::is_base_of_v<IComponent, T>, "T must be an IComponent");
    T& ReturnComponent() { return _t; }
private:
    T _t{};
};

template<class... TStaticComponents>
class ComponentContainer : private RuntimeComponentContainer, private ComponentProvider<TStaticComponents>...
{
public:

    template<class T, class... TArgs>
    void AddComponent(TArgs... args)
    {
        static_assert(!is_in_pack<T, TStaticComponents...>{}.value, "Cannot add static component to container!");
        AddComponentToMap<T>(args...);
    }

    template<class T>
    void RemoveComponent()
    {
        static_assert(!is_in_pack<T, TStaticComponents...>{}.value, "Cannot add static component to container!");
        RemoveComponentFromMap<T>();
    }

    template<class T>
    T& GetComponent()
    {
        if constexpr (is_in_pack<T, TStaticComponents...>{}.value)
        {
            return ComponentProvider<T>::ReturnComponent();
        }
        else 
        {
            return RuntimeComponentContainer::ReturnComponent<T>();
        }
    }

    template<class TFirst, class TSecond, class... TRest>
    decltype(auto) GetComponent()
    {
        return GetComponent<TFirst>().template GetComponent<TSecond, TRest...>();
    }
};

template<class T>
using VectorComponent = ComponentImpl<std::vector<T>>;
using IntComponent = ComponentImpl<int>;
using StringComponent = ComponentImpl<std::string>;

class Entity : public ComponentContainer<IntComponent>
{
};

int main()
{
    Entity ent{};

    // IntComponent is a static component, we don't need to add it to access it
    // ent.AddComponent<IntComponent>(); // this line won't compile since IntComponent is statically defined in Entity
    auto& i = ent.GetComponent<IntComponent>();
    i.get() = 100;
    DBGEXPR(ent.GetComponent<IntComponent>().get())

    // StringComponent is dynamically defined and accessed
    ent.AddComponent<StringComponent>(); // Requires adding before usage
    auto& str = ent.GetComponent<StringComponent>().get();
    str = "Hello, World!";
    DBGSTR(ent.GetComponent<StringComponent>().get())

    // Once StringComponent is removed, calling GetComponent<StringComponent> throws an error
    ent.RemoveComponent<StringComponent>();
    try
    {
        str = ent.GetComponent<StringComponent>().get();
        DBGSTR(str)
    }
    catch (const std::exception& e)
    {
        std::cout << "Could not unwrap StringComponent\n e.what() = " << e.what() << std::endl << std::endl;
    }

    // Construct from the AddComponent call
    ent.AddComponent<VectorComponent<int>>(1, 2, 3, 4, 5, 4, 3, 2, 1);
    auto& vec = ent.GetComponent<VectorComponent<int>>().get();
    std::cout << "ent.GetComponent<VectorComponent<int>>().get() = ";
    for (const auto& j : vec) std::cout << std::to_string(j) << ", ";
    std::cout << std::endl;

    return 0;
}
