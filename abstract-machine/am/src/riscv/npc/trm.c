#include <am.h>
#include <klib-macros.h>

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

// uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
// uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
// uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }

void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
// void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
// void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }

#define SERIAL_PORT     (0xa0000000 + 0x00003f8)
#define KBD_ADDR        (0xa0000000 + 0x0000060)
#define VGACTL_ADDR     (0xa0000000 + 0x0000100)

void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
	asm volatile("mv a0, %0; ebreak" : :"r"(code));
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
