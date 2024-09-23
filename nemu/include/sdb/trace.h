#include <common.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
struct function_trace
{
    char *name; // 函数名
    uint32_t start; // 函数起始点
    uint32_t end;  // 函数范围
};
extern uint32_t func_amount;   // 函数的个数
extern struct function_trace functions[];   // 假设有100个函数 