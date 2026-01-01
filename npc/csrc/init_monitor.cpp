#include "../include/common.h"
#include "../include/sim_main.h"
#include "../include/paddr.h"
#include "../include/utils.h"
#include "../include/isa.h"
#include "sdb.h"
#include "disasm.h"
#include <getopt.h>

char *elf_file = NULL;
char *log_file = NULL;
char *diff_so_file = NULL;
char *img_file = NULL;
static int difftest_port = 1234;

void sdb_set_batch_mode();
void init_log(const char *log_file);
void init_ftrace(const char *elf_file);
void init_difftest(char *ref_so_file, long img_size, int port);

void welcome() {
  Log("Trace-I: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  Log("Waveform: %s", MUXDEF(CONFIG_VCD, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  Log("NVboard: %s", MUXDEF(CONFIG_NVBOARD, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT("riscv32e", ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

long load_img(){
    if (img_file == NULL) {
      Log("No image is given. Use the default build-in image.");
      return 4096;
    } 
    FILE *fp = fopen(img_file, "rb");
    Assert(fp, "Can not open '%s'", img_file);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    Log("The image is %s, size = %ld", img_file, size);

    fseek(fp, 0, SEEK_SET);
    uint32_t ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
    assert(ret == 1);
    fclose(fp);
    return size;
}

uint32_t *parse_args(int argc, char *argv[]){
  int opt;
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {"ftrace"   , required_argument, NULL, 'f'},
    {0          , 0                , NULL,  0 },
  };
  while ( (opt = getopt_long(argc, argv, "-bhf:l:d:p:", table, NULL)) != -1) {
    switch (opt) {
      case 'b': sdb_set_batch_mode();break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'f': elf_file = optarg; break;
      case 1: img_file = optarg; break;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\t-f,--ftrace             output fuction info to terminal\n");
        printf("\n");
        exit(0);
      }
    }
    return 0;
}


void init_monitor(int argc, char *argv[]){
  parse_args(argc,argv);

  init_log(log_file);

  init_ftrace(elf_file);

  init_mem();

  //IFDEF(CONFIG_DEVICE, init_device());

  init_isa();

  long img_size = load_img();

  init_regex();

  init_wp_pool();

  init_difftest(diff_so_file, img_size, difftest_port);

  #ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
  #endif
  
  welcome();
}


