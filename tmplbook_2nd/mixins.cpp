#include <string>
#include <vector>

template <typename... Mixins>
struct Point: Mixins... {
    double x, y;
    Point(): Mixins()..., x(0), y(0){}
    Point(double x, double y): Mixins()..., x(x), y(y){}
};

template <typename... Mixins>
class Polygon {
    std::vector<Point<Mixins...>> points;
public:
    //some public interface
};

struct Label {
    std::string label;
    Label(): label(){}
};

struct Color {
unsigned char red = 0, green = 0, blue = 9;
};

using MyPoint = Point<Label, Color>;

//curious mixins, client mixin classes can obtain the information about Point class through the template parameter
template <template<typename> class... Mixins>
struct Point2: Mixins<Point2<Mixins...>>... {
    double x, y;
    Point2(): Mixins<Point2>()..., x(0), y(0){}
    Point2(double x, double y): Mixins<Point2>()..., x(x), y(y){}
};

template <typename Point>
struct Label1 {
    std::string label;
};

int main(){
    return 0;
}

