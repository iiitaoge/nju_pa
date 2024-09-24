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

#ifndef __DEVICE_MAP_H__
#define __DEVICE_MAP_H__

#include <cpu/difftest.h>

typedef void(*io_callback_t)(uint32_t, int, bool);  // 回调函数
uint8_t* new_space(int size); // 为设备分配的内存空间

typedef struct {
  const char *name;
  // we treat ioaddr_t as paddr_t here
  paddr_t low;  // 设备的起始地址
  paddr_t high; // 设备的终止地址
  void *space;  // 映射的目标空间
  io_callback_t callback; // 一个回调函数
} IOMap; // IOMap 结构体表示设备的一部分（如设备的寄存器或内存区域）的映射，而不一定是整个设备


// 用来检查访问的内存地址是否属于设备的范围之内
static inline bool map_inside(IOMap *map, paddr_t addr) {
  return (addr >= map->low && addr <= map->high);
}

// IOMap *maps 代表一个或多个 IOMap结构体
static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
  int i;
  for (i = 0; i < size; i ++) { // 遍历maps数组 找到对应地址映射的IO设备
    if (map_inside(maps + i, addr)) { // maps + 1 是指针的偏移，并不会只是加 1 个字节的跨度。它实际上根据结构体的大小自动计算
      difftest_skip_ref();
      return i;
    }
  }
  return -1;
}

// 添加端口映射（PIO）。将一个设备映射到 I/O 端口空间
void add_pio_map(const char *name, ioaddr_t addr,
        void *space, uint32_t len, io_callback_t callback);

// 添加内存映射（MMIO）。将一个设备映射到内存空间        
void add_mmio_map(const char *name, paddr_t addr,
        void *space, uint32_t len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, int len, word_t data, IOMap *map);

#endif
