#include <iostream>
#include <memory>

template <typename T>
struct foo_traits {
    static void foo(const T& t) { t.foo(); }
};

class foo {
private:
    class foo_holder {
    public:
        virtual std::unique_ptr<foo_holder> clone() const = 0;
        virtual void invoke_foo() const = 0;
        virtual ~foo_holder() = default;
    };

    template <typename T>
    class foo_holder_impl:public foo_holder {
    private:
        T foo_;
    public:
        foo_holder_impl(const T& t): foo_(t){};
        foo_holder_impl(T&& t): foo_(std::move(t)) {}
        foo_holder_impl(const foo_holder_impl&) = default;
        foo_holder_impl(foo_holder_impl&&) = default;
        foo_holder_impl& operator=(const foo_holder_impl&) = default;
        foo_holder_impl& operator=(foo_holder_impl&&) = default;
        ~foo_holder_impl() = default;
    public:
        std::unique_ptr<foo_holder> clone() const override { return std::make_unique<foo_holder_impl>(*this); }
        void invoke_foo() const override { foo_traits<T>::foo(foo_); }
    };
private:
    std::unique_ptr<foo_holder> foo_;
public:
    void swap(foo& other) noexcept {
        using std::swap;
        swap(foo_, other.foo_);
    }
    foo() = delete;
    foo(const foo& other): foo_(other.foo_->clone()){}
    foo(foo&& other) noexcept = default;
    foo& operator=(const foo& other) {
        foo tmp(other);
        this->swap(tmp);
        return *this;
    }
    foo& operator=(foo&& other) noexcept {
        this->swap(other);
        return *this;
    }
    ~foo() = default;
    template <typename T, typename = std::enable_if_t<!std::is_base_of_v<foo, std::decay_t<T>>>>
    foo(T&& t): foo_(std::make_unique<foo_holder_impl<std::decay_t<T>>>(std::forward<T>(t))) {}
    template <typename T>
    foo& operator=(T&& t) {
        foo tmp{std::forward<T>(t)};
        swap(tmp);
        return *this;
    }
public:
    void invoke_foo() const { foo_->invoke_foo();  }
};

struct foo1 {
    void foo() const { std::cout << "foo1" << std::endl;}
};

template<>
struct foo_traits<int> {
    static void foo(int i) { std::cout << "foo: " << i << std::endl;}
};

int main() {
    foo f1(foo1{});
    foo fi(42);
    f1.invoke_foo();
    fi.invoke_foo();

    return 0;
}

