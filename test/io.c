#include<stdio.h>
#include<stdlib.h>
int GET() {
    int v;
    printf("input number:");
    scanf("%d", &v);
}
void * MALLOC(int v) {
    return malloc(v);
}
void FREE(void *p) {
    return free(p);
}
extern void PRINT(int v) {
    printf("%d\n", v);
}