#include <iostream>
#include <vector>
#include <cassert>

template <typename T>
using ArrayRep = std::vector<T>;

template <typename T>
struct Traits;

template <typename T, typename Op1, typename Op2>
class Add {
private:
    typename Traits<Op1>::type op1;
    typename Traits<Op2>::type op2;
public:
    Add(const Op1& op1, const Op2& op2): op1(op1), op2(op2) {}
    T operator[](std::size_t i) const { return op1[i] + op2[i];}
    std::size_t size() const { return op1.size() != 0 ? op1.size() : op2.size();}
};

template <typename T, typename Op1, typename Op2>
class Sub {
private:
    typename Traits<Op1>::type op1;
    typename Traits<Op2>::type op2;
public:
    Sub(const Op1& op1, const Op2& op2): op1(op1), op2(op2) {}
    T operator[](std::size_t i) const { return op1[i] - op2[i]; }
    std::size_t size() const { return op1.size() != 0 ? op1.size() : op2.size(); }
};

template <typename T, typename Op1, typename Op2>
class Mult {
private:
    typename Traits<Op1>::type op1;
    typename Traits<Op2>::type op2;
public:
    Mult(const Op1& op1, const Op2& op2): op1(op1), op2(op2) {}
    T operator[](std::size_t i) const { return op1[i] * op2[i];}
    std::size_t size() const { return op1.size() != 0 ? op1.size() : op2.size();}
};

template <typename T>
class Scala {
private:
    const T& v;
public:
    Scala(const T& v): v(v) {}
    T operator[](std::size_t i) const { return v;}
    std::size_t size() const { return 0;}
};

template <typename T>
struct Traits {
    using type = const T&;
};
template <typename T>
struct Traits<Scala<T>> {
    using type = Scala<T>;
};

template <typename T, typename Rep = ArrayRep<T>>
class Array {
private:
    Rep exprRep;
public:
    explicit Array(std::size_t n): exprRep(n) {}
    explicit Array(std::size_t n, const T& v): exprRep(n, v) {}
    explicit Array(const Rep& rep): exprRep(rep) {}
    template <typename T2, typename Rep2>
    Array(const Array<T2, Rep2>& other): exprRep(other.size()) {
        for(int i = 0; i < other.size(); ++i) {
            exprRep[i] = other[i];
        }
    }
    Array& operator=(const Array& rhs) {
        for(std::size_t i = 0; i < rhs.size(); ++i) {
            exprRep[i] = rhs[i];
        }
        return *this;
    }
    template <typename T2, typename Rep2>
    Array& operator=(const Array<T2, Rep2>& rhs) {
        assert(exprRep.size() == rhs.size());
        for(std::size_t i = 0; i < rhs.size(); ++i) {
            exprRep[i] = rhs[i];
        }
        return *this;
    }
    std::size_t size() const { return exprRep.size(); }
    decltype(auto) operator[](std::size_t i) const { return exprRep[i]; }
    T& operator[] (std::size_t i) { return exprRep[i]; }
    Rep& rep() { return exprRep; }
    const Rep& rep() const { return exprRep; }
};

template <typename T, typename Rep1, typename Rep2>
Array<T, Add<T, Rep1, Rep2>> operator+(const Array<T, Rep1>& lhs, const Array<T, Rep2>& rhs) {
    return Array<T, Add<T, Rep1, Rep2>>(Add<T, Rep1, Rep2>(lhs.rep(), rhs.rep()));
}
template <typename T, typename Rep1, typename Rep2>
Array<T, Sub<T, Rep1, Rep2>> operator-(const Array<T, Rep1>& lhs, const Array<T, Rep2>& rhs) {
    return Array<T, Sub<T, Rep1, Rep2>>(Sub<T, Rep1, Rep2>(lhs.rep(), rhs.rep()));
}
template <typename T, typename Rep1, typename Rep2>
Array<T, Mult<T, Rep1, Rep2>> operator*(const Array<T, Rep1>& lhs, const Array<T, Rep2>& rhs) {
    return Array<T, Mult<T, Rep1, Rep2>>(Mult<T, Rep1, Rep2>(lhs.rep(), rhs.rep()));
}
template <typename T, typename Rep>
Array<T, Mult<T, Scala<T>, Array<T, Rep>>> operator*(const T& v, const Array<T, Rep>& a) {
    return Array<T, Mult<T, Scala<T>, Array<T, Rep>>>(Mult<T, Scala<T>, Array<T, Rep>>(v, a));
}

int main() {
    Array<double> a(10, 1), b(10, -1);
    Array<double> c = 1.2 * a + a * b - 1.0 * b;
    for(int i = 0; i < c.size(); ++i)
        std::cout << c[i] << ',';

    return 0;
}

