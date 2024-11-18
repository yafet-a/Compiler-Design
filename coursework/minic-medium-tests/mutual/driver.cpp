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
int hofstadterMale(int n);
int hofstadterFemale(int n);
}

int main() {
    int female = hofstadterFemale(7);
    int male = hofstadterMale(7);
    if (female == 5 && male == 4)
        std::cout << "PASSED Result: " << female << ", " << male << std::endl;
    else
        std::cout << "FAILED Result: " << female << ", " << male << std::endl;
}
