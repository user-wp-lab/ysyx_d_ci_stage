MODULE = ysyxSoCFull
IMG ?= 

ifeq ($(VERILATOR_ROOT),)
VERILATOR = verilator
VERILATOR_COVERAGE = verilator_coverage
else
export VERILATOR_ROOT
VERILATOR = $(VERILATOR_ROOT)/bin/verilator
VERILATOR_COVERAGE = $(VERILATOR_ROOT)/bin/verilator_coverage
endif

CXXFLAGS += -g       ###调试使用
CXXFLAGS += $(shell llvm-config --cxxflags) -fPIE

DIFF_LIBS += $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
LLVM_LIBS += $(shell llvm-config --libs)
LDFLAGS += -lasan $(LLVM_LIBS) -lreadline 

INCLUDE_FLAGS += -I$(YSYXSOC_HOME)/perip/uart16550/rtl
INCLUDE_FLAGS += -I$(YSYXSOC_HOME)/perip/spi/rtl

# VERILATOR_FLAGS += -Wall
VERILATOR_FLAGS += --trace
VERILATOR_FLAGS += --assert
VERILATOR_FLAGS += --top-module $(MODULE)
VERILATOR_FLAGS += -cc --exe
VERILATOR_FLAGS += $(INCLUDE_FLAGS)
VERILATOR_FLAGS += -CFLAGS "$(CXXFLAGS) -U__STDC_FORMAT_MACROS"
VERILATOR_FLAGS += -LDFLAGS "$(LDFLAGS)"
VERILATOR_FLAGS += --build 
VERILATOR_FLAGS += -MMD
VERILATOR_FLAGS += -j 4
VERILATOR_FLAGS += --timescale "1ns/1ns"
VERILATOR_FLAGS += --no-timing
VERILATOR_FLAGS += --autoflush
# VERILATOR_FLAGS += --trace-fst
#VERILATOR_FLAGS += -x-assign fast
#VERILATOR_FLAGS += --coverage
#VERILATOR_FLAGS += --debug
#VERILATOR_FLAGS += --gdbbt 
#VERILATOR_FLAGS += -Iobj_dir

VERILOG_SOURCES = $(wildcard vsrc/*.v)
VERILOG_SOURCES += $(shell find /home/wp/Desktop/ysyx-workbench/ysyxSoC/perip/ -name "*.v")
VERILOG_SOURCES += $(shell find /home/wp/Desktop/ysyx-workbench/ysyxSoC/ready-to-run/D-stage -name "*.v")

CPP_SOURCES = $(wildcard csrc/*.cpp)
CPP_SOURCES += csrc/disasm.cc

VERILATOR_INPUT = $(VERILOG_SOURCES) $(CPP_SOURCES)

IMG ?=
ARGS ?=
TARGET := $(subst ",,$(ARGS))
TARGET += --log=$(NPC_HOME)/npc-log.txt
TARGET += --diff=$(DIFF_LIBS)

BUILD_DIR = $(shell pwd)
BINARY = $(BUILD_DIR)/obj_dir/V$(MODULE)

NPC_EXEC := $(BINARY) $(TARGET) $(IMG)

.PHONY:run build waves
waves: 
	@gtkwave waveform.vcd

run: build
	@echo "###  Running NPC  ###"
	@echo "./obj_dir/V${MODULE} ${NPC_EXEC}"
	$(NPC_EXEC)

build:
	@echo "--------  Verilator --------"
	@rm -rf ./obj_dir
	@rm -rf waveform.vcd
	@echo "${VERILATOR} ${VERILATOR_FLAGS} ${VERILATOR_INPUT}"
	$(VERILATOR) $(VERILATOR_FLAGS) $(VERILATOR_INPUT)

gdb: build
	gdb --args $(NPC_EXEC) 

default: build

.PHONY:lint
lint: $(VERILOG_SOURCES)
	verilator --lint-only -Wall $(VERILOG_SOURCES)

.PHONY: clean
clean:
	rm -rf ./obj_dir
	rm -rf npc-log.txt
	rm -rf waveform.vcd


# TOPNAME = ysyxSoCFull
# IMG ?= 
# INC_PATH ?=
# NXDC_FILES = constr/top.nxdc

# ifeq ($(VERILATOR_ROOT),)
# VERILATOR = verilator
# VERILATOR_COVERAGE = verilator_coverage
# else
# export VERILATOR_ROOT
# VERILATOR = $(VERILATOR_ROOT)/bin/verilator
# VERILATOR_COVERAGE = $(VERILATOR_ROOT)/bin/verilator_coverage
# endif

# VERILATOR_FLAGS += -MMD
# VERILATOR_FLAGS += --build
# VERILATOR_FLAGS += -cc
# VERILATOR_FLAGS += --trace
# VERILATOR_FLAGS += --assert
# VERILATOR_FLAGS += -j 4
# VERILATOR_FLAGS += --timescale "1ns/1ns"
# VERILATOR_FLAGS += --no-timing
# VERILATOR_FLAGS += --autoflush

# BUILD_DIR = ./build
# BIN = $(BUILD_DIR)/$(TOPNAME)

# default: $(BIN)

# $(shell mkdir -p $(BUILD_DIR))

# SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
# $(SRC_AUTO_BIND): $(NXDC_FILES)
# 	python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# VSRCS = $(wildcard vsrc/*.v)
# VSRCS += $(shell find /home/wp/Desktop/ysyx-workbench/ysyxSoC/perip/ -name "*.v")
# VSRCS += $(shell find /home/wp/Desktop/ysyx-workbench/ysyxSoC/build/ -name "*.v")

# CSRCS = $(wildcard csrc/*.cpp)
# CSRCS += csrc/disasm.cc
# CSRCS += $(SRC_AUTO_BIND)

# INCLUDE_FLAGS += -I$(YSYXSOC_HOME)/perip/uart16550/rtl
# INCLUDE_FLAGS += -I$(YSYXSOC_HOME)/perip/spi/rtl

# include $(NVBOARD_HOME)/scripts/nvboard.mk

# INCFLAGS = $(addprefix -I, $(INC_PATH))
# INCFLAGS += -I$(NVBOARD_HOME)/include

# CXXFLAGS += -g       ###调试使用
# CXXFLAGS += $(shell llvm-config --cxxflags) -fPIE
# CXXFLAGS += $(INCFLAGS)
# CXXFLAGS += -fsanitize=leak
# # CXXFLAGS += -fsanitize=address

# DIFF_LIBS += $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
# LLVM_LIBS += $(shell llvm-config --libs)  
# LDFLAGS += $(LLVM_LIBS) -lreadline 

# IMG ?=
# ARGS ?=
# TARGET := $(subst ",,$(ARGS))
# TARGET += --log=$(NPC_HOME)/npc-log.txt
# TARGET += --diff=$(DIFF_LIBS)

# NPC_EXEC := $(BIN) $(TARGET) $(IMG)

# .PHONY:run waves lint clean

# clean:
# 	rm -rf ./build/*
# 	rm -rf npc-log.txt
# 	rm -rf waveform.vcd

# waves: 
# 	@gtkwave waveform.vcd

# $(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
# 	@echo "--------  Verilator --------"
# 	$(VERILATOR) $(VERILATOR_FLAGS) $(INCLUDE_FLAGS) \
# 		--top-module $(TOPNAME) $^ \
# 		$(addprefix -CFLAGS , $(CXXFLAGS)) -CFLAGS -U__STDC_FORMAT_MACROS \
# 		$(addprefix -LDFLAGS , $(LDFLAGS)) \
# 		--Mdir $(BUILD_DIR) --exe -o $(abspath $(BIN))

# run: clean $(BIN)
# 	@echo "###  Running NPC  ###"
# 	$(NPC_EXEC)

# gdb: build
# 	gdb --args $(NPC_EXEC) 

# lint: $(VSRCS)
# 	verilator --lint-only -Wall $(VSRCS)

