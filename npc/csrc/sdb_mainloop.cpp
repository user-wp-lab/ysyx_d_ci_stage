#include "../include/common.h"
#include "../include/cpu.h"
#include "../include/isa.h"
#include "../include/paddr.h"
#include "../include/globals.h"
#include "sdb.h"
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
static int is_batch_mode = false;

static char* rl_gets() {
  static char *line_read = NULL;
  if (line_read) {
    free(line_read);
    line_read = NULL;
  }
  line_read = readline("(npc) ");
  if (line_read && *line_read) {
    add_history(line_read);
  }
  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);  //uint32_t = 4294967295 只够运行2h40min
  return 0;
}

static int cmd_q(char *args) {
  npc_state.state = NPC_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	int N;
	if(args == NULL){
	    N = 1;
	}else{
	    N = strtol(args,NULL,10);
	}
	cpu_exec(N);
	if(npc_state.state == NPC_STOP)printf("Program runs %d steps \n",N);
	return 0;
}

static int cmd_info(char *args){
	if(args == NULL){

	}else if(strcmp(args,"r") == 0) {
    isa_reg_display();
  }else if(strcmp(args,"w") == 0){
    //print watchpoint information! 
	  list_watchpoint();
  }else{      //### "w" ###
	  printf("Invalid parameter\n");
	}
	return 0;
}

static int cmd_x(char *args){
    if(args == NULL){
      return 0;
	  }
    uint32_t len;
    uint32_t addr;
    sscanf(args,"%u %x",&len,&addr);
    for(int i=0; i<len; i++){
        uint32_t temp = paddr_read(addr,4);
        if(temp) printf("0x%x :  0x%08x\n",addr,temp);
        else return 0;
        addr += 4;
    }
    return 0;
}

static int cmd_p(char *args){
    if(strcmp(args,"pc") == 0){
      printf("pc = 0x%08x\n",top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__PC);
      return 0;
    }
    bool flag = true;
    word_t result = expr(args,&flag);
	  if(result == 4294967295){
	  	printf("ERROR!\n");
		  return 0;
	  }else if((result != 4294967295) && (flag == false)){
	  	return 0;
	  }
	  printf("result(DEC) : %u\n",result);
	  printf("result(HEX) : 0x%08x\n",result);
	  return 0;
}

static int cmd_w(char *args){
  set_watchpoint(args);
	return 0;
}

static int cmd_d(char *args){
  int num_d = strtol(args,NULL,10);
	bool tt = delete_watchpoint(num_d);
	if(tt == true){
		printf("成功删除节点%d\n",num_d);
	}else{
		printf("未能删除节点\n");
	}
	return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si", "[N]  Pause execution after the program steps N instructions", cmd_si },
  {"info", "[SUBCMD]  Print register status", cmd_info },
  {"x","[N] [EXPR]  Scan memory",cmd_x},
  {"p","[EXPR] Caculate the value of expr",cmd_p},
  {"w","[EXPR] set watchpoint for expr",cmd_w},
  {"d","[NO] delete watchpoint with nember NO",cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  char *arg = strtok(NULL, " ");
  int i;
  if (arg == NULL) { 
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop()
{
    if (is_batch_mode) {
        cmd_c(NULL);
        return;
    }
    for (char *str; (str = rl_gets()) != NULL; ) {
        char *str_end = str + strlen(str);
        char *cmd = strtok(str, " ");
        if (cmd == NULL) { continue; }
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }
        int i;
        for (i = 0; i < NR_CMD; i ++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
              if (cmd_table[i].handler(args) < 0) { return; }
              break;
            }
        }
        if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
    }
}

