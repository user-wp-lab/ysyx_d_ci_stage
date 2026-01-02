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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
// this should be enough

static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static void gen_num() {
    unsigned num = rand() % 1000 + 1;
    sprintf(buf + strlen(buf), "%u", num);
}

static void gen(char c) {
    if (strlen(buf) + 1 < sizeof(buf)) {
        sprintf(buf + strlen(buf), "%c", c);
    }
}

static void gen_rand_op() {
    char operator[] = {'+', '-', '*', '/'};
    if (strlen(buf) + 1 < sizeof(buf)){
      		sprintf(buf + strlen(buf), "%c",operator[rand() % 4]);
    }
}

static int choose(int number) {
    return rand() % number;
}

static void gen_rand_expr_pro(int k) {
	if(k<5){
		k = k + 1;
	}else{
		gen_num();
		return;
	}
    switch (choose(5)) {
    case 0:
	    if(strlen(buf) +1 < sizeof(buf)){
        	gen_num();
	    }
        break;
    case 1:
        if (strlen(buf) + 3 < sizeof(buf)) {
            gen('(');
            gen_rand_expr_pro(k);
            gen(')');
        }
        break;
    default:
        if (strlen(buf) + 3 < sizeof(buf)) {
            gen_rand_expr_pro(k);
            gen_rand_op();
	    gen_rand_expr_pro(k);
        }
        break;
    }
}

static int gen_rand_expr() {
    buf[0] = '\0'; // 清空缓冲区
    int k = 0;
    gen_rand_expr_pro(k);
     if (strcmp(buf, "0") == 0 || strstr(buf, "/0") != NULL) {
        return -1; 
    }

}
  
  
  //随机插入空格
  //int len = strlen(buf);
  // for(int i = 0; i < len;i++){
  //    if(rand()% 4 == 0 && len+1 < 65536){
  //    	for(int k = 0; k < len-i ;k++){	
  //	      buf[i+1] = buf[i];
  //	}
  //	buf[i]= ' ';
  //    }
  //}


int main(int argc, char *argv[]) {
  int th_val = 1000000;
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    if(gen_rand_expr() == -1){continue;}

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp!= NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    unsigned result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);	
    if(result >= th_val){continue;}
    printf("%u %s\n", result, buf);
  }
  return 0;
}
