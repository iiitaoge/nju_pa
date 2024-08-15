/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int token_count = 0; // 用于跟踪当前表达式中的 tokens 数量

static void gen_num() {
    int num = rand() % 100; // 生成 0 到 99 之间的整数
    char buffer[20];
    sprintf(buffer, "%d", num); // 转换为字符串形式
    strcat(buf, buffer); // 将数字附加到表达式缓冲区中
    token_count++; // 增加 token 计数
}

static void gen_rand_op() {
    char ops[] = {'+', '-', '*', '/'};
    char op = ops[rand() % 4]; // 随机选择一个操作符
    strcat(buf, " ");
    strncat(buf, &op, 1); // 将操作符附加到表达式缓冲区中
    strcat(buf, " ");
    token_count++; // 增加 token 计数
}

static void gen_rand_expr(int remaining_tokens) {
    if (remaining_tokens == 1) {
        // 如果只剩下一个 token，必须生成一个数字
        gen_num();
        return;
    }

    int choice = rand() % 3;
    if (choice == 0) {
        // 生成一个数字
        gen_num();
    } else if (choice == 1 && remaining_tokens >= 3) {
        // 生成带括号的表达式，至少需要3个tokens
        strcat(buf, "(");
        token_count++;
        gen_rand_expr(remaining_tokens - 2); // 内部表达式消耗 tokens，括号消耗 2 个
        strcat(buf, ")");
        token_count++;
    } else if (remaining_tokens >= 3) {
        // 生成带操作符的表达式，至少需要3个tokens
        int tokens_left_for_right_expr = (remaining_tokens - 1) / 2;
        int tokens_left_for_left_expr = remaining_tokens - 1 - tokens_left_for_right_expr;
        gen_rand_expr(tokens_left_for_left_expr); // 生成左侧表达式
        gen_rand_op(); // 生成操作符
        gen_rand_expr(tokens_left_for_right_expr); // 生成右侧表达式
    } else {
        gen_num(); // 其他情况下生成一个数字
    }
}

// static void gen_rand_expr() 
// {
//   buf[0] = '\0';
// }

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr(32);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
