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
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <elf.h>
#include <config/ftrace.h>   //或者采用全局变量
#include "/home/wp/桌面/ysyx-workbench/nemu/src/monitor/sdb/sdb.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 20

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();
void ftrace_debug(Decode *_this, vaddr_t dnpc);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT 
  if(CONFIG_WATCHPOINT){
    if(scan_watchpoint()){
         nemu_state.state = NEMU_STOP;
    }
  }
#endif
}


typedef struct FunctionSymbol{
	char *name;
	uint32_t addr;
}funsym;	
funsym *func_table = NULL;
int func_num = 0;
void init_ftrace(const char *elf_file)
{
	FILE *ft = fopen(elf_file,"rb");
	//assert(ft != NULL);

	Elf32_Ehdr ehdr;
	int ret = fread(&ehdr, 1, sizeof(ehdr), ft);
	
	int i;
	Elf32_Shdr *shdr = malloc(ehdr.e_shentsize*ehdr.e_shnum);
	fseek(ft, ehdr.e_shoff, SEEK_SET);
	ret = fread(shdr, ehdr.e_shentsize, ehdr.e_shnum, ft);
	

	char *strstb = NULL;
	size_t strstb_size = 0;
	for(i = 0;i < ehdr.e_shnum; i++)
	{
		if(shdr[i].sh_type == SHT_STRTAB && i != ehdr.e_shstrndx)
		{
			strstb_size = shdr[i].sh_size;
			strstb = malloc(strstb_size);
			fseek(ft, shdr[i].sh_offset, SEEK_SET);
			ret = fread(strstb, 1, strstb_size,ft);
			break;
		}	
	}	

	for(i = 0;i < ehdr.e_shnum; i++)
	{
		if(shdr[i].sh_type == SHT_SYMTAB)
		{
			Elf32_Sym *sym = malloc(shdr[i].sh_size);
			fseek(ft, shdr[i].sh_offset, SEEK_SET);
			ret = fread(sym, shdr[i].sh_size, 1, ft);
			if(ret){}
			int sym_cnt = shdr[i].sh_size/(sizeof(Elf32_Sym));
			for(int k = 0;k < sym_cnt; k++)
			{
				if(ELF32_ST_TYPE(sym[k].st_info) == STT_FUNC)
				{
					func_num++;
				}
			}
			func_table = malloc(func_num*sizeof(funsym));
			func_num = 0;
			for(int k = 0;k < sym_cnt; k++)
			{
				if(ELF32_ST_TYPE(sym[k].st_info) == STT_FUNC)
				{
					func_table[func_num].name = &(strstb[sym[k].st_name]);
					func_table[func_num].addr = sym[k].st_value;
					func_num++;
				}
			}	
		        free(sym);
		}
	}
	free(strstb);
	free(shdr);
	fclose(ft);
}

char *temp[120] = {'\0'};
static int wr_pos = 0;
static int put_space = 0;
void ftrace_debug(Decode *s, vaddr_t dnpc){
 	int cnt = 0;
	if((s->isa.inst.val & 0xffff707f) == 0x00000067){
            printf("0x%08x:",s->pc);
            put_space--;
            for(int i = 0;i < put_space;i++){
	        printf(" ");
            }
            printf("ret [%s]\n",temp[0]);
	    for(int i = 0;i < wr_pos;i++){
	    	temp[i] = temp[i+1];
	    }
	    wr_pos--;
            return;
        }
	for(cnt = 0;cnt < func_num;cnt++){
                if(func_table[cnt].addr == dnpc){
			if (wr_pos > 0) {
                            for (int k = wr_pos; k > 0; k--) {
                                temp[k] = temp[k - 1];
                            }
                        }
                        temp[0] = func_table[cnt].name;
                        wr_pos++;
                        put_space++;
                        if (wr_pos == 120) wr_pos = 0;
                        if (wr_pos == 0) temp[wr_pos] = func_table[cnt].name;
			
			printf("0x%08x:",s->pc);
			for(int i = 0;i < put_space;i++){
				printf(" ");
			}
			printf("call [%s@0x%08x]\n",func_table[cnt].name,dnpc);
			break;
		}
	}
}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
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
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}

#ifdef CONFIG_IRINGBUF
#define RINGBUF_SIZE 500
static char ringbuf[RINGBUF_SIZE] = {'\0'};
static void iringbuf(Decode *s)
{
    if (nemu_state.state == NEMU_ABORT)
    {
      printf("%s",ringbuf);
      printf("--> %s\n",s->logbuf);
    }
    int ringbuf_len = strlen(ringbuf);
    int logbuf_len = strlen(s->logbuf);
    memcpy(ringbuf + ringbuf_len, s->logbuf, logbuf_len);
    ringbuf[ringbuf_len + logbuf_len] = '\n';  // 添加换行符
    if(strlen(ringbuf) >=RINGBUF_SIZE-100 || nemu_state.state == NEMU_ABORT)    //将满时，清空缓存区
    {
      memset(ringbuf, '\0', RINGBUF_SIZE);
    } 
}
#endif

#ifdef CONFIG_CACHESIM
#define cache_way 8   //多少组
#define cache_set 4   //每组多少个
#define cache_line 4  //每块多大
#define cache_offset_size (cache_line/4) 
//cache_line为四个字节时，offset在元数据中占两位（00,01,10,11分别代表这四个字节，一般以四字节为单位读出）
//此处的cachesim通过cache_line设定cache_offset_size大小，则offset每越过四个字节切换一组cache

#define cache_n \
    ((cache_way == 1) ? 0 : \
     (cache_way == 2) ? 1 : \
     (cache_way == 4) ? 2 : \
     (cache_way == 8) ? 3 : \
     (cache_way == 16) ? 4 : \
     (cache_way == 32) ? 5 : 0)

#define cache_m \
    ((cache_line == 4) ? 2 : \
     (cache_line == 8) ? 3 : \
     (cache_line == 16) ? 4 : 0)

#ifdef CONFIG_CACHESIM_LRU
typedef struct {
    unsigned int tag;
    int valid;
    unsigned int last_use; // 用于 LRU 的时间戳
} CacheBlock;
unsigned int time_stamp = 0; // 全局时间戳，初始化为0
#else 
typedef struct {
    unsigned int tag;
    int valid;
} CacheBlock;
#endif

typedef struct {
    CacheBlock blocks[cache_set];
} CacheSet;    
CacheSet cache[cache_way][cache_offset_size];
unsigned int hit = 0;  //over wai
unsigned int miss = 0;

typedef struct {
    int ptr;
} fifo_ptr;
fifo_ptr fifo[cache_way];

int cache_set_i = 0;              //over wai
  #ifdef CONFIG_CACHESIM_LRU
    int cachesim(unsigned int pc){
      int cache_temp   = (32 - cache_n - cache_m);
      int tag    = pc >> (cache_n + cache_m);
      int off    = ((pc << (32 - cache_m)) >> (32 - cache_m));
      int offset = 0;
      if(off <= 3 && off >= 0){
        offset = 0;
      }else if (off <= 7 && off >= 4)
      {
        offset = 1;
      }else {
        offset = 0;  //只考虑4,8字节的情况
      }
      int index  = ((pc >> cache_m) << (cache_temp + cache_m)) >> (cache_temp + cache_m);

      for (cache_set_i = 0 ; cache_set_i < cache_set ; cache_set_i++){
          if((cache[index][offset].blocks[cache_set_i].tag == tag) && (cache[index][offset].blocks[cache_set_i].valid == 1)){
              hit ++;
              time_stamp++; // 更新时间戳
              cache[index][offset].blocks[cache_set_i].last_use = time_stamp; // 更新该块的最后使用时间
              cache_set_i = cache_set+1; 
          }
      }
      if(cache_set_i == cache_set){  //未命中
          miss++;
          cache_set_i = 0;
          int min_last_use = 0xFFFFFFFF; // 初始为最大值
          int replace_index = 0;
          for (cache_set_i = 0; cache_set_i < cache_set; cache_set_i++) {
              if (cache[index][offset].blocks[cache_set_i].valid == 1) {
                  // 有效块，比较 last_use
                  if (cache[index][offset].blocks[cache_set_i].last_use < min_last_use) {  //确保没有越界，同时找出last_use最小的块（最该被替换）
                      min_last_use = cache[index][offset].blocks[cache_set_i].last_use;
                      replace_index = cache_set_i;
                  }
              } else {
                  // 找到了无效块，优先替换
                  replace_index = cache_set_i;
                  break;
              }
          }

          // 插入新块
          cache[index][offset].blocks[replace_index].tag = tag;
          cache[index][offset].blocks[replace_index].valid = 1;
          time_stamp++;
          cache[index][offset].blocks[replace_index].last_use = time_stamp;
      }
      return 0;
    }
  #else 
    int cachesim(unsigned int pc){
      int cache_temp   = (32 - cache_n - cache_m);
      int tag    = pc >> (cache_n + cache_m);
      int off    = ((pc << (32 - cache_m)) >> (32 - cache_m));
      int offset = 0;
      if(off <= 3 && off >= 0){
        offset = 0;
      }else if (off <= 7 && off >= 4)
      {
        offset = 1;
      }else {
        offset = 0;  //只考虑4,8字节的情况
      }
      int index  = ((pc >> cache_m) << (cache_temp + cache_m)) >> (cache_temp + cache_m);

      for (cache_set_i = 0 ; cache_set_i < cache_set ; cache_set_i++){
          if((cache[index][offset].blocks[cache_set_i].tag == tag) && (cache[index][offset].blocks[cache_set_i].valid == 1)){
              hit ++;
              cache_set_i = cache_set+1; 
          }
      }
      if(cache_set_i == cache_set){  //未命中
          miss++;
          cache_set_i = 0;
          cache[index][offset].blocks[fifo[index].ptr].tag   = tag;
          cache[index][offset].blocks[fifo[index].ptr].valid = 1;
          fifo[index].ptr++;
          if(fifo[index].ptr >= cache_set){
              fifo[index].ptr = 0;
          }
      }
      return 0;
    }
  #endif
#endif

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    #ifdef CONFIG_CACHESIM
    cachesim(cpu.pc);
    #endif
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    #ifdef CONFIG_FTRACE
    ftrace_debug(&s, cpu.pc);
    #endif
    #ifdef CONFIG_IRINGBUF
    iringbuf(&s);
    #endif
    
    if (nemu_state.state != NEMU_RUNNING) {
      #ifdef CONFIG_CACHESIM
      printf("AMAT = %10.4f\n",(double)hit/(hit+miss)); 
      printf("hit = %d miss = %d\n",hit,miss);
      #endif
      break;
      }
    IFDEF(CONFIG_DEVICE, device_update());
  }
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

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
