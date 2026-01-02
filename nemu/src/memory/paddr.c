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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};  //flash
#endif

#ifdef CONFIG_SRAM
static uint8_t sram[CONFIG_SRAM_SIZE] PG_ALIGN = {};  //sram
#endif

#ifdef CONFIG_SDRAM
static uint8_t sdram[CONFIG_SDRAM_SIZE] PG_ALIGN = {};  //sdram
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

#ifdef CONFIG_SRAM
static word_t sram_read(paddr_t addr, int len) {
  uint8_t *saddr = sram + addr - CONFIG_SRAM_BASE;
  word_t ret = host_read(saddr, len);
  return ret;
}

static void sram_write(paddr_t addr, int len, word_t data) {
  uint8_t *saddr = sram + addr - CONFIG_SRAM_BASE;
  host_write(saddr, len, data);
}

// static void out_of_sram(paddr_t addr) {
//   uint32_t sram_right = CONFIG_SRAM_BASE + CONFIG_SRAM_SIZE;
//   panic("address = " FMT_PADDR " is out of bound of sram [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
//       addr, CONFIG_SRAM_BASE, sram_right, cpu.pc);
// }
#endif

#ifdef CONFIG_SDRAM
static word_t sdram_read(paddr_t addr, int len) {
  uint8_t *sdaddr = sdram + addr - CONFIG_SDRAM_BASE;
  word_t ret = host_read(sdaddr, len);
  return ret;
}

static void sdram_write(paddr_t addr, int len, word_t data) {
  uint8_t *sdaddr = sdram + addr - CONFIG_SDRAM_BASE;
  host_write(sdaddr, len, data);
}

#endif

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
#ifdef CONFIG_SRAM
  IFDEF(CONFIG_SRAM, memset(sram, rand(), CONFIG_SRAM_SIZE));
#endif
#ifdef CONFIG_SDRAM
  IFDEF(CONFIG_SDRAM, memset(sdram, rand(), CONFIG_SDRAM_SIZE));
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

#ifdef CONFIG_SRAM
static bool in_sram(paddr_t addr) {
  return addr - CONFIG_SRAM_BASE < CONFIG_SRAM_SIZE;
}
#endif

#ifdef CONFIG_SDRAM
static bool in_sdram(paddr_t addr) {
  return addr - CONFIG_SDRAM_BASE < CONFIG_SDRAM_SIZE;
}
#endif

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) {
#ifdef CONFIG_MTRACE
	if(CONFIG_MTRACE_COND_ONE <= addr && CONFIG_MTRACE_COND_TWO >=addr)
	printf("read: addr = 0x%08x len = %x inst = %08x\n",addr,len,pmem_read(addr,len));	  
#endif
	return pmem_read(addr, len);
  }
#ifdef CONFIG_SRAM
  if(likely(in_sram(addr)))
  {
    return sram_read(addr, len);
    printf("Shouldn't reach here(read)\n");
  }
#endif
#ifdef CONFIG_SDRAM
  if(likely(in_sdram(addr)))
  {
    return sdram_read(addr, len);
    printf("Shouldn't reach here(read)\n");
  }
#endif
 #ifdef CONFIG_CACHESIM
  if(addr == 0x10000005){
    return 0x00000020;
  }  
  #endif
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { 
  #ifdef CONFIG_SDRAM
  printf("flash write");
  #endif
	pmem_write(addr, len, data);
#ifdef CONFIG_MTRACE	
	if(CONFIG_MTRACE_COND_ONE <= addr && CONFIG_MTRACE_COND_TWO >=addr)
	printf("wirte: addr = 0x%08x len = %x data = %08x\n",addr,len,data);
#endif
	return; 
  }
#ifdef CONFIG_SRAM
  if(likely(in_sram(addr)))
  {
    sram_write(addr, len, data);
    return;
    printf("Shouldn't reach here(write)\n");
  }
#endif
#ifdef CONFIG_SRAM
  if(likely(in_sdram(addr)))
  {
    sdram_write(addr, len, data);
    return;
    printf("Shouldn't reach here(write)\n");
  }
#endif
  #ifdef CONFIG_DEVICE
    #ifdef CONFIG_CACHESIM
    if(((addr >= 0x10000001) && (addr <= 0x10000005)) || ((addr == 0x10000000) && (data == 0x01))){
      return;
    }
    else if((addr == 0x10000000) && (data != 0x01))
    {
      IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
    }
    else {
      IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
    }
    #else
    IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
    #endif
  #endif
  out_of_bound(addr);
}
