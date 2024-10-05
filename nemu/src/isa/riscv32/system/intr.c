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

word_t isa_raise_intr(word_t NO, vaddr_t epc) { // NO 在 am CTE中初始化
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */

  if (NO == 11)
  {
    epc += 4;
  }

  IFDEF(CONFIG_ETRACE, printf("产生了异常编号为 %d\n", NO));

  cpu.csr.mcause = NO;
  cpu.csr.mepc = epc;
  
  // printf("异常编号为 %d, 异常地址为 %#x\n", NO, epc); // 在 nemu 中实现异常踪迹
  return cpu.csr.mtvec;

}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
