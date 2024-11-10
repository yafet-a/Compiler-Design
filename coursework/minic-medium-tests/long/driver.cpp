#include <cstdio>
#include <iostream>

// clang++ driver.cpp addition.ll -o add

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT int print_int(int X) {
    fprintf(stderr, "%d\n", X);
    return 0;
}

extern "C" DLLEXPORT float print_float(float X) {
    fprintf(stderr, "%f\n", X);
    return 0;
}

extern "C" {
bool drive();
}

int main() {
    bool result = drive();
    if (result == 0)
        std::cout << "PASSED Result: " << result << std::endl;
    else
        std::cout << "FAILED Result: " << result << std::endl;
}
