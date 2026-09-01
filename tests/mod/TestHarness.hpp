#ifndef TH08_MOD_TESTS_TEST_HARNESS_HPP
#define TH08_MOD_TESTS_TEST_HARNESS_HPP

#include "ModApi.h"

#include <iostream>

extern int g_failures;

#define CHECK(condition)                                                       \
    do                                                                         \
    {                                                                          \
        if (!(condition))                                                      \
        {                                                                      \
            std::cerr << __FILE__ << ':' << __LINE__                           \
                      << ": check failed: " #condition << '\n';              \
            ++g_failures;                                                      \
        }                                                                      \
    } while (false)

void ResetRuntime();

#endif
