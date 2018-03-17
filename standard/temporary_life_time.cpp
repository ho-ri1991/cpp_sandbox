#include <iostream>
#include <vector>

struct X {
    const std::vector<int>& v;
    X(const std::vector<int>& v): v(v){}
};

struct Y {
    const std::vector<int>& v;
};

X func1(const X& x, const X& y){
    return x.v.size() != 0 ? x.v : y.v;
}

X func2(const X& x) {
    return x.v;
}

int main() {
//    undefined behavior, x.v is a dangling reference. When I compiled with -O2 option, the output was 0.
    X x = std::vector<int>{1,2,3};
    std::cout << x.v[0] << std::endl;

    // OK, aggregate initialization directory initializes the members
    Y y ={std::vector<int>{1,2,3}};
    std::cout << y.v[0] << std::endl;

    // OK, life time of temporaries extends within the full expression
    // i.e. std::vector<int>{...}s in arguments of func1 and return value of func1
    std::cout 
        << func2(func1(func1(std::vector<int>{-1,2,3}, std::vector<int>{1,2,3}), func1(std::vector<int>{1,2,3}, std::vector<int>{1,2,3}))).v[0] 
        << std::endl;

    return 0;
}

