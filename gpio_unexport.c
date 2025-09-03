#include <stdio.h>
#include "gpio.h"

int main(void) {
    unexport_all();
    printf("=== All GPIO Unexported.(maybe)\n===");

    return 0;
}

