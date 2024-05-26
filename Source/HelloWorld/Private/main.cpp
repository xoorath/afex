// Hello World
#include "./Logging/AppLogging.h"

// Engine
#include <Core/afex.h>
#include <Core/Logging.h>

// System
#include <conio.h>
#include <vector>
#include <string>

int main() {
    // Configure SPD first. This will let the logging macros from Core/Logging output consistently. 
    HelloWorld::ConfigureLogging();

    afex();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    afex_print_vector(vec);

    AFEX_LOG_INFO("Welcome to spdlog from main!");
    
    while(!_getch());
}
