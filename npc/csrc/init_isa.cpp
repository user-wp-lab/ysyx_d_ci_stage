#include "../include/isa.h"
#include "../include/paddr.h"
#include "../include/globals.h"
#include "../include/common.h"
static const uint32_t img [] = {  
  //此处放置的是npc用AM编译出来的指令
  0x00000413,  //dummy
  0x00009117,  
  0xffc10113,  
  0x00c000ef,  
  0x00000513,  
  0x00008067,
  0xff410113,
  0x00000517,
  0x01c50513,
  0x00112423,
  0xfe9ff0ef,
  0x00050513,
  0x00100073,  //ebreak
  0x0000006f,  //j
};

static void restart() {
  cpu.pc = RESET_VECTOR;
  cpu.gpr[0] = 0;
  cpu.csr[0x300] = 0x1800;
}

void init_isa() {
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
  restart();
}