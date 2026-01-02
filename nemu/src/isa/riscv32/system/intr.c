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
  //0x342 mcause; 0x341 mepc; 0x300 status; 0x305 mtvec
  cpu.csr[0x341] = epc;
  cpu.csr[0x342] = NO;
  word_t mtvec = cpu.csr[0x305];
  
#ifdef CONFIG_ETRACE
  printf("\nnemu : \n");
  for(int i = 0;i < 32;i++){
      printf("gpr[%d] = 0x%08x\n",i,cpu.gpr[i]);
  }
  printf("mcause = %08x mstatus_t = %08x mepc = %08x\n", cpu.csr[0x342], cpu.csr[0x300], epc);
#endif
  return mtvec;
}

void isa_mret_intr(){
  word_t mstatus_t = cpu.csr[0x300];
  mstatus_t &= ~(0x3 << 11);
  word_t mpie = (mstatus_t >> 7) & 0x1;
  mstatus_t = (mstatus_t & ~(0x1 << 3)) | (mpie << 3);
  mstatus_t |= (0x1 << 7);
  cpu.csr[0x300] = mstatus_t;
  return ;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
