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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */

  // 设置 mcause 寄存器，记录中断或异常的原因
  cpu.csr.mcause = NO;

  // 设置 mepc 寄存器，记录中断或异常发生时的 PC 值
  cpu.csr.mepc = epc;

  // 可选：设置 mtval 寄存器，记录与中断或异常相关的额外信息
  // cpu.csr.mtval = some_value;

  // 获取中断向量表的地址
  vaddr_t vector_addr = cpu.csr.mtvec;

  // 返回中断向量表的地址
  return vector_addr;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
