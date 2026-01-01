#ifndef __SDB_H__
#define __SDB_H__

#include "../include/common.h"

uint32_t expr(char *e, bool *success);
void set_watchpoint(char *e);
bool delete_watchpoint(int NO);
void list_watchpoint();
int scan_watchpoint();

void init_regex();
void init_wp_pool();

#endif