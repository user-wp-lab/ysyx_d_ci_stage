ENTRY(_start)
MEMORY {
    FLASH   : ORIGIN = 0x30000000, LENGTH = 16M
    SDRAM   : ORIGIN = 0xa0000000, LENGTH = 32M 
    SRAM    : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS {
    . = _pmem_start + _entry_offset;

    .fsbl : {
        *(entry)
        *(.fsbl*)
    } > FLASH

    .bootloader : {
        _bootloader_start = .;
        *(.bootloader*)
        _bootloader_end = .;
        _bootloader_load = LOADADDR(.bootloader);
    } > SRAM AT> FLASH 
    
    /* FLASH 中存放代码段 */
    .text : {
        _text_start = .;
        *(.text*)
        _text_end = .;
        _text_load = LOADADDR(.text);
    } > SDRAM AT> FLASH

    /* SDRAM 中存放可写数据段和栈区 */
    .data : {
        _data_start = .;
        *(.data*)
        *(.sdata*)
        _data_end = .;
        _data_load = LOADADDR(.data);
    } > SDRAM AT> FLASH

    /* SDRAM 中存放只读数据段 */
    .rodata : {
        _rodata_start = .;
        *(.rodata*)
        *(.srodata*)  
        _rodata_end = .;
        _rodata_load = LOADADDR(.rodata);
    } > SDRAM AT> FLASH

    .bss : {
        _bss_start = .;
        *(.bss*)
        *(.sbss*)
        *(.scommon)
        *(COMMON)
        _bss_end = .;
    } > SDRAM

    _stack_end = ORIGIN(SDRAM) + LENGTH(SDRAM);  /* 栈结束地址，SRAM 末尾 */
    _stack_start = _stack_end - 0x400000;        /* 设置栈的大小为 128KB 足够microbench等测试 */

    _heap_start = _bss_end;                /* 堆的起始地址，SRAM 开始 */
    _heap_end = _heap_start + 0x400000;     /* 堆的结束地址为栈的起始地址 */

    /* 定义堆栈指针 */
    _stack_pointer = _stack_end;
}
