#include "../include/paddr.h"
#include "../include/common.h"
#include "../include/globals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *pmem = NULL;
uint16_t sdram1_bank1[8192][512];
uint16_t sdram1_bank2[8192][512];
uint16_t sdram1_bank3[8192][512];
uint16_t sdram1_bank4[8192][512];
uint16_t sdram2_bank1[8192][512];
uint16_t sdram2_bank2[8192][512];
uint16_t sdram2_bank3[8192][512];
uint16_t sdram2_bank4[8192][512];
uint32_t host_read(void *addr, int len){
    if (addr == NULL) {
        fprintf(stderr, "Error: NULL pointer access in host_read\n");
        return 0;
    }
    switch (len){
        case 1: return *(uint8_t *)addr;
        case 2: return *(uint16_t *)addr;
        case 4: return *(uint32_t *)addr;
        default: return 0;
    }
}
void host_write(void *addr, int len, uint32_t data){
    if (addr == NULL) {
        fprintf(stderr, "Error: NULL pointer access in host_write\n");
        return;
    }
    switch (len){
        case 1:  *(uint8_t  *)addr = data; return;
        case 2:  *(uint16_t *)addr = data; return;
        case 4:  *(uint32_t *)addr = data; return;
        default: assert(0);
    }
}
uint8_t *guest_to_host(uint32_t paddr) {
  if (paddr < CONFIG_MBASE || paddr >= CONFIG_MBASE + CONFIG_MSIZE) {
        fprintf(stderr, "Error: Address out of range in guest_to_host\n");
        return NULL;
  }
  return pmem + paddr - CONFIG_MBASE; 
}

word_t pmem_read(paddr_t addr, int len){
  word_t ret = host_read(guest_to_host(addr),len);
#ifdef CONFIG_MTRACE
  printf("read: addr = 0x%08x, len = %d, data = 0x%08x\n",addr,len,ret);
#endif
  return ret;
}

void pmem_write(paddr_t addr, int len, word_t data){
#ifdef CONFIG_MTRACE
  printf("write: addr = 0x%08x, len = %d, data = 0x%08x\n",addr,len,data);
#endif
  host_write(guest_to_host(addr), len, data);
}

void init_mem(){
  pmem = (uint8_t *)malloc(CONFIG_MSIZE);
  memset(pmem,rand(),CONFIG_MSIZE);
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

bool in_pmem(paddr_t addr){
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
  return;
}

word_t paddr_read(paddr_t addr, int len){
  if(in_pmem(addr)){
    return pmem_read(addr, len);
  }
#ifdef CONFIG_DEVICE
  return 0;
#endif
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data){
  if(in_pmem(addr)){
    pmem_write(addr, len, data);
    return;
  }
#ifdef CONFIG_DEVICE
  return ;
#endif
  out_of_bound(addr);
}

uint32_t paddr_sdram_read1(int bankaddr, int row, int col){
  // uint32_t row = row & 0x1fff;
  // uint32_t col = col & 0x1fff;
  uint32_t data = 0;
  if(bankaddr == 0){
    data = sdram1_bank1[row][col];
  }
  else if(bankaddr == 1){
    data = sdram1_bank2[row][col];
  }
  else if(bankaddr == 2){
    data = sdram1_bank3[row][col];
  }
  else if(bankaddr == 3){
    data = sdram1_bank4[row][col];
  }
  return data;
}
uint32_t paddr_sdram_read2(int bankaddr, int row, int col){
  // uint32_t row = row & 0x1fff;
  // uint32_t col = col & 0x1fff;
  uint32_t data = 0;
  if(bankaddr == 0){
    data = sdram2_bank1[row][col];
  }
  else if(bankaddr == 1){
    data = sdram2_bank2[row][col];
  }
  else if(bankaddr == 2){
    data = sdram2_bank3[row][col];
  }
  else if(bankaddr == 3){
    data = sdram2_bank4[row][col];
  }
  return data;
}

void paddr_sdram_write1(int bankaddr, int row, int col, unsigned int data, int mask){
  // uint32_t row = row & 0x1fff;
  // uint32_t col = col & 0x1fff;
  int maskt = mask & 0x00000003;
  uint16_t wdata = data & 0x0000ffff;
  uint16_t data_read = paddr_sdram_read1(bankaddr, row, col);
  // uint8_t data_high  = (uint8_t)(data_read >> 8);
  // uint8_t data_low   = (uint8_t)(data_read & 0xff);
  if(bankaddr == 0){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram1_bank1[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram1_bank1[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram1_bank1[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 1){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram1_bank2[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram1_bank2[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram1_bank2[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 2){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram1_bank3[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram1_bank3[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram1_bank3[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 3){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram1_bank4[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram1_bank4[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram1_bank4[row][col] = wdata;
      break;
    default:
      break;
    }
  }
}

void paddr_sdram_write2(int bankaddr, int row, int col, unsigned int data, int mask){
  // uint32_t row = row & 0x1fff;
  // uint32_t col = col & 0x1fff;
  int maskt = mask & 0x00000003;
  uint16_t wdata = data & 0x0000ffff;
  uint16_t data_read = paddr_sdram_read2(bankaddr, row, col);
  if(bankaddr == 0){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram2_bank1[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram2_bank1[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram2_bank1[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 1){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram2_bank2[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram2_bank2[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram2_bank2[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 2){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram2_bank3[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram2_bank3[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram2_bank3[row][col] = wdata;
      break;
    default:
      break;
    }
  }
  else if(bankaddr == 3){
    switch (maskt)
    {
    case 0:
      break;
    case 1:
      sdram2_bank4[row][col] = (wdata & 0x00ff) | (data_read & 0xff00);
      break;
    case 2:
      sdram2_bank4[row][col] = (wdata & 0xff00) | (data_read & 0x00ff);
      break;
    case 3:
      sdram2_bank4[row][col] = wdata;
      break;
    default:
      break;
    }
  }
}