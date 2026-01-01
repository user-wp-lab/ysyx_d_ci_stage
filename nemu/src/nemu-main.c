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

#include <common.h>
#include "monitor/sdb/sdb.h"
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
int gen_expr_test();

/*
int gen_expr_test(){
	FILE *fp = fopen("/home/wp/桌面/ysyx-workbench/nemu/tools/gen-expr/input","r");
	if( fp == NULL){
		printf("Empty file");
	}
	char s[1000];
	while(fgets(s,1000,fp) != NULL){
		printf("%s",s);
		char a[] = " ";
		bool test = true;
		char* token = strtok(s,a);
		if(token != NULL){
			token = strtok(NULL,a);
		}
		int result = expr(token,&test);
		printf("result = %d\n",result);
	}
	return 0;
}
*/

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();
  //gen_expr_test();
  return is_exit_status_bad();
}
