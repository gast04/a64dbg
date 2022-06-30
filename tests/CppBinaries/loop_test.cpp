#include <iostream>
#include <fstream>

uint64_t glob_int = 0;

int main() {
    printf("starting loop test\n");
    while(1) {
        printf("glob: %lu\n", glob_int++);
    }
    return 0;
}
