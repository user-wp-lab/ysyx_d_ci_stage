#include "../include/cpu.h"
#include <stdio.h>
#include "../include/globals.h"

void set_npc_state(int state, unsigned int pc, int halt_ret){
	npc_state.state = state;
  	npc_state.halt_pc = pc;
  	npc_state.halt_ret = halt_ret;
}
