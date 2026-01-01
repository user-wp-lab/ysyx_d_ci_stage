AM_SRCS := riscv/ysyxSoC/start.S \
           riscv/ysyxSoC/trm.c \
           riscv/ysyxSoC/ioe.c \
           riscv/ysyxSoC/timer.c \
           riscv/ysyxSoC/input.c \
           riscv/ysyxSoC/gpu.c \
           riscv/ysyxSoC/cte.c \
           riscv/ysyxSoC/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections
LDFLAGS   += -T $(AM_HOME)/scripts/soclinker.ld \
						 --defsym=_pmem_start=0x30000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start 
# LDFLAGS  += --print-map 
#CLINT外设地址与SDRAM冲突了
#运行用soc的bin运行nemu，测试cachesim，启动时要打开nemu的E选项，以及各个存储空间，flash改为0x30000000开头
#$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin
NPCFLAGS += -b
# NPCFLAGS += -f $(IMAGE).elf
ADD_FLAGS = $(YSYXSOC_HOME)/ready-to-run/D-stage/new.bin

CFLAGS += -DMAINARGS=\"$(mainargs)\"
.PHONY: $(AM_HOME)/am/src/riscv/ysyxSoC/trm.c 

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: image
	$(MAKE) -C $(NPC_HOME) run ARGS="$(NPCFLAGS)" IMG=$(ADD_FLAGS)

gdb: image
	$(MAKE) -C $(NPC_HOME) gdb ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

