#include <iostream>
#include <cstddef>

template <typename T>
class ObjectCounter {
private:
    friend T;
    inline static std::size_t count = 0;
    ObjectCounter(){ ++count; }
    ObjectCounter(const ObjectCounter<T>&){ ++count; }
    ObjectCounter(ObjectCounter<T>&&){ ++count; }
    ~ObjectCounter(){ --count; }
public:
    static std::size_t live(){ return count; }
};

class MyClass: private ObjectCounter<MyClass> {
public:
    using ObjectCounter<MyClass>::live;
};

int main() {
    std::cout << MyClass::live() << std::endl;
    {
        MyClass c1;
        std::cout << MyClass::live() << std::endl;
        { 
            MyClass c2;
    	    std::cout << MyClass::live() << std::endl;
        }
        std::cout << MyClass::live() << std::endl;
    }
    std::cout << MyClass::live() << std::endl;
}

