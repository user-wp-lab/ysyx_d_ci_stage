#include <regex.h>
#include "../include/common.h"
#include "../include/isa.h"
#include "sdb.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
enum
{
  /* TODO: Add more token types */
  TK_NOTYPE = 256,TK_EQ = 255,TK_NEQ = 254,TK_AND = 253,TK_OR = 252,
  HEX_NUM = 251,DEC_NUM = 250,REG = 249,TK_LESS = 248,TK_GREAT = 247,DEREF = 246,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},   // spaces
  {"\\+", '+'},        // add
  {"-", '-'},          // substract
  {"\\*", '*'},        // plus
  {"/", '/'},          // devision
  {"\\(", '('},        
  {"\\)", ')'},  
  {"0[xX][0-9a-fA-F]+", HEX_NUM}, 
  {"[0-9]+", DEC_NUM},
  {"==", TK_EQ},       // complete equal
  {"!=", TK_NEQ},      // not equal
  {">=", TK_GREAT},
  {"<=", TK_LESS},
  {"\\$(\\$0|[a-z0-9]+)", REG},
  {"&&", TK_AND},         // logic and
  {"\\|\\|", TK_OR},      // logic or
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};
void init_regex() {                                      //初始化正则表达式
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);       //regcomp函数将正则表达式字符串编译成为regex_t类型的可执行的正则表达式
    if (ret != 0) {              //re[i]保存编译好的正则表达式,rules[i].regex要编译的正则表达式字符串，REG_EXTENDED扩展型，成功返回0，错误返回i错误码
      regerror(ret, &re[i], error_msg, 128); 
      /* ret: 指定要解析的错误码，各个reg系列函数的返回值 re[i]: 指向编译好的正则表达式，指向 regex_t类型变量 error_msg: 一块内存空间，用来保存解析得到的错误信息 128: 数组空间的大小，避免保存错误信息时越界*/
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {      //记录类型与十进制数
  int type;
  char str[320];
} Token;          //Token结构体记录token信息

static Token tokens[320] __attribute__((used)) = {};   //按顺序存放识别出的token信息
static int nr_token __attribute__((used))  = 0;       //识别出的数目

static bool make_token(char *e) {         //处理表达式
  int position = 0;                       //识别到的位置
  int i;                                  //表达式的位置
  regmatch_t pmatch;                     //保存匹配到的信息

  nr_token = 0;                          //初始化识别出的数目


    /*regexec函数中，re[i]指向编译好的，要执行的正则表达式，e+ positione匹配的源字符串
      pmatch：用来保存匹配到的模式的信息，内部结构：
	typedef struct {
		regoff_t rm_so; //匹配到的字符串起始位置 start offset
		regoff_t rm_eo; //匹配到的字符串的结束位置 end offset
		} regmatch_t;*/
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
		           
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;
	
		if(strlen(tokens[nr_token].str) != 0){
			for(int k=0 ; k<320 ;k++){
				memset(tokens[k].str,'\0',sizeof(tokens[k].str));
			}
		}	 
	
        switch (rules[i].token_type) {		
            case '+':
            case '-':
            case '*':
            case '/':
            case '(':
            case ')':      //处理表达式符号
            case DEC_NUM:
            case HEX_NUM:	  //处理十进制数与十六进制数
            case TK_EQ:
            case TK_NEQ:
            case TK_AND:
            case TK_OR:    //处理扩展表达式
            case TK_LESS:
            case TK_GREAT:
            case REG:
              tokens[nr_token].type = rules[i].token_type;
              strncpy(tokens[nr_token].str, substr_start, substr_len);
              nr_token++;
              break;
          default:
            // 结束此次switch-case语句
	    break; 
      }
         break;
    }
  }

    if (i == NR_REGEX) {
    	  if(e[position] != '\n'){
      		printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      	  	return false;
      	  }else{
	  	return true;	    
      	  }
    }
  }

  return true;
}

bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')')
    return false;
  
  int cnt = 0;
  for (int i = p + 1; i < q; i++) {
    if (tokens[i].type == '(')
      cnt++;
    else if (tokens[i].type == ')') {
      if (cnt == 0){
        return false;
        }
      cnt--;
    }
  }
  if(cnt != 0 ){
    return false;
  } 
  else{
    return true;
  }
}

int find_the_primary_operator(int p, int q) {
  int p_count = 0;
  
  for (int i = q; i >= p; i--) {
    if (tokens[i].type == '(') {
      p_count++;
    } else if (tokens[i].type == ')') {
      p_count--;
    } else if (p_count == 0 && (tokens[i].type == '+' || tokens[i].type == '-')) {
      return i;
    }
  }
  
  for (int i = q; i >= p; i--) {
    if (tokens[i].type == '(') {
      p_count++;
    } else if (tokens[i].type == ')') {
      p_count--;
    } else if (p_count == 0 && (tokens[i].type == '*' || tokens[i].type == '/')) {
      return i;
    }
  }
  
  return -1;
}

word_t eval(int p, int q) {
  if (p > q) {
    /* Bad expression */
    printf("Bad expression\n");
    //assert(0);
    return -1;
  }
  else if (p == q) {
    word_t number = 0;
    if (tokens[p].type == DEC_NUM) {
      number = strtol(tokens[p].str, NULL, 10);
    } else if (tokens[p].type == HEX_NUM) {
      number = strtol(tokens[p].str, NULL, 16);
    }else if(tokens[p].type == REG){
      bool success;
      char *temp = strtok(tokens[p].str,"$");
      number = isa_reg_str2val(temp,&success);
      if(success){
	      printf("reg $%s: 0x%08x\n",temp,number);
      }else{ return -1;} 
    }else{
      printf("meaningless expersion\n");
    }
    return number;
     
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  else {
    int op = find_the_primary_operator(p, q);
    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);
    char op_type;
    op_type = tokens[op].type;
    switch (op_type) {
      case '+': return val1+val2;
      case '-': return val1-val2;
      case '*': return val1*val2;
      case '/': 
                if(val2!=0){
                    return val1/val2;
                }else{
                    printf("The dividend is zero!!!\n");
                    return -1;
                }	
      default: printf("Unknown expression\n");
               //assert(0);
              return -1;
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  	}
  /* Evaluate the expression recursively */
  int result_p = eval(0, nr_token-1);
  word_t result = eval(0, nr_token - 1);
  //含地址或者寄存器变量
  for(int i=0; i<nr_token; i++){
	if(tokens[i].type == REG || tokens[i].type == HEX_NUM){
	    if(result == 4294967295){
	            *success = false;
	            printf("success : false! \n");
	            return -1;
            }else{
            	*success = true;
	          printf("success : true! \n");
	          return result;
            }
	}
  }
  //不含地址或者寄存器变量
  if(result == 4294967295){
	  *success = false;
	  printf("success : false! \n");
	  return -1;
  }else if(result_p < 0){
  	*success = false;
  	printf("success : flase!(overflow)\nresult is less than zero\n");
	return result;
  }else{
  	*success = true;
	printf("success : true! \n");
	return result;
  }
}

