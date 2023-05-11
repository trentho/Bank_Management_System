#include "libc.h"

int main(int argc, char** argv) {
    printf("*** argc: %x\n",argc);

    shutdown();
    return 0;
}
