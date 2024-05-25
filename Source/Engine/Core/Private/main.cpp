#include <Core/afex.h>
#include <vector>
#include <string>

int main() {
    afex();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    afex_print_vector(vec);
}
