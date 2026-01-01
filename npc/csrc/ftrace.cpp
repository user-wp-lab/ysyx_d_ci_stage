#include "../include/common.h"
#include "../include/macro.h"
#include "../include/globals.h"
#include <stdio.h>
#include <elf.h>
typedef struct FunctionSymbol{
	char *name;
	uint32_t addr;
}funsym;	
funsym *func_table = NULL;
int func_num = 0;
void init_ftrace(const char *elf_file)
{
    #ifdef CONFIG_FTRACE
	FILE *ft = fopen(elf_file,"rb");
    Assert(elf_file, "ELF file is empty");
    Assert(ft, "Can not open '%s'", elf_file);


	Elf32_Ehdr ehdr;
	int ret = fread(&ehdr, 1, sizeof(ehdr), ft);
	
	int i;
	Elf32_Shdr *shdr = (Elf32_Shdr *)malloc(ehdr.e_shentsize*ehdr.e_shnum);
	fseek(ft, ehdr.e_shoff, SEEK_SET);
	ret = fread(shdr, ehdr.e_shentsize, ehdr.e_shnum, ft);
	

	char *strstb = NULL;
	size_t strstb_size = 0;
	for(i = 0;i < ehdr.e_shnum; i++)
	{
		if(shdr[i].sh_type == SHT_STRTAB && i != ehdr.e_shstrndx)
		{
			strstb_size = shdr[i].sh_size;
			strstb = (char *)malloc(strstb_size);
			fseek(ft, shdr[i].sh_offset, SEEK_SET);
			ret = fread(strstb, 1, strstb_size,ft);
			break;
		}	
	}	

	for(i = 0;i < ehdr.e_shnum; i++)
	{
		if(shdr[i].sh_type == SHT_SYMTAB)
		{
			Elf32_Sym *sym = (Elf32_Sym *)malloc(shdr[i].sh_size);
			fseek(ft, shdr[i].sh_offset, SEEK_SET);
			ret = fread(sym, shdr[i].sh_size, 1, ft);
			if(ret){}
			int sym_cnt = shdr[i].sh_size/(sizeof(Elf32_Sym));
			for(int k = 0;k < sym_cnt; k++)
			{
				if(ELF32_ST_TYPE(sym[k].st_info) == STT_FUNC)
				{
					func_num++;
				}
			}
			func_table = (funsym *)malloc(func_num*sizeof(funsym));
			func_num = 0;
			for(int k = 0;k < sym_cnt; k++)
			{
				if(ELF32_ST_TYPE(sym[k].st_info) == STT_FUNC)
				{
					func_table[func_num].name = &(strstb[sym[k].st_name]);
					func_table[func_num].addr = sym[k].st_value;
					func_num++;
				}
			}	
		        free(sym);
		}
	}
	//free(strstb);
	free(shdr);
	fclose(ft);
    #endif
}

char *temp[120] = {nullptr};
static int wr_pos = 0;
static int put_space = 0;
void ftrace_debug(Decode *s, vaddr_t dnpc){
 	int cnt = 0;
	if((s->isa.inst.val & 0xffff707f) == 0x00000067){
            printf("0x%08x:",s->pc);
            put_space--;
            for(int i = 0;i < put_space;i++){
	        printf(" ");
            }
            printf("ret [%s]\n",temp[0]);
	    for(int i = 0;i < wr_pos;i++){
	    	temp[i] = temp[i+1];
	    }
	    wr_pos--;
            return;
        }
	for(cnt = 0;cnt < func_num;cnt++){
        if(func_table[cnt].addr == dnpc){
			if (wr_pos > 0) {
                for (int k = wr_pos; k > 0; k--) {
                    temp[k] = temp[k - 1];
                }
            }
            temp[0] = func_table[cnt].name;
            wr_pos++;
            put_space++;
            if (wr_pos == 120) wr_pos = 0;
            if (wr_pos == 0) temp[wr_pos] = func_table[cnt].name;
			
			printf("0x%08x:",s->pc);
			for(int i = 0;i < put_space;i++){
				printf(" ");
			}
			printf("call [%s@0x%08x]\n",func_table[cnt].name,dnpc);
			break;
		}
	}
}