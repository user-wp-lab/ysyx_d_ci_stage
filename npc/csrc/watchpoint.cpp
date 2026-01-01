#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr_t[32];
  word_t new_val;
  word_t old_val;
  /* TODO: Add more members if necessary */
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].expr_t[0] = '\0';
    wp_pool[i].new_val = -1;
    wp_pool[i].old_val = -1;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

bool success = true;
WP* new_wp(){
    WP* current = free_;
    while (current != NULL) {
        if (current->expr_t[0] == '\0') {
            free_ = current->next; 
            current->next = NULL;
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void free_wp(WP *wp){
    WP* temp = head;
    WP* prev = NULL; 

    while(temp != NULL){
        if(wp->NO == temp->NO){
            // 头节点
            if (prev == NULL) {
                head = temp->next;
            } else {
                prev->next = temp->next;
            }

            wp->expr_t[0] = '\0';
            wp->new_val = -1;
            wp->old_val = -1;
            wp->next = free_;
            free_ = wp;
            printf("监视点已归还\n");
            return;
        } else {
            prev = temp;  
            temp = temp->next;
        }
    }
    assert(0);
}

void set_watchpoint(char *e){
	expr(e,&success);
	if(success == false){	
		printf("fail to set watchpoint\n");
		return;
	}
	WP *newnode = new_wp();
	if(newnode != NULL ){
                newnode->old_val = expr(e,&success);
                strcpy(newnode->expr_t,e);
                newnode->next = NULL;
                if(head == NULL){
                      head = newnode;
                }else{
                      WP *lastnode = head;
                      while(lastnode->next != NULL ){
                            lastnode = lastnode->next;
                      }
                      lastnode->next = newnode;
               }
        	printf("set watchpoint #%d\nexpr\t:%s\nold_val = 0x%08x\n",newnode->NO,newnode->expr_t,newnode->old_val);
		return;
	}else{
		printf("暂时无空闲监视点\n");
		return;
	}	
}
bool delete_watchpoint(int NO){
	WP *temp;
	temp = head;
	if(head == NULL){
		printf("未设置监视点\n");
		return false;
	}
	if(head->NO == NO){
		free_wp(temp);
		return true;
	}else{
		WP *pre = head;
		while(pre->next != NULL){
			if(pre->next->NO == NO){
				temp = pre->next;
				free_wp(temp);
				return true;
			}else{
				pre = pre->next;
			}
		}
		return false;
	}
}
void list_watchpoint(){
	WP *insert = head;
	if(insert == NULL){	
		printf("监视点为空\n");
		return;
	}
	printf("NO\texpr\t\t\t\told_val\n");
	while(insert != NULL){
		if(insert->expr_t[0] != '\0'){
			printf("%d\t%-30s0x%08x\n",insert->NO,insert->expr_t,insert->old_val);
		}
		insert = insert->next;
	}
	return;
}

int scan_watchpoint(){
	WP *day;
	day = head;
	while(day!=NULL){
            if(day->expr_t[0] != '\0'){
		day->new_val = expr(day->expr_t,&success);
		if(day->new_val != day->old_val){
			printf("Hit watchpoint #%d\nexpr : %s\nold_val = 0x%08x\nnew_val = 0x%08x\nprogram pause\n",day->NO,day->expr_t,day->old_val,day->new_val);
		return 1;
		}
		day = day->next;
	    }
	}
	return 0;
}