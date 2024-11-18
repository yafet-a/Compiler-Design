#include <stdio.h>

extern int hofstadterMale(int n);

int hofstadterFemale(int n) {
    if (n == 0) {
        return 1;
    } else {
        return (n - hofstadterMale(hofstadterFemale(n - 1)));
    }
}

int hofstadterMale(int n) {
    if (n == 0) {
        return 0;
    } else {
        return (n - hofstadterFemale(hofstadterMale(n - 1)));
    }
}

int main() {
    printf("%d\n", hofstadterFemale(7));
    printf("%d\n", hofstadterMale(7));
}
