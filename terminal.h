#include "quads.h"

#define EXPAND_SIZE_INSTR 1024
#define CURR_SIZE_INSTR (totalInstr*sizeof(instruction*))
#define NEW_SIZE_INSTR (EXPAND_SIZE*sizeof(quad)+CURR_SIZE_INSTR)

char* find_var(unsigned pos);
void new_var(char* name, unsigned pos);
char* find_localvar(unsigned pos);
void new_localvar(char* name, unsigned pos);
char* find_formalvar(unsigned pos);
void new_formalvar(char* name, unsigned pos);
unsigned consts_newstring (char* s);
unsigned consts_newnumber (double n);
unsigned libfuncs_newused (char* s);
unsigned userfuncs_newfunc (struct SymbolTableEntry* sym);

extern void generateAll(void);

extern void generate_add (quad* q);
extern void generate_sub (quad*);
extern void generate_mul (quad*);
extern void generate_divi(quad*);
extern void generate_mod (quad*);
extern void generate_newtable (quad*);
extern void generate_tablegetelem (quad*);
extern void generate_tablesetelem (quad*);
extern void generate_assign (quad*);
extern void generate_nop (quad*);
// extern void generate_jump (quad*);
// extern void generate_if_eq (quad*);
// extern void generate_if_noteq (quad*);
// extern void generate_if_greater (quad*);
// extern void generate_if_greatereq (quad*);
// extern void generate_if_less (quad*);
// extern void generate_if_lesseq (quad*);
extern void generate_not(quad*);
extern void generate_and (quad*);
extern void generate_or (quad*);
extern void generate_param (quad*);
extern void generate_call (quad*);
extern void generate_getretval (quad*);
extern void generate_puncstart (quad*);
extern void generate_return (quad*);
extern void generate_funcstart (quad*);
extern void generate_funcend (quad*);

typedef enum vmopcode {
	assign_v,	add_v,	sub_v,
	mul_v,		div_v,	mod_v,
	uminus_v,	and_v,	or_v,
	not_v,		jeq_v,	jne_v,
	jle_v,		jge_v,	jlt_v,
	jgt_v,		call_v,	pusharg_v,
	funcenter_v, jump_v,
	funcexit_v,
	newtable_v,		tablegetelem_v,
	tablesetelem_v,	nop_v,
}vmopcode;
extern void generate_relational(vmopcode op,quad* q);

typedef enum vmarg_t {
	label_a = 0,
	global_a = 1,
	formal_a = 2,
	local_a = 3,
	number_a = 4,
	string_a = 5,
	bool_a = 6,
	nil_a = 7,
	userfunc_a = 8,
	libfunc_a = 9,
	retval_a = 10
}vmarg_t;

typedef struct vmarg {
	enum vmarg_t type;
	unsigned val;
}vmarg;

typedef struct instruction {
	vmopcode opcode;
	vmarg result;
	vmarg arg1;
	vmarg arg2;
	unsigned srcLine;
}instruction;

struct userfunc {
	unsigned address;
	unsigned localSize;
	char* id;
};

unsigned nextinstructionlabel(void);
void make_operand (expr* e, vmarg* arg);
void vmemit(instruction* t);

char* vmopcodeToStr(vmopcode op);

double* numConsts;
unsigned totalNumConsts;
char** stringConsts;
unsigned totalStringConsts;
char** namedLibFuncs;
unsigned totalNamedLibFuncs;
struct userfunc* userFuncs;
unsigned totalUserFuncs;

void add_incomplete_jump(unsigned instrNo, unsigned iaddress);
void patch_incomplete_jumps(void);
unsigned currprocessedquad(void);

typedef struct incomplete_jump {
	unsigned instrNo;
	unsigned iaddress;
	struct incomplete_jump* next;
}incomplete_jump;

typedef struct fstack{
	const char *name;
	unsigned iaddress;
	unsigned totallocals;
	struct funcreturn *funcreturns;
	struct fstack *next;
}fstack;

fstack *fstackpop();

struct fstack *fshead =NULL;

typedef struct funcreturn{
	unsigned target_label;
	struct funcreturn *next;
}funcreturn;

void reset_operand(vmarg*);
fstack *fstacktop(void);
void append(funcreturn *r, unsigned n);
fstack *func_add(const char *id, unsigned taddress, unsigned totallocals);
void pushfunc(fstack *s);

char* find_symbol(unsigned offset, int arg);

typedef struct vartable{
	unsigned pos;
	char* name;
	struct vartable* next;
}vartable;

typedef struct localvartable{
	unsigned pos;
	char* name;
	struct localvartable* next;
}localvartable;

typedef struct formalvartable{
	unsigned pos;
	char* name;
	struct formalvartable* next;
}formalvartable;

unsigned userfuncs_newfunc(struct SymbolTableEntry *sym);
void backpatcth_ret(funcreturn *f, unsigned n);

unsigned new_LibFunc(char *s);
