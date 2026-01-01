#include "../include/isa.h"
#include "../include/common.h"
#include "../include/globals.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display(){
  printf("The value of all reg is: \n");
	printf("  name\t  value\n");
	for(int i=0;i<32;i++){
		printf("   %s:\t0x%08x\n",regs[i],cpu.gpr[i]);
	}
	printf("\n");
}

word_t isa_reg_str2val(const char *s, bool *success) {
       for(int k=0;k<32;k++){
       		if(strcmp(regs[k],s) == 0){
          *success = true;
          return gpr(k);
        }else if(strcmp(s,"0") == 0){
          *success = true;
          return gpr(0);
        }
      }
      *success = false;
      printf("No such reg!\n");
      return -1; 
}

int check_reg_idx(int idx){
  return idx;
}

const char *reg_name(int idx){
  return regs[idx];
}
