
#include <stdio.h>
#include "lab1.h"
#include <time.h>
#include <stdint.h>
#include "lab2.cpp"
#include "lab1.h"
void test1(void)
{
    mem_dump();
    void* mem_area1 = mem_alloc(256);
    printf("Block Address:\t%p \n \n",mem_area1);
    mem_dump();
    void* mem_area2 = mem_alloc(3000);
    printf("Block Address:\t%p \n \n",mem_area2);
    mem_dump();
    mem_free(mem_area2);
    mem_dump();
}
int main(int argc, char *argv[])
{
    test1();
    return 0;
}
