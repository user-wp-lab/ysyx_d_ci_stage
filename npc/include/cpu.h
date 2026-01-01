#ifndef __CPU_H__
#define __CPU_H__
#include "common.h"
// void cpu_exec(uint32_t n);
void cpu_exec(uint64_t n);

void set_npc_state(int state, unsigned int pc, int halt_ret);
void invalid_inst(uint32_t thispc);

#define NPCTRAP(thispc, code) set_npc_state(NPC_END, thispc, code);
#define INV(thispc) invalid_inst(thispc) 


#endif
