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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO; // 监视点序号
  struct watchpoint *next;
  word_t value; // 监视点存储的表达式的值

  /* TODO: Add more members if necessary */

} WP;

static WP* new_wp();
static void free_wp(int NO);

char *wp_address[NR_WP]; // 记录地址，更新值的时候要求值
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]); // 最后一个节点没有next
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


// 0,1,2,3....顺序获得节点
static WP* new_wp()
{
  assert(free_);  // 当free_为空的时候触发
  // 两种情况， head == NULL 或 head != NULL
  if (head == NULL)
  {
    head = free_;
    free_ = free_->next;
    head->next = NULL;
    return head;
  }
  else
  {
    WP *temp = head;
    while (temp != NULL)
    {
        if (temp->next == NULL)
        {
        temp->next = free_;
        free_ = free_->next;
        temp->next->next = NULL;
        return temp->next;
        }
        else
        {
            temp = temp->next;
        }
    }
  }
  return NULL;
}


// 释放指定序号的节点回free_
static void free_wp(int NO)  // 参数为序号
{
  assert(head); // 监视点池没有节点
  WP *temp = head;
  if (head->next == NULL) // 监视点池只有一个
  {
    if (head->NO == NO)
    {
        head = NULL;
        temp->next = free_;
        free_ = temp;
        free_->value = 0; //归还的监视点出厂化
        return;
    }
  }
  while (temp != NULL)
  {
    if (temp->next != NULL && temp->next->NO == NO) // 监视点数目多于一个
    {
      WP *free_p = temp->next;
      temp->next = temp->next->next;  
      free_p->next = free_;
      free_ = free_p;
    }
    else
    {
        temp = temp->next;
    }
  }
}

// 打印监视点
bool print_watch_pointer()
{
  WP* temp = head;
  bool is_value_new = false;
  while (temp != NULL)
  {
    bool success = false;
    word_t new_wp_value = expr(wp_address[temp->NO], &success);
    if (new_wp_value != temp->value)
    {
      printf("\nwatchpoint %d\nold value : %u\nnew value : %u\n", temp->NO, temp->value, new_wp_value);
      is_value_new = true;
      temp->value = new_wp_value; // 更新value
    }
    temp = temp->next;
  }
  return is_value_new;
}

// 外部文件调用new_wp()的接口
void sdb_new_wp(char *args)
{
  char *address = strdup(args);  // 复制字符串并保存到新的内存中
  WP* temp = new_wp();
  bool success = false;
  wp_address[temp->NO] = address;  // 储存起来，求值要用
  temp->value = expr(address, &success);
}

// 外部文件删除监视点的接口
void delete_wp(char *args)
{
  free_wp(atoi(args));
}