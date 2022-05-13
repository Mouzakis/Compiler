#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define EXPAND_SIZE 1024
#define CURR_SIZE (totalq*sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE*sizeof(quad)+CURR_SIZE)

extern unsigned programVarOffset;
extern unsigned functionLocalOffset;
extern unsigned formalArgOffset;
extern unsigned scopeSpaceCounter;

extern struct quad* quads;

// typedef struct Variable {
// 	const char *name;
// 	unsigned int scope;
// 	unsigned int line;
// } Variable;

// typedef struct Function {
// 	const char *name;
// 	unsigned int scope;
// 	unsigned int line;
// 	unsigned iaddress;
// 	unsigned totallocals;
// } Function;

/*struct SymbolTableEntry {
	int isActive;
	union {
		Variable *varVal;
		Function *funcVal;
	} value;
	char *type;
	struct SymbolTableEntry *next;
};*/


struct SymbolTableEntry *head;


// void Insert(int var, char *type, char *name, unsigned int scope, unsigned int line);
// struct SymbolTableEntry * Insert2(int var, char *type2, char *name, unsigned int scope, unsigned int line);
// void init(void);
// void Print(void);
// void initList(void);
// int LookUp(char *name, int scope, char *type);
// struct SymbolTableEntry* LookUp2(char *name, int scope, char *type);
// char *generate_unknown(void);

typedef enum symbol_t { var_s, programfunc_s, libraryfunc_s}symbol_t;

typedef enum scopespace_t {
	programvar,
	functionlocal,
	formalarg
}scopespace_t;


typedef struct Variable {
	const char *name;
	unsigned int scope;
	unsigned int line;
} Variable;

typedef struct Function {
	const char *name;
	unsigned int scope;
	unsigned int line;
	unsigned iaddress;
	unsigned totallocals;
} Function;

typedef struct SymbolTableEntry {
	symbol_t stype;
	scopespace_t space;
	unsigned int offset;
	int isActive;
	union {
		Variable *varVal;
		Function *funcVal;
	} value;
	char *type;
	struct SymbolTableEntry *next;
}Symtab;

void Hide(int scope);
void Insert(int var,char *type, char *name, unsigned int scope, unsigned int line);
struct SymbolTableEntry * Insert2(int var, char *type2, char *name, unsigned int scope, unsigned int line);
void init(void);
void Print(void);
void initList(void);
Symtab* LookUp(char *name, unsigned int scope, char *type);
char *generate_unknown(void);


typedef enum iopcode {
	assign, 	add,			sub,
	mul,		divi,			mod,
	uminus,		and,			or,
	not,		if_eq,			if_noteq,
	if_lesseq,	if_greatereq,	if_less,
	if_greater,	call,			param,
	ret,		getretvar,		funcstart,
	funcend,	tablecreate,    jump,
	tablegetelem,	tablesetelem
}iopcode;


// struct SymbolTableEntry {
// 	symbol_t type;
// 	char* name;
// 	scopespace_t space;
// 	unsigned int offset;
// 	int isActive;
// 	union {
// 		Variable *varVal;
// 		Function *funcVal;
// 	} value;
// 	char *type2;
// 	struct SymbolTableEntry *next;
// };

typedef enum expr_t{
	var_e,
	tableitem_e,
	programfunc_e,
	libraryfunc_e,
	arithexpr_e,
	boolexpr_e,
	assignexpr_e,
	newtable_e,
	constint_e,
	constdouble_e,
	constbool_e,
	conststring_e,
	nil_e
}expr_t;

typedef struct expr{
	enum expr_t type;
	struct SymbolTableEntry* sym;
	struct expr* index;
	double	numConst;
	char* strConst;
	unsigned char boolConst;
	struct expr* next;
}expr;

typedef struct quad{
	enum iopcode op;
	struct expr* result;
	struct expr* arg1;
	struct expr* arg2;
	unsigned label;
	unsigned line;
	unsigned address;
}quad;

struct indexed {
	struct expr* arg1;
	struct expr* arg2;
	struct indexed *next;
};

struct strcall {
	struct expr* elist;
	int method;
	char* name;
};

struct forpr{
	unsigned test;
	unsigned enter;
};

void expand(void); 

void emit(iopcode op,expr *arg1,expr* arg2,expr *result,unsigned label,unsigned line);

expr *newexpr(expr_t expression);
expr* newexpr_constbool(unsigned char b);
expr* newexpr_constint(double num);
expr* lvalue_expr(struct SymbolTableEntry *sym);

unsigned nextquad(void);

char* newtempname(void);
void resettemp(void);
struct SymbolTableEntry *newtemp(int scope, int line);

struct breaklist {
	int q;
	int active;
	struct breaklist *next;
};
struct contlist {
	int q;
	int active;
	struct contlist *next;
};
struct retlist {
	int q;
	int active;
	struct retlist *next;
};
struct statement {
	struct breaklist *breaklist;
	struct contlist *contlist;
	struct retlist *retlist;
};

void patchlabel(unsigned q, unsigned label);

void resetformalargsoffset(void);

void resetfunctionlocaloffset(void);

void enterscopespace(void);
void exitscopespace(void);

void checkuminus(expr* e);

void printquads();

typedef struct locals{
	unsigned functionLocalOffset;
	struct locals *next;
}locals_s;

struct locals *hlocal;

void addlocal(unsigned n);
unsigned readlocal(void);

expr* emit_iftableitem (expr* e, int scope, int line);
expr* newexpr_conststring (char* s);
expr* member_item(expr* lvalue, char* name, int scope, int line);
expr* make_call(expr* lvalue, expr* elist, int scope, int line);

void add_front(struct strcall* c, expr* el);
expr* reverselist(expr* head);
struct indexed* reverseindexed(struct indexed* head);