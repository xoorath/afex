#include <Core/afex.h>
#include <vector>
#include <string>
#include <spdlog/spdlog.h>

int main() {
    afex();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    afex_print_vector(vec);
    spdlog::info("Welcome to spdlog from main!");
    system("pause");
}
