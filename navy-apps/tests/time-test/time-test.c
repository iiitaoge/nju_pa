#include <stdint.h>
#include <NDL.h>
#define SYS_gettimeofday 19

extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
    printf("time-test.c 开始执行\n");
    int sec = 1;
    while (1)
    {
        while(NDL_GetTicks() / 500 < sec) ;
        printf("fuck you time-test\n");
        sec ++;
    }
    return 0;
}