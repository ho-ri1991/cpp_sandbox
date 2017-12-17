struct Empty1{};

struct Empty2{};

struct Empty3: Empty1{};

struct Empty4: Empty2{};

struct Empty5: Empty1, Empty4{};

struct Empty6: Empty1, Empty3{};

int main(){
   static_assert(sizeof(Empty5) == sizeof(Empty1));
   static_assert(sizeof(Empty6) > sizeof(Empty1)); //diamond inheritance
}

