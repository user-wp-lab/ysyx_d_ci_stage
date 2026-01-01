#include "../include/cpu.h"
#include "../include/debug.h"
#include "../include/paddr.h"
#include "../include/macro.h"
#include "../include/isa.h"
#include "../include/utils.h"
#include "../include/globals.h"
#include "../include/difftest.h"
#include "sdb.h"
#include "disasm.h"
#include <locale.h>
#include <time.h>
#ifdef CONFIG_NVBOARD
#include <nvboard.h>
#endif

#define MAX_INST_TO_PRINT 20
uint64_t get_time();
uint64_t g_nr_guest_inst = 0;
static bool g_print_step = false;
static uint64_t g_timer = 0;
int flag = 0;
// int no_ebreak = 0;
// extern "C" void notify_ebreak(){
//   no_ebreak = 1;
// }
void end_simulation(uint32_t pc, int32_t code){
  NPCTRAP(pc, code);
	Verilated::gotFinish(true);
}

// extern "C" void flash_read(int32_t addr, int32_t *data) { assert(0); }
#ifdef CONFIG_FLASH

extern "C" void flash_read(int32_t addr, int32_t *data) { 
  if (addr >= 0x00000000 & addr <= 0x0fffffff)
  {
    // printf("flash addr = %08x\n",addr);
    addr = addr + 0x30000000;
    uint32_t num = paddr_read(addr,4);
    // uint32_t new_num = 0;
    // new_num |= (num & 0x000000FF) << 24;
    // new_num |= (num & 0x0000FF00) << 8;
    // new_num |= (num & 0x00FF0000) >> 8;
    // new_num |= (num & 0xFF000000) >> 24;
    // *data = new_num;
    *data = num;
    // printf("flash\n addr = %08x\n",addr);
  }
  return;
 }
#endif

extern "C" void mrom_read(int32_t addr, int32_t *data) { 
  // printf("addr = %08x\n",addr);
  addr = addr & ~0x3u;
  *data = paddr_read(addr,4);
}

#ifdef CONFIG_SDRAM
extern "C" int sdram_read1(int bankaddr, int row, int col){
  uint32_t data = paddr_sdram_read1(bankaddr,row,col);
  return data;
}
extern "C" int sdram_read2(int bankaddr, int row, int col){
  uint32_t data = paddr_sdram_read2(bankaddr,row,col);
  return data;
}
extern "C" void sdram_write1(int bankaddr, int row, int col, unsigned int data, int mask){
  paddr_sdram_write1(bankaddr,row,col,data,mask);
}
extern "C" void sdram_write2(int bankaddr, int row, int col, unsigned int data, int mask){
  paddr_sdram_write2(bankaddr,row,col,data,mask);
}
#endif

extern "C" int memory_read(int raddr) {
  // 总是读取地址为`raddr & ~0x3u`的4字节返回
  //raddr = raddr & ~0x3u; //未进行字节对齐
  #ifdef CONFIG_DEVICE   //clint is corupted with sdram
  int offset = raddr - 0x02000048;
  if(raddr == 0x0200004c | raddr == 0x02000048){
    flag = 2;
    uint64_t us = get_time();
    uint32_t ret = us >> 32;
    if(offset == 4){
      return ret;
    }
    else {
      return (uint32_t)us;
    }
  }
  else {
    flag = 0;
  }
  #endif
  return paddr_read(raddr,4);
}
extern "C" void memory_write(int waddr, int wdata, char wmask) {
  // int offset = 8*(waddr % 4);
  // waddr = waddr & ~0x3u;  未进行字节对齐
#ifdef CONFIG_DEVICE
  if(waddr == 0xa00003f8){
    flag = 2;
    char ch = (char)wdata;
    putchar(ch);
    fflush(stdout);
    return;
  }
  else {
    flag = 0;
  }
#endif
  int temp = paddr_read(waddr,4);
  if (wmask == 1)
  {
    temp = (temp & ~0xff) | (wdata & 0xff);
  }else if (wmask == 3)
  {
    temp = (temp & ~0xffff) | (wdata & 0xffff);
  }else if (wmask == 15)
  {
    temp = (temp & ~0xffffffff) | (wdata & 0xffffffff);
  } 
  paddr_write(waddr,4,temp);
}

#ifdef CONFIG_CAHCE
uint32_t cache_data_miss = 0;
uint32_t cache_miss_cycle = 0;
uint32_t cache_access_cycle = 0;
uint32_t cache_miss_cycle_total = 0;
uint32_t cache_cycle = 0;
extern "C" void CACHE_PMC(uint32_t cache_miss, uint32_t ifu_cache_fetch, uint32_t cache_ifu_valid){
    if(ifu_cache_fetch && !cache_ifu_valid) {
      cache_cycle ++;
      if(cache_miss){
        cache_miss_cycle++;
      }
    }else if(ifu_cache_fetch && cache_ifu_valid){
      if(cache_miss_cycle) cache_data_miss++;
      cache_access_cycle += cache_cycle - cache_miss_cycle;
      cache_miss_cycle_total += cache_miss_cycle;
      cache_cycle = 0;
      cache_miss_cycle = 0;
    }
}

uint32_t ifu_req_con = 0;  //IFU请求指令的时间
uint32_t ifu_con = 0;  //IFU取到指令的次数
extern "C" void IFU_PMC(uint32_t ifu_fetch, uint32_t cache_valid) {
    if(ifu_fetch && !cache_valid) ifu_req_con = 1;
    else ifu_req_con = 0;
    if(ifu_fetch && cache_valid){
      ifu_con++;
    }
}
int inst_type1 = 0;
uint32_t idu_mem = 0;
uint32_t idu_mem_cycle = 0;
uint32_t idu_csr = 0;
uint32_t idu_csr_cycle = 0;
uint32_t idu_shift = 0;
uint32_t idu_shift_cycle = 0;
uint32_t idu_arith = 0;
uint32_t idu_arith_cycle = 0;
uint32_t idu_logic = 0;
uint32_t idu_logic_cycle = 0;
uint32_t idu_comp = 0;
uint32_t idu_comp_cycle = 0;
uint32_t idu_branch = 0;
uint32_t idu_branch_cycle = 0;
uint32_t idu_jal = 0;
uint32_t idu_jal_cycle = 0;
uint32_t idu_environment = 0;
uint32_t idu_environment_cycle = 0;
uint32_t idu_load = 0;
uint32_t idu_store = 0;
extern "C" void IDU_PMC(uint32_t load,uint32_t store, uint32_t shift, 
uint32_t arithmetic, uint32_t logic_op, uint32_t lui, uint32_t auipc, uint32_t compare_reset,
uint32_t branch, uint32_t jal, uint32_t jalr, uint32_t csr_ecall, uint32_t csr_mret, uint32_t csr){
    if(load || store){
      idu_mem++;
      if(load){
        idu_load++;
      }else if (store)
      {
        idu_store++;
      }
      inst_type1 = 1;
    }else if(shift){
      idu_shift++;
      inst_type1 = 2;
    }else if(arithmetic || lui || auipc){
      idu_arith++;
      inst_type1 = 3;
    }else if(logic_op){
      idu_logic++;
      inst_type1 = 4;
    }else if(compare_reset){
      idu_comp++;
      inst_type1 = 5;
    }else if(branch){
      idu_branch++;
      inst_type1 = 6;
    }else if(jal || jalr){
      idu_jal++;
      inst_type1 = 7;
    }else if(csr_ecall){
      idu_environment++;
      inst_type1 = 8;
    }else if(csr_mret || csr){
      idu_csr++;
      inst_type1 = 9;
    }
}

uint32_t exu_alu = 0;
extern "C" void EXU_PMC(uint32_t ervalid,uint32_t erready){
  if(ervalid && erready){
    exu_alu++;
  }
}
uint32_t lsu_req_con = 0;
uint32_t lsu_con_r = 0;
uint32_t lsu_con_w = 0;
extern "C" void LSU_PMC(uint32_t lsu_req, uint32_t lrvalid, uint32_t lrready, uint32_t lwvalid, uint32_t lwready) {
    if(lsu_req && !(lrvalid && lrready)) lsu_req_con = 1;
    else lsu_req_con = 0;
    if(lrvalid && lrready){
      lsu_con_r++;
    }
    else if (lwvalid && lwready)
    {
      lsu_con_w++;
    }
}
#endif

void ftrace_debug(Decode *s, vaddr_t dnpc);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  
#ifdef CONFIG_WATCHPOINT 
  if(CONFIG_WATCHPOINT){
    if(scan_watchpoint()){
         npc_state.state = NPC_STOP;
    }
  }
#endif
}

void update_cpu_registers() {
    for (int i = 0; i < 32; i++) {
        cpu.gpr[i] = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__regfile__DOT__rf[i];
    }
    // cpu.csr[0x300] = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__wbu__DOT__CSR_mstatus;  //mstatus
    // cpu.csr[0x305] = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__wbu__DOT__CSR_mtvec;  //mtvec
    // cpu.csr[0x341] = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__wbu__DOT__CSR_mepc;  //mepc
    // cpu.csr[0x342] = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__wbu__DOT__CSR_mcause;  //mcause
}

static void execute(Decode *s) {
    while (!Verilated::gotFinish()) {
        contextp->timeInc(1);
        top->clock = !top->clock;
        top->eval();
        #ifdef CONFIG_VCD
        tfp->dump(contextp->time());
        #endif 
        s->pc = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__PC;
        s->dnpc = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__next_pc;
        cpu.pc = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__next_pc;  
        s->isa.inst.val = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__inst;
        
#ifdef CONFIG_ITRACE
        char *p = s->logbuf;
        p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
        int ilen = 4;
        int i;
        uint8_t *inst = (uint8_t *)&s->isa.inst.val;
        for (i = ilen - 1; i >= 0; i --) {
          p += snprintf(p, 4, " %02x", inst[i]);
        }
        int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
        int space_len = ilen_max - ilen;
        if (space_len < 0) space_len = 0;
        space_len = space_len * 3 + 1;
        memset(p, ' ', space_len);
        p += space_len;
        #ifndef CONFIG_ISA_loongarch32r
        disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
            MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
        #else
        #endif
#endif
        contextp->timeInc(1);
        top->clock = !top->clock;
        top->eval();
        #ifdef CONFIG_VCD
        tfp->dump(contextp->time());
        #endif
        update_cpu_registers();
        if (s->pc== 0x800002e0)
        {
          end_simulation(s->pc,cpu.gpr[10]);
        }
        // if(no_ebreak == 1){
        //     NPCTRAP(s->pc, cpu.gpr[10]);
        //   	Verilated::gotFinish(true);
        // }
        #ifdef CONFIG_NVBOARD
        nvboard_update();
        #endif
        return;
    }
    if (Verilated::gotFinish()) {
        return;
    }
    return; 
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

#ifdef CONFIG_IRINGBUF
#define RINGBUF_SIZE 500
static char ringbuf[RINGBUF_SIZE] = {'\0'};
static void iringbuf(Decode *s)
{
    if (npc_state.state == NPC_ABORT)
    {
      printf("%s",ringbuf);
      printf("--> %s\n",s->logbuf);
    }
    int ringbuf_len = strlen(ringbuf);
    int logbuf_len = strlen(s->logbuf);
    memcpy(ringbuf + ringbuf_len, s->logbuf, logbuf_len);
    ringbuf[ringbuf_len + logbuf_len] = '\n';  // 添加换行符
    if(strlen(ringbuf) >=RINGBUF_SIZE-100 || npc_state.state == NPC_ABORT)    //将满时，清空缓存区
    {
      memset(ringbuf, '\0', RINGBUF_SIZE);
    } 
}
#endif

int a = 0;
uint32_t diff_pc[3] = {};
#ifdef CONFIG_IPC
uint32_t before_pc = RESET_VECTOR;
uint64_t cycle_count = 0;
uint64_t inst_count = 0;
uint64_t lsu_cycle_count = 0;
uint64_t ifu_cycle_count = 0;
#endif
void cpu_exec(uint64_t n){
    Decode s;
    g_print_step = (n < MAX_INST_TO_PRINT);
    switch (npc_state.state) {
    case NPC_END: case NPC_ABORT:
      printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
      return;
    default: npc_state.state = NPC_RUNNING;
    }

    uint64_t timer_start = get_time();

    int count = 0;
    for (;n > 0; n --) 
    {
      execute(&s);
      g_nr_guest_inst++;
      trace_and_difftest(&s, cpu.pc);
      #ifdef CONFIG_FTRACE
      ftrace_debug(&s, cpu.pc);
      #endif
      #ifdef CONFIG_IRINGBUF
      iringbuf(&s);
      #endif
      #ifdef CONFIG_DIFFTEST
      if(s.pc != diff_pc[0]){
        diff_pc[2] = diff_pc[1];
        diff_pc[1] = diff_pc[0];
        diff_pc[0] = s.pc;
        if(count == 1 && flag != 2 && diff_pc[2] >= 0x30000000){
            difftest_step(diff_pc[2], s.dnpc);
        }
        else {
          diff_pc[2] = diff_pc[1];
          diff_pc[1] = diff_pc[0];
          count++;
        }
      }
      #endif
      #ifdef CONFIG_IPC
      cycle_count++;
      if(s.pc != before_pc){
        before_pc = s.pc;
        inst_count++;
      }
      if(lsu_req_con){
        lsu_cycle_count++;
      }
      if (ifu_req_con)
      {
        ifu_cycle_count++;
      }
      // if ((s.pc >= 0x0f000000)  & (s.pc <= 0x0f00002C) & (a==0))
      // {
      //   if(s.pc == 0x0f00002C) a = 1;
      //   printf("pc = 0x%08x, ifu_cycle_count = %ld\n",s.pc,ifu_cycle_count);
      // }
      

      if(inst_type1 == 1){
        idu_mem_cycle++;
      }else if(inst_type1 == 2){
        idu_shift_cycle++;
      }else if(inst_type1 == 3){
        idu_arith_cycle++;
      }else if(inst_type1 == 4){
        idu_logic_cycle++;
      }else if(inst_type1 == 5){
        idu_comp_cycle++;
      }else if(inst_type1 == 6){
        idu_branch_cycle++;
      }else if(inst_type1 == 7){
        idu_jal_cycle++;
      }else if(inst_type1 == 8){
        idu_environment_cycle++;
      }else if(inst_type1 == 9){
        idu_csr_cycle++;
      }
      #endif

      if (npc_state.state != NPC_RUNNING) break;
    }
    
    #ifdef CONFIG_IPC
    printf(ANSI_FMT("NPC : IPC         = %10.6f | inst_number = %-10ld\n", ANSI_FG_YELLOW), 
           (double)inst_count / cycle_count, inst_count);
    #endif
    #ifdef CONFIG_PMC
    double cache_hit = (double)(ifu_con-cache_data_miss)*100/ifu_con;
    double access_time = (double)cache_access_cycle / ifu_con;
    double miss_penalty = (double)cache_miss_cycle_total / cache_data_miss;
    double amat = access_time + (1-cache_hit/100)*miss_penalty;
    printf(ANSI_FMT("NPC : CACHE_HIT   = %8.3f %% | CACHE_ACCESS_TIME = %-10.2f | CACHE_MISS_TIME = %-10.2f | AMAT = %-10.2f\n", ANSI_FG_YELLOW), 
          cache_hit, access_time, miss_penalty, amat);
    printf(ANSI_FMT("NPC : IFU_INST    = %10u | IFU_fetch_TIME = %-10.4f\n", ANSI_FG_YELLOW), 
           ifu_con, (double)ifu_cycle_count / ifu_con);

    printf(ANSI_FMT("NPC : IDU_MEM     = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_mem, (double)idu_mem / inst_count, (double)idu_mem_cycle / idu_mem);
    printf(ANSI_FMT("NPC : IDU_LOAD    = %10u | INST_PRO = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_load, (double)idu_load / inst_count);
    printf(ANSI_FMT("NPC : IDU_STORE   = %10u | INST_PRO = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_store, (double)idu_store / inst_count);

    printf(ANSI_FMT("NPC : IDU_SHIFT   = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_shift, (double)idu_shift / inst_count, (double)idu_shift_cycle / idu_shift);
    printf(ANSI_FMT("NPC : IDU_ARITH   = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_arith, (double)idu_arith / inst_count, (double)idu_arith_cycle / idu_arith);
    printf(ANSI_FMT("NPC : IDU_LOGIC   = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_logic, (double)idu_logic / inst_count, (double)idu_logic_cycle / idu_logic);
    printf(ANSI_FMT("NPC : IDU_COMP    = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_comp, (double)idu_comp / inst_count, (double)idu_comp_cycle / idu_comp);
    printf(ANSI_FMT("NPC : IDU_BRANCH  = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_branch, (double)idu_branch / inst_count, (double)idu_branch_cycle / idu_branch);
    printf(ANSI_FMT("NPC : IDU_JAL_R   = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_jal, (double)idu_jal / inst_count, (double)idu_jal_cycle / idu_jal);
    printf(ANSI_FMT("NPC : IDU_ENVIRON = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_environment + 1, (double)(idu_environment + 1) / inst_count, (double)idu_environment_cycle / (idu_environment + 1));
    printf(ANSI_FMT("NPC : IDU_CSR     = %10u | INST_PRO = %-10.3f | EXCUT_TIME = %-10.3f\n", ANSI_FG_YELLOW), 
           idu_csr, (double)idu_csr / inst_count, (double)idu_csr_cycle / idu_csr);

    printf(ANSI_FMT("NPC : EXU_PMC     = %10u\n", ANSI_FG_YELLOW), 
           exu_alu);
    printf(ANSI_FMT("NPC : LSU_MEM_R   = %10u | LSU_MEM_W = %-10u\n", ANSI_FG_YELLOW), 
           lsu_con_r, lsu_con_w);
    double lsu_time = (double)lsu_cycle_count / cycle_count;
    double lsu_mem_time = (double)lsu_cycle_count / (lsu_con_r+lsu_con_w);
    printf(ANSI_FMT("NPC : LSU_MEM_TIME = %-10.4f | LSU_TIME(in total inst_cycle) = %-10.4f(load+store)\n", ANSI_FG_YELLOW), 
           lsu_mem_time, lsu_time);
  #endif


    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (npc_state.state) {
    case NPC_RUNNING: npc_state.state = NPC_STOP; break;

    case NPC_END: case NPC_ABORT:
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
    case NPC_QUIT: statistic();
  }
}
