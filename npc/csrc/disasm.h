#ifndef __DISAMS_H__
#define __DISAMS_H__

#ifdef __cplusplus
extern "C" {
#endif

void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
void init_disasm(const char *triple);

#ifdef __cplusplus
}
#endif

#endif // DISAMS_H
