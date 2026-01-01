#include "../include/sim_main.h"
#include "../include/common.h"
#include "../include/globals.h"

#ifdef CONFIG_NVBOARD
#include <nvboard.h>
void nvboard_bind_all_pins(VysyxSoCFull* top);
#endif

VerilatedContext *contextp = new VerilatedContext;
VysyxSoCFull *top = new VysyxSoCFull{contextp};
#ifdef CONFIG_VCD
VerilatedVcdC *tfp = new VerilatedVcdC;
#endif

void cpu_rst() {
    top->reset = 1;  // 复位信号开始为1
    int rst_time = 24;  // 将rst_time设置为4,设置为2来不及复位

    while (rst_time > 0) {
        contextp->timeInc(1);
        top->clock = !top->clock;  // 时钟信号翻转
        top->eval();
        #ifdef CONFIG_VCD
        tfp->dump(contextp->time());
        #endif
        rst_time--;
    }
    
    top->reset = 0;  // 完成复位，复位信号变为0
}


void init_verilate(int argc,char **argv){
  Verilated::commandArgs(argc, argv);
  #ifdef CONFIG_NVBOARD
  nvboard_bind_all_pins(top);
  nvboard_init();
  #endif
  #ifdef CONFIG_VCD
  Verilated::traceEverOn(true);
  top->trace(tfp, 9);
  tfp->open("waveform.vcd");
  #endif
  cpu_rst();
}

void finish_verilate(){
    delete top;
    #ifdef CONFIG_VCD
    tfp->close();
    #endif
    delete contextp;
}

CPU_state cpu = {};
NPCState npc_state = { NPC_STOP, 0, 0 };

int is_exit_status_bad() {
  int good = (npc_state.state == NPC_END && npc_state.halt_ret == 0) ||
    (npc_state.state == NPC_QUIT);
  return !good;
}

int main(int argc, char* argv[]) {
    init_verilate(argc,argv);
    init_monitor(argc,argv);
    sdb_mainloop();
    finish_verilate();
    // return 0;
    return is_exit_status_bad();
}
