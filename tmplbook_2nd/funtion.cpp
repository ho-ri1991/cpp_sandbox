#include <iostream>
#include <utility>

template <typename>
class function;

template <typename R, typename... Args>
class function<R(Args...)> {
private:
    class function_holder {
    public:
        virtual ~function_holder(){}
        virtual function_holder* clone() const = 0;
        virtual R invoke(Args... args) const = 0;
    };

    template <typename Functor>
    class function_holder_impl: public function_holder {
    private:
        Functor functor;
    public:
        template <typename F>
        function_holder_impl(F&& f): functor(std::forward<F>(f)){}
        function_holder* clone() const override { return new function_holder_impl(*this); }
        R invoke(Args... args) const override { return functor(std::forward<Args>(args)...); }
    };

    function_holder* func;

public:
    void swap(function& other){
        using std::swap;
        swap(func, other->func);
    }
    function(): func(nullptr){}
    function(const function& other): func(nullptr){
        if(other.func) {
            func = other.func->clone();
        }
    }
    function(function& other): function(static_cast<const function&>(other)){}
    function(function&& other): func(other.func) {
        other.func = nullptr;
    }
    template <typename F, typename = std::enable_if_t<!std::is_base_of_v<function, std::decay_t<F>>>>
    function(F f): func(new function_holder_impl<std::decay_t<F>>(std::forward<F>(f))){}
    function& operator=(const function& other) {
        function tmp{other};
        tmp.swap(*this);
        return *this;
    }
    function& operator=(function&& other) {
        delete func;
        func = nullptr;
        swap(other);
        return *this;
    }
    template <typename F>
    function& operator=(F&& f) {
        function tmp{std::forward<F>(f)};
        swap(tmp);
        return *this;
    }
    ~function(){ delete func; }

    explicit operator bool() const { return func != nullptr; }
    R operator()(Args... args) const { return func->invoke(std::forward<Args>(args)...); }
};

int main(){
    function<int(void)> f;
    function<int(int)> f1 = [](int x){ return x; };
    std::cout << f1(1) << std::endl;
    auto f2 = f1;
    std::cout << f2(0) << std::endl;
    return 0;
}

