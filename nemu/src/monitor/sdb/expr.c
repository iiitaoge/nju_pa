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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/paddr.h>

static bool check_parentheses(int p, int q);
static bool check_small(int p, int q);
// static void print_tokens();
static word_t eval(int p, int q);
static int find_main_operator(int p, int q);
static void pointer_derefenrece();


enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBER_D, TK_POINTER, TK_NUMBER_H, TK_REGS,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus : ASCII 43
  {"==", TK_EQ},        // equal
  
  // 以下为我所补充的代码
  {"0[xX][0-9a-fA-F]+", TK_NUMBER_H},    // number_H
  {"[0-9]+", TK_NUMBER_D},// number_D
  {"-", '-'},           // minus  
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
  {"\\(", '('},         // left (
  {"\\)", ')'},         // right (
  {"\\$((pc)|(ra)|(sp)|(gp)|(tp)|(t[0-6])|(s([0-9]|1[0-1]))|(a[0-7])|0)", TK_REGS},  // 匹配寄存器
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[3200]; // 用来记录 token 的子串，比如一个整数的具体大小
} Token;

static Token tokens[3200] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') 
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) 
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) 
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        // 输入的字符匹配 rules ，匹配到就记录到 tokens
        switch (rules[i].token_type) {
          case TK_NUMBER_D:
            tokens[nr_token].type = TK_NUMBER_D;
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case '+':
            tokens[nr_token].type = '+';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case '-':
            tokens[nr_token].type = '-';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case '*':
            tokens[nr_token].type = '*';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case '/':
            tokens[nr_token].type = '/';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case '(':
            tokens[nr_token].type = '(';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;
          
          case ')':
            tokens[nr_token].type = ')';
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;
          
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;
          
          case TK_NOTYPE:
            break;
          
          case TK_NUMBER_H:
            tokens[nr_token].type = TK_NUMBER_H;
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          case TK_REGS:
            tokens[nr_token].type = TK_REGS;
            snprintf(tokens[nr_token].str, sizeof(tokens[nr_token].str), "%.*s", substr_len, substr_start);
            nr_token++;
            break;

          default: 
            // TODO();
            printf("未知命令");
        }

        break;
      }
    }

    // rules匹配到头都没有，i 就会 == NR_REGEX, 这意味着这个token不在rules
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // print_tokens(0, nr_token - 1);
  // bool a = check_parentheses(0, nr_token - 1);
  // if (a)
  // {
  //   printf("最外层括号匹配\n");
  // }
  // else
  // {
  //   a = check_small(0, nr_token - 1);
  //   if (a)
  //   {
  //     printf("表达式合法\n");
  //   }
  //   else 
  //   {
  //     printf("表达式非法\n");
  //     assert(0);
  //   }
  // }
  pointer_derefenrece();
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  return eval(0, nr_token - 1);
}

// static void print_tokens(int l, int r) // 打印 tokens 数组，检查
// {
//   for (int i = l; i <= r; i++)
//   {
//     printf("%s ", tokens[i].str);
//   }
//   printf("\n");
// }

// 将解引用和乘号区分
static void pointer_derefenrece()
{
  for (int i = 0; i < nr_token; i ++) 
  {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_NUMBER_D && tokens[i - 1].type != ')'))) 
    {
      tokens[i].type = TK_POINTER;
    }
  }
}

// 算数表达式语法分析器
static word_t eval(int p, int q) 
{
  if (p > q) {
    /* Bad expression */
    exit(1);
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_NUMBER_D)
    {
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == TK_NUMBER_H)
    {
      // 返回十六进制数
      return strtol(tokens[p].str, NULL, 16);// 到底要从内存中读取多长的数据
    }
    else if (tokens[p].type == TK_REGS) // 取出寄存器的值
    {
      char *reg_name = tokens[p].str + 1;
      bool success = false;
      return isa_reg_str2val(reg_name, &success);
    }
    else
    {
      assert(0);
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else 
  {
    if (check_small(p,q) == false)
    {
      printf("表达式括号非法");
      assert(0);
    }
    int op = find_main_operator(p, q);
    if (op == -1) // 此时有解引用符号 *
    {
      return paddr_read(eval(p + 1, q), 4); // 解引用，返回32位，4个字节
    }
    else
    {
      word_t val1 = eval(p, op - 1);
      word_t val2 = eval(op + 1, q);

      switch (tokens[op].type) 
      {
          case '+': return val1 + val2;
          case '-': return val1 - val2;
          case '*': return val1 * val2;
          case '/': 
              if (val2 == 0) 
              {
                  printf("Error: Division by zero!\n");
                  exit(1);  // 返回一个错误值或处理错误
              }
              return val1 / val2;
          // 可以根据需要扩展其他运算符
          default:
              printf("Unknown operator!\n");
              exit(1);  // 返回一个错误值或处理错误
      /* We should do more things here. */
      }
    }
  }
}

// 检查 最外层 的括号是否匹配
static bool check_parentheses(int p, int q)
{
  if (tokens[p].type != '(' || tokens[q].type != ')')
  {
    return false;
  }
  if (check_small(p + 1,q - 1))
  {
    return true;
  }
  else
  {
    return false;
  }
}

// 检查表达式的括号是否 合法
static bool check_small(int p, int q) {
    int balance = 0;

    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(') {
            balance++;
        } else if (tokens[i].type == ')') {
            balance--;
            if (balance < 0) {
                return false;  // 多余的右括号，匹配失败
            }
        }
    }

    return balance == 0;  // 如果 balance 最终为 0，表示匹配成功
}

// 寻找主运算符
static int find_main_operator(int p, int q) {
    int op = -1;
    int level = 0;  // 用于跟踪括号的嵌套深度

    for (int i = p; i <= q; i++) 
    {
        if (tokens[i].type == '(') 
        {
            level++;
        } 
        else if (tokens[i].type == ')') 
        {
            level--;
        } 
        else if (level == 0) 
        {
            // 在最外层
            if (tokens[i].type == '+' || tokens[i].type == '-') 
            {
                op = i;  // 优先考虑加减运算符
            } 
            else if ((tokens[i].type == '*' || tokens[i].type == '/') && op == -1) 
            {
                op = i;  // 如果没有找到加减运算符，记录乘除运算符的位置
            }
        }
    }

    // if (op == -1)
    // {
    //   printf("位置于 %d %d 出现了问题", p, q);
    //   print_tokens(p, q);
    // }

    return op;
}