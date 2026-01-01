#include <am.h>
#include <klib-macros.h>
#include "/home/wp/桌面/ysyx-workbench/abstract-machine/am/src/riscv/riscv.h"
// #define RT_thread

extern char _heap_start;
extern char _heap_end;

extern char _rodata_start, _rodata_end, _rodata_load;
extern char _data_start, _data_end, _data_load;

extern char _text_start, _text_end, _text_load;
extern char _bss_start, _bss_end;
#ifdef RT_thread
extern char _bss_extra_start, _bss_extra_end;
extern char _data_extra_start, _data_extra_end, _data_extra_load;
#endif
extern char _bootloader_start, _bootloader_end, _bootloader_load;

int main(const char *args);
void _trm_init();
void _bootloader_entry();

Area heap = RANGE(&_heap_start, &_heap_end);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

#define UART_PORT   0x10000000
#define UART_DLL    (UART_PORT + 0)  // 除数寄存器低8位
#define UART_DLM    (UART_PORT + 1)  // 除数寄存器高8位
#define UART_FIFO   (UART_PORT + 2)  // FIFO控制寄存器
#define UART_LCR    (UART_PORT + 3)  // 线路控制寄存器
#define UART_LSR    (UART_PORT + 5)  // 线路状态寄存器

#define UART_LSR_THRE 0x20           // Transmit FIFO is Empty
#define UART_LCR_DLAB 0x80           // 除数锁存访问位

void uart_init() {
  // 使能 DLAB 来访问 DLL 和 DLM 寄存器
  outb(UART_LCR, UART_LCR_DLAB);

  // 设置除数寄存器，仿真中无关实际波特率，设为任意值
  outb(UART_DLM, 0x00);  // 除数高位,先写高位
  outb(UART_DLL, 0x01);  // 除数低位

  // 禁用 DLAB 并设置8位数据、无校验、1个停止位
  outb(UART_LCR, 0x03);

  // outb(UART_FIFO, 0xC0);
}

void putch(char ch) {
  while (!(inb(UART_LSR) & UART_LSR_THRE)) {
  }
  outb(UART_PORT, ch);
}

void halt(int code) {
	asm volatile("mv a0, %0; ebreak" : :"r"(code));
  while (1);
}

/*void int_to_str(uintptr_t num, char *buf) {
    int i = 0;
    int j;
    char temp;
    if (num == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }
    // 反向存储数字
    while (num > 0 && i < 15) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    buf[i] = '\0';
    // 反转字符串
    for (j = 0; j < i / 2; j++) {
        temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}

void csr_read() {
  uintptr_t mvendorid = 0;
  uintptr_t marchid = 0;
  
  // 读取 mvendorid 和 marchid 寄存器
  asm volatile("csrr %0, mvendorid" : "=r"(mvendorid));
  asm volatile("csrr %0, marchid" : "=r"(marchid));
  
  char vendor_str[5]; // 4个字符 + 1个终止符
  vendor_str[0] = (mvendorid >> 24) & 0xFF;
  vendor_str[1] = (mvendorid >> 16) & 0xFF;
  vendor_str[2] = (mvendorid >> 8) & 0xFF;
  vendor_str[3] = mvendorid & 0xFF;
  vendor_str[4] = '\0';
  
  for (size_t i = 0; i < 4; i++) {
      putch(vendor_str[i]);
  }
  char march_str[16];
  int_to_str(marchid, march_str);
  for (size_t i = 0; march_str[i] != '\0'; i++) {
      putch(march_str[i]);
  }
  putch('\n');
}*/

__attribute__((section(".fsbl"))) void _fsbl_init() {
  char *src = &_bootloader_load;
  char *dst = &_bootloader_start;
  while (dst < &_bootloader_end) {
    *dst++ = *src++;
  }
  _bootloader_entry();
}

__attribute__((section(".bootloader"))) void _bootloader_entry() {
  char *src = &_text_load;
  char *dst = &_text_start;
  while (dst < &_text_end) {
    *dst++ = *src++;
  }

  #ifdef RT_thread
  char *src3 = &_data_extra_load;  // 额外数据段(rt-thread)
  char *dst3 = &_data_extra_start;
  while (dst3 < &_data_extra_end) {
    *dst3++ = *src3++;
  }
  #endif
   
  char *src1 = &_data_load;         // 将数据段从 MROM 复制到 SRAM
  char *dst1 = &_data_start;
  while (dst1 < &_data_end) {
    *dst1++ = *src1++;
  }
  
  char *src2 = &_rodata_load;       // 将数据段从 MROM 复制到 SRAM
  char *dst2 = &_rodata_start;
  while (dst2 < &_rodata_end) {
    *dst2++ = *src2++;
  }

  for (dst = &_bss_start; dst < &_bss_end; dst++)
  {
    *dst = 0;
  }

  #ifdef RT_thread
  for (dst = &_bss_extra_start; dst < &_bss_extra_end; dst++)
  {
    *dst = 0;   // 额外的bss段(rt-thread)
  }
  #endif
  
  _trm_init();
}

void _trm_init() {
  uart_init();          //开difftest会导致nemu识别到地址溢出
  char str[] = "trm_init start...\n";
  for (size_t i = 0; i < 18; i++) {
    putch(str[i]);
  }
  
  char str1[] = "bootloader start...\n";
  for (size_t i = 0; i < 20; i++) {
    putch(str1[i]);
  }
  // csr_read();

  char str2[] = "bootloader end...\n";
  for (size_t i = 0; i < 18; i++) {
    putch(str2[i]);
  }
  int ret = main(mainargs);
  halt(ret);
}
