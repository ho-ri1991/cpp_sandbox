template <typename T>
struct EqualyComparable {
    friend bool operator!=(const T& lhs, const T& rhs) { return !(lhs == rhs); }
};

struct X: EqualyComparable<X> {
    int x;
    friend bool operator==(const X& lhs, const X& rhs) { return lhs.x == rhs.x; }
};

int main() {
    X x1, x2;
    x1 == x2;
    return 0;
}
