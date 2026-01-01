#ifndef __ISA_H__
#define __ISA_H__

#include "common.h"

#define gpr(idx) (cpu.gpr[idx])

void isa_reg_display();
word_t isa_reg_str2val(const char *s, bool *success);

const char *reg_name(int idx);
int check_reg_idx(int idx);

void init_isa();

#endif