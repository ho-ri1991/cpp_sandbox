#include <iterator>

template <typename Derived, typename Value, typename Category, typename Reference = Value&, typename Distance = std::ptrdiff_t>
class IteratorFacade;

class IteratorFacadeAccess {
    template <typename Derived, typename Value, typename Category, typename Reference, typename Distance>
    friend class IteratorFacade;

    template <typename Reference, typename Iterator>
    static Reference dereference(const Iterator& itr) { return itr.dereference(); }
    template <typename Iterator>
    static void increment(Iterator& itr) { itr.increment(); }
    template <typename Iterator>
    static void decrement(Iterator& itr) { itr.decrement(); }
    template <typename Iterator, typename Distance>
    static void advance(Iterator& itr, Distance n) { itr.advance(n); }
    template <typename Iterator>
    static bool equal(const Iterator& itr1, const Iterator& itr2) { return itr1.equal(itr2); }
};

template <typename Derived, typename Value, typename Category, typename Reference, typename Distance>
class IteratorFacade {
private:
    Derived& asDerived() { return *static_cast<Derived*>(this); }
    const Derived& asDerived() const{ return *static_cast<const Derived*>(this); }
public:
    using value_type = std::remove_const<Value>;
    using reference = Reference;
    using pointer = Value*;
    using difference_type = Distance;
    using iterator_category = Category;

    //input iterator interface
    reference operator *() const { return IteratorFacadeAccess::dereference<reference>(this->asDerived()); }
    pointer operator ->()const { return &IteratorFacadeAccess::dereference<reference>(this->asDerived()); }
    Derived& operator++() {
        IteratorFacadeAccess::increment(this->asDerived());
	return this->asDerived();
    }
    Derived operator++(int) {
        Derived copy = this->asDerived();
	IteratorFacadeAccess::increment(this->asDerived());
	return copy;
    }
    friend bool operator== (const IteratorFacade& lhs, const IteratorFacade& rhs) {
        return IteratorFacadeAccess::equal(lhs.asDerived(), rhs.asDerived());
    }

    //bidrectional iterator interface
    Derived& operator --() {
        IteratorFacadeAccess::decrement(this->asDerived());
	return IteratorFacadeAccess::dereference<reference>(this->asDerived());
    }
    Derived operator --(int) {
        Derived copy = IteratorFacadeAccess::dereference<reference>(this->asDerived());
	IteratorFacadeAccess::decrement(this->asDerived());
	return copy;
    }

    //random access iterator interface
    reference operator[](difference_type n) const{
        auto itr = *this;
	IteratorFacadeAccess::advance(itr.asDerived(), n);
	return IteratorFacadeAccess::dereference<reference>(itr.asDerived());
    }
    Derived& operator+=(difference_type n) {
        IteratorFacadeAccess::advance(this->asDerived(), n);
	return IteratorFacadeAccess::dereference<reference>(this->asDerived());
    }
};

template <typename T>
class ListNode {
public:
    T value;
    ListNode<T>* next = nullptr;
    ~ListNode() { delete next; }
};

template <typename T>
class ListNodeIterator: public IteratorFacade<ListNodeIterator<T>, T, std::forward_iterator_tag> {
    friend class IteratorFacadeAccess;
    ListNode<T>* current = nullptr;

    T& dereference() const{ return current->value; }
    void increment() { current = current->next; }
    bool equal(const ListNodeIterator& other) const{ return current == other.current; }
    
public:
    ListNodeIterator(ListNode<T>* current = nullptr): current(current){}
};

struct X{
    int x;
    int y;
};

int main(){
    auto ls = new ListNode<X>();
    ls->value = {0, 0};
    ls->next = new ListNode<X>();
    ListNodeIterator<X> itr(ls);
    ListNodeIterator<X> itr1(ls->next);
    *itr;
    itr->x = 1;
    ++itr;

    itr == itr1;

    delete ls;
    return 0;
}
