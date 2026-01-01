#ifndef __MACRO_H__
#define __MACRO_H__

#define CONFIG_ISA_riscv 1
#define CONFIG_MSIZE (0xfffffff)
#define CONFIG_MBASE (0x30000000)
#define CONFIG_PC_RESET_OFFSET 0x0
#define PMEM_LEFT  ((uint32_t)CONFIG_MBASE)
#define PMEM_RIGHT ((uint32_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

// #define CONFIG_NVBOARD 1
// #define CONFIG_IPC 1
// #define CONFIG_PMC 1
#define CONFIG_FLASH
// #define CONFIG_SDRAM
// #define CONFIG_CACHE
#define CONFIG_VCD  1
// #define CONFIG_TRACE 1
// #define CONFIG_ITRACE 1
// #define ITRACE_COND 1
// #define CONFIG_ITRACE_COND 1
// #define CONFIG_TARGET_NATIVE_ELF 1
// #define CONFIG_TRACE_START 0
// #define CONFIG_TRACE_END 10000
// #define CONFIG_WATCHPOINT 1
// #define CONFIG_FTRACE 1
// #define CONFIG_MTRACE 1
// #define CONFIG_IRINGBUF 1

// #define CONFIG_DIFFTEST 1
#define CONFIG_DEVICE 1
#define CONFIG_TIMER_GETTIMEOFDAY 1

#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)

#define CHOOSE2nd(a, b, ...) b
#define MUX_WITH_COMMA(contain_comma, a, b) CHOOSE2nd(contain_comma a, b)
#define MUX_MACRO_PROPERTY(p, macro, a, b) MUX_WITH_COMMA(concat(p, macro), a, b)

#define __P_DEF_0  X,
#define __P_DEF_1  X,
#define __P_ONE_1  X,
#define __P_ZERO_0 X,

#define MUXDEF(macro, X, Y)  MUX_MACRO_PROPERTY(__P_DEF_, macro, X, Y)
#define MUXNDEF(macro, X, Y) MUX_MACRO_PROPERTY(__P_DEF_, macro, Y, X)
#define MUXONE(macro, X, Y)  MUX_MACRO_PROPERTY(__P_ONE_, macro, X, Y)
#define MUXZERO(macro, X, Y) MUX_MACRO_PROPERTY(__P_ZERO_,macro, X, Y)

#define ISDEF(macro) MUXDEF(macro, 1, 0)
#define ISNDEF(macro) MUXNDEF(macro, 1, 0)
#define ISONE(macro) MUXONE(macro, 1, 0)
#define ISZERO(macro) MUXZERO(macro, 1, 0)
#define isdef(macro) (strcmp("" #macro, "" str(macro)) != 0)


#define __IGNORE(...)
#define __KEEP(...) __VA_ARGS__
#define IFDEF(macro, ...) MUXDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
#define IFNDEF(macro, ...) MUXNDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
#define IFONE(macro, ...) MUXONE(macro, __KEEP, __IGNORE)(__VA_ARGS__)
#define IFZERO(macro, ...) MUXZERO(macro, __KEEP, __IGNORE)(__VA_ARGS__)

#endif
