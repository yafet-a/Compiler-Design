extern int print_int(int X);
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
