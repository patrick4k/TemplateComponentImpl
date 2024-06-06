// Example Component Implementation
#include <iostream>
#include <string>
 
template<class T>
class ComponentProvider
{
public:
    T& ReturnComponent() { return _t; }
private:
    T _t{};
};
 
template<class... TComponents>
class ComponentContainer : private ComponentProvider<TComponents>...
{
public:
    template<class T>
    T& GetComponent()
    {
        return ComponentProvider<T>::ReturnComponent();
    }
    
    template<class TFirst, class TSecond, class... TRest>
    decltype(auto) GetComponent()
    {
        return GetComponent<TFirst>().template GetComponent<TSecond, TRest...>();
    }
};
 
class SubComponent: public ComponentContainer<int>
{
};

class SubSubComponent: public ComponentContainer<SubComponent>
{
};
 
class Entity: public ComponentContainer<int, std::string, SubComponent, SubSubComponent>
{   
};
 
int main(void)
{
    Entity ent{};
    
    int& i = ent.GetComponent<int>();
    i = 1;
    
    auto& nested = ent.GetComponent<SubComponent>();
    int& j = nested.GetComponent<int>();
    j = 2;
    
    int& k = ent.GetComponent<SubSubComponent, SubComponent, int>();
    k = 3;
    
    ent.GetComponent<std::string>() = "Hello, World!";
    
    std::cout << std::to_string(ent.GetComponent<int>()) << std::endl;
    std::cout << std::to_string(ent.GetComponent<SubComponent, int>()) << std::endl;
    std::cout << std::to_string(ent.GetComponent<SubSubComponent, SubComponent, int>()) << std::endl;
    std::cout << ent.GetComponent<std::string>() << std::endl;
    return 0;
}
