#ifndef __PADDR_H__
#define __PADDR_H__
#include <stdint.h>
#include <stddef.h>
#include "common.h"
void init_mem();
uint32_t host_read(void *addr, int len);
void host_write(void *addr, int len, uint32_t data);
uint8_t *guest_to_host(uint32_t paddr);
word_t pmem_read(paddr_t addr, int len);
void pmem_write(paddr_t addr, int len, word_t data);
word_t paddr_read(paddr_t addr, int len);
void paddr_write(paddr_t addr, int len, word_t data);
uint32_t paddr_sdram_read1(int bankaddr, int row, int col);
uint32_t paddr_sdram_read2(int bankaddr, int row, int col);
void paddr_sdram_write1(int bankaddr, int row, int col, unsigned int data, int mask);
void paddr_sdram_write2(int bankaddr, int row, int col, unsigned int data, int mask);
#endif
