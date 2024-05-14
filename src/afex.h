#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define AFEX_EXPORT __declspec(dllexport)
#else
  #define AFEX_EXPORT
#endif

AFEX_EXPORT void afex();
AFEX_EXPORT void afex_print_vector(const std::vector<std::string> &strings);
