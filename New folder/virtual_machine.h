
#include "terminal.h"



#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_TABLE_HASHSIZE 211
#define AVM_STACKENV_SIZE 4

#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1

#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v

typedef enum avm_memcell_t{
	number_m = 0,
	string_m = 1,
	bool_m = 2,
	table_m = 3,
	userfunc_m = 4,
	libfunc_m = 5,
	nil_m = 6,
	undef_m = 7
}avm_memcell_t;


typedef struct avm_memcell{
	avm_memcell_t type;
	union{
		double numVal;
		char* strVal;
		unsigned char boolVal;
		struct avm_table* tableVal;
		unsigned funcVal;
		char* libfuncVal;
	}data;
} avm_memcell;


static void avm_initstack (void);

avm_memcell* avm_translate_operand (vmarg* arg, avm_memcell* reg);
void avm_memcellclear (avm_memcell* m);

avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned top, topsp;

avm_memcell* avm_translate_operand (vmarg* arg, avm_memcell* reg);

avm_memcell stack[AVM_STACKSIZE];





typedef struct avm_table_bucket{
	avm_memcell key;
	avm_memcell value;
	struct avm_table_bucket* next;
}avm_table_bucket;



typedef struct avm_table{
	unsigned refCounter;
	avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
	unsigned total;
}avm_table;


avm_table* avm_tablenew(void);
void avm_tabledestroy(avm_table* t);
avm_memcell* avm_tablegetelem(avm_memcell* key, avm_memcell* index);
void avm_tablesetelem(avm_memcell* key, avm_memcell* value, avm_memcell* value1);






void avm_tableincrefcounter (avm_table* t);
void avm_tabledecrefcounter (avm_table* t);
void avm_tablebucketsinit (avm_table_bucket** p);

avm_table* avm_tablenew (void);

void avm_tablebucketsdestroy (avm_table_bucket** p);

void avm_tabledestroy (avm_table* t);



typedef void (*execute_func_t)(instruction*);


extern void execute_assign (instruction* i);
 
extern void execute_add (instruction* i); 
extern void execute_sub (instruction* i);
extern void execute_mul (instruction* i); 
extern void execute_div (instruction* i); 
extern void execute_mod (instruction* i);

extern void execute_uminus (instruction* i); 
extern void execute_and (instruction* i);
extern void execute_or (instruction* i);
extern void execute_not (instruction* i);
 
extern void execute_jeq (instruction* i); 
extern void execute_jne (instruction* i); 
extern void execute_jle (instruction* i); 
extern void execute_jge (instruction* i); 
extern void execute_jlt (instruction* i);
extern void execute_jgt (instruction* i);
 
extern void execute_call (instruction* i);
extern void execute_pusharg (instruction* i); 
extern void execute_funcenter (instruction* i); 
extern void execute_funcexit (instruction* i);  

extern void execute_newtable (instruction* i); 
extern void execute_tablegetelem (instruction* i); 
extern void execute_tablesetelem (instruction* i);
extern void execute_nop (instruction* i); 



typedef double (*arithmetic_func_t)(double x, double y);


typedef char* (*tostring_func_t)(avm_memcell*);

extern char* number_tostring (avm_memcell*);
extern char* string_tostring(avm_memcell*);
extern char* bool_tostring(avm_memcell*);
extern char* table_tostring(avm_memcell*);
extern char* userfunc_tostring(avm_memcell*);
extern char* libfunc_tostring(avm_memcell*);
extern char* nil_tostring(avm_memcell*);
extern char* undef_tostring(avm_memcell*);




typedef void (*memclear_func_t)(avm_memcell*);

extern void memclear_string (avm_memcell* m);
extern void memclear_table(avm_memcell* m);

memclear_func_t memclearFuncs[] ={
	0, /* number */
	memclear_string,
	0, /* bool */
	memclear_table,
	0, /* userfunc */
	0, /* livfunc */
	0, /* nil */
	0  /* undef */
};

extern void avm_warning (char* format, char* sec, char* thi);
void avm_error(char* format,  char* s, char*s1);

void avm_callsaveenvironment(void);

double consts_getnumber(unsigned n);

char* consts_getstring(unsigned n);

char* libfuncs_getused(unsigned n);



typedef void (*library_func_t) (void);
void avm_registerlibfunc (char* id, library_func_t addr);
library_func_t avm_getlibraryfunc (char* id); 


void avm_assign (avm_memcell* lv, avm_memcell* rv);
void avm_memcellclear(avm_memcell *v);	


unsigned char avm_tobool(avm_memcell* rv);