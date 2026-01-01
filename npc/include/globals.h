#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "common.h"
#include "VysyxSoCFull.h"
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <verilated_dpi.h>
#include "VysyxSoCFull__Dpi.h"
#include "VysyxSoCFull___024root.h"

extern VerilatedContext *contextp;
extern VysyxSoCFull *top;
extern VerilatedVcdC *tfp;

typedef struct {
  uint32_t gpr[32];
  uint32_t pc;
  uint32_t csr[0xfff];
}CPU_state;

extern CPU_state cpu;

enum { NPC_RUNNING, NPC_STOP, NPC_END, NPC_ABORT, NPC_QUIT };

typedef struct {
  int state;
  uint32_t halt_pc;
  uint32_t halt_ret;
} NPCState;

extern NPCState npc_state;


typedef struct ISADecodeInfo{
  union {
    uint32_t val;
  } inst;
}ISADecodeInfo;

typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;

extern Decode s;

extern uint64_t g_nr_guest_inst;
#endif