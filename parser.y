%{
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "quads.h"



int yyerror(char* yaccProvidedMessage);
int yylex(void);

extern int yylineno;
extern char* yytext;
extern FILE* yyin;

char *curtype;
int infunc;
int func_initial_scope[20];
int linec = 0;
int elflag = 0;
int libflag = 0;
int lvalfunc = 0;
unsigned int curr_scope = 0;
unsigned int scope_function = 0;
unsigned int whileFlag = 0;
unsigned loopcounter = 0;
struct statement* bclist;
Symtab *res;
int rescope = 0;

%}

%start program

%union {
	char* stringValue;
	int intValue;
	double realValue;
	struct SymbolTableEntry *entry;
	struct expr *exprVal;
	struct forpr *forpref;
	struct strcall *scall;
	struct indexed* indexedEl;
}

%token <stringValue> ID
%token <intValue> INTCONST
%token <realValue> REALCONST
%token <stringValue> STRING
%token IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE AND NOT OR LOCAL TRUE FALSE NIL
%token DOUBLE_COLON DOUBLE_DOT EQUAL NOT_EQUAL LESS_EQUAL GREATER_EQUAL INCREMENT DECREMENT LINE_COM
%type <intValue> ifprefix elseprefix whilestart whilecond M N
%type <forpref> forprefix
%type <entry> funcprefix funcdef
%type <stringValue> funcname
%type <exprVal> lvalue const expr primary term assignexpr member objectdef elist call
%type <indexedEl> indexed indexedelem
%type <scall> methodcall normcall callsuffix


%right '='
%left OR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc '>' GREATER_EQUAL '<' LESS_EQUAL
%left '+' '-'
%left  '*' '/' '%'
%left '.' DOUBLE_DOT
%right NOT INCREMENT DECREMENT UMINUS
%left '[' ']'
%left '(' ')'

%%

program :		program stmt
				|
				;
stmt:			expr ';'
				|ifstmt {printf("IF statement\n");}
				|whilestmt {printf("WHILE statement\n");}
				|forstmt {printf("FOR statement\n");}
				|returnstmt {printf("RETURN statement\n"); if(infunc==0){printf("Error at line %d: Use of return while not in a function!\n",yylineno);
						}
						emit(jump, NULL, NULL, NULL, -1, yylineno);
						
						if (bclist->retlist->q == -1) {
							struct retlist* new = (struct retlist*)malloc(sizeof(struct retlist));
							new -> q = -1;
							new->active = 0;
							new->next = NULL;
							bclist->retlist->q = nextquad();
							bclist->retlist->active = 1;
							bclist->retlist->next = new;
						}
						else {
							struct retlist* new = (struct retlist*)malloc(sizeof(struct retlist));
							new -> q = nextquad();
							new->active = 1;

							struct retlist* trailer = bclist->retlist;
							while(trailer->next->q != -1) {
								trailer = trailer -> next;
							}
							new->next = trailer->next;
							trailer->next = new;
						}
						printf("\n\n\n%d\n\n\n", bclist->retlist->next->q);
				}
				|BREAK';' {printf("BREAK\n");if(elflag==0){printf("Error at line %d: Use of Break while not in a loop!\n",yylineno);}
					emit(jump, NULL, NULL, NULL, -1, yylineno);
					if (bclist->breaklist->q == -1) {
						struct breaklist* new = (struct breaklist*)malloc(sizeof(struct breaklist));
						new -> q = -1;
						new->active = 0;
						new->next = NULL;
						bclist->breaklist->q = nextquad();
						bclist->breaklist->active = 1;
						bclist->breaklist->next = new;
					}
					else {
						struct breaklist* new = (struct breaklist*)malloc(sizeof(struct breaklist));
						new -> q = nextquad();
						new->active = 1;

						struct breaklist* trailer = bclist->breaklist;
						while(trailer->next->q != -1) {
							trailer = trailer -> next;
						}
						new->next = trailer->next;
						trailer->next = new;
					}
					}
				|CONTINUE';' {printf("CONTINUE\n");if(elflag==0){printf("Error at line %d: Use of Continue while not in a loop!\n", yylineno);}
								emit(jump, NULL, NULL, NULL, -1, yylineno);
								if (bclist->contlist->q == -1) {
									struct contlist* new = (struct contlist*)malloc(sizeof(struct contlist));
									new -> q = -1;
									new->active = 0;
									new->next = NULL;
									bclist->contlist->q = nextquad();
									bclist->contlist->active = 1;
									bclist->contlist->next = new;
								}
								else {
									struct contlist* new = (struct contlist*)malloc(sizeof(struct contlist));
									new -> q = nextquad();
									new->active = 1;

									struct contlist* trailer = bclist->contlist;
									while(trailer->next->q != -1) {
										trailer = trailer -> next;
									}
									new->next = trailer->next;
									trailer->next = new;
								}
				}
				|block {printf("BLOCK statement\n");}
				|funcdef {printf("FUNCTION statement\n");}
				|';'
				|LINE_COM {linec += 1;}
				;

expr:			assignexpr	{printf("Assign Expression\n");}
				|expr '+' expr 	{printf("Expression + Expression\n");
					$$ = (expr*)newexpr(arithexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(add, $1, $3, $$,-1,yylineno);}
				|expr '-' expr 	{printf("Expression - Expression\n");
					$$ = (expr*)newexpr(arithexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(sub, $1, $3, $$,-1,yylineno);}
				|expr '*' expr 	{printf("Expression * Expression\n");
					$$ = (expr*)newexpr(arithexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(mul, $1, $3, $$,-1,yylineno);}
				|expr '/' expr 	{printf("Expression / Expression\n");
					$$ = (expr*)newexpr(arithexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(divi, $1, $3, $$,-1,yylineno);}
				|expr '%' expr 	{printf("Expression %c Expression\n", '%');
					$$ = (expr*)newexpr(arithexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(mod, $1, $3, $$,-1,yylineno);}
				|expr '>' expr 	{printf("Expression > Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_greater, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr GREATER_EQUAL expr 	{printf("Expression >= Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_greatereq, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr '<' expr 	{printf("Expression < Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_less, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr LESS_EQUAL expr 	{printf("Expression <= Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_lesseq, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr EQUAL expr 	{printf("Expression == Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_eq, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr NOT_EQUAL expr 	{printf("Expression != Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_noteq, $1, $3, NULL, nextquad()+3, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+4, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|expr AND expr 	{printf("Expression AND Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(and, $1, $3, $$,-1,yylineno);}
				|expr OR expr 	{printf("Expression OR Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(or, $1, $3, $$,-1,yylineno);}
				|term			{printf("Term\n");$$=$1;}
				;



term:			'(' expr ')' {printf("(Expression)\n");$$=$2;}
				|'-' expr %prec UMINUS	{printf("- Expression\n");
					checkuminus($2);
					 $$ = newexpr(arithexpr_e);
					 $$->sym = newtemp(curr_scope, yylineno);
					 emit(uminus, $2, NULL, $$, -1, yylineno);}
				|NOT expr 	{printf("NOT Expression\n");
					$$ = (expr*)newexpr(boolexpr_e);
					$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
					emit(if_eq, $2, newexpr_constbool(1), NULL, nextquad()+5, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+2, yylineno);
					emit(assign, newexpr_constbool(1), NULL, $$, -1, yylineno);
					emit(jump, NULL, NULL, NULL, nextquad()+3, yylineno);
					emit(assign, newexpr_constbool(0), NULL, $$, -1, yylineno);}
				|INCREMENT lvalue 	{printf("++Lvalue\n");
														if ($2->type == tableitem_e) {
															$$ = (expr*)emit_iftableitem($2, curr_scope, yylineno);
															emit(add, $$, newexpr_constint(1), $$, -1, yylineno);
															emit(tablesetelem, $2->index, $$, $2, -1, yylineno);
														}
														else {
															emit(add, $2, newexpr_constint(1), $2, -1, yylineno);
															$$ = newexpr(arithexpr_e);
															$$->sym = newtemp(curr_scope, yylineno);
															emit(assign, $2, NULL, $$, -1, yylineno);
														}
														if($2 != NULL){
															if(strcmp($2->sym->type,"userfunc")==0 || strcmp($2->sym->type,"libfunc")==0){
															if (strcmp($2->sym->type,"libfunc")==0){
																printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $2->sym->value.funcVal->name);}
															else{
																printf("Error at line %d: Cannot use function %s as an lvalue!\n", yylineno, $2->sym->value.funcVal->name);}}}
									}
				|lvalue INCREMENT 	{printf("Lvalue++\n");
														$$ = newexpr(var_e);
														$$->sym = newtemp(curr_scope, yylineno);
														if ($1->type == tableitem_e) {
															expr* value = (expr*)emit_iftableitem($1, curr_scope, yylineno);
															emit(assign, value, NULL, $$, -1, yylineno);
															emit(add, value, newexpr_constint(1), value, -1, yylineno);
															emit(tablesetelem, $1->index, value, $1, -1, yylineno);
														}
														else {
															emit(assign, $1, NULL, $$, -1, yylineno);
															emit(add, $1, newexpr_constint(1), $1, -1, yylineno);
														}
														if($1 != NULL){
															if(strcmp($1->sym->type,"userfunc")==0 || strcmp($1->sym->type,"libfunc")==0){
															if (strcmp($1->sym->type,"libfunc")==0){
																printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $1->sym->value.funcVal->name);}
															else{
																printf("Error at line %d: Cannot use function %s as an lvalue!\n", yylineno, $1->sym->value.funcVal->name);}}}
									}
				|DECREMENT lvalue 	{printf("--Lvalue\n");
														if ($2->type == tableitem_e) {
															$$ = (expr*)emit_iftableitem($2, curr_scope, yylineno);
															emit(sub, $$, newexpr_constint(1), $$, -1, yylineno);
															emit(tablesetelem, $2->index, $$, $2, -1, yylineno);
														}
														else {
															emit(add, $2, newexpr_constint(1), $2, -1, yylineno);
															$$ = newexpr(arithexpr_e);
															$$->sym = newtemp(curr_scope, yylineno);
															emit(assign, $2, NULL, $$, -1, yylineno);
														}
														if($2 != NULL){
															if(strcmp($2->sym->type,"userfunc")==0 || strcmp($2->sym->type,"libfunc")==0){
															if (strcmp($2->sym->type,"libfunc")==0){
																printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $2->sym->value.funcVal->name);}
															else{
																printf("Error at line %d: Cannot use function %s as an lvalue!\n", yylineno, $2->sym->value.funcVal->name);}}}
									}
				|lvalue DECREMENT 	{printf("Lvalue--\n");
														$$ = newexpr(var_e);
														$$->sym = newtemp(curr_scope, yylineno);
														if ($1->type == tableitem_e) {
															expr* value = (expr*)emit_iftableitem($1, curr_scope, yylineno);
															emit(assign, value, NULL, $$, -1, yylineno);
															emit(sub, value, newexpr_constint(1), value, -1, yylineno);
															emit(tablesetelem, $1->index, value, $1, -1, yylineno);
														}
														else {
															emit(assign, $1, NULL, $$, -1, yylineno);
															emit(sub, $1, newexpr_constint(1), $1, -1, yylineno);
														}
														if($1 != NULL){
															if(strcmp($1->sym->type,"userfunc")==0 || strcmp($1->sym->type,"libfunc")==0){
															if (strcmp($1->sym->type,"libfunc")==0){
																printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $1->sym->value.funcVal->name);}
															else{
																printf("Error at line %d: Cannot use function %s as an lvalue!\n", yylineno, $1->sym->value.funcVal->name);}}}
									}
				|primary {printf("Primary\n");$$=$1;}
				;

assignexpr:		lvalue '=' expr {printf("Lvalue=Expression\n");
					if($1->sym != NULL){
					if($1->sym -> type != NULL) {
						if((strcmp($1 ->sym-> type,"userfunc")==0) || (strcmp($1 ->sym-> type,"libfunc")==0)) {
								if (strcmp($1-> sym -> type,"libfunc")==0){
									printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $1 -> sym ->value.funcVal -> name);}
								else{
									printf("Error at line %d: Cannot use function %s as an lvalue!\n", yylineno, $1 -> sym -> value.funcVal -> name);}
						}
					}}
					if ($1->type == tableitem_e){
						emit(tablesetelem, $1->index, $3, $1, -1, yylineno);
						$$=emit_iftableitem($1, curr_scope, yylineno); 
						$$->type = assignexpr_e;
					}
					else{ 
						emit(assign,$3,NULL,$1, -1, yylineno); 
						$$=(expr*)newexpr(assignexpr_e); 
						$$->sym = (struct SymbolTableEntry*)newtemp(curr_scope, yylineno);
						emit(assign, $1, NULL, $$, -1, yylineno);
					}
					}
				;

primary:		lvalue {printf("Lvalue\n");$$ = emit_iftableitem($1, curr_scope, yylineno);}
				|call {printf("Call\n");libflag = 0;lvalfunc=0;}
				|objectdef {printf("Object definition\n");}
				|'(' funcdef ')' {printf("(Function statement)\n");$$ = newexpr(programfunc_e);
					 $$->sym = $2; }
				|const {printf("Const\n");$$=$1;}
				;

lvalue:			ID {printf("ID\n");$$=(expr*)newexpr(var_e);
					res = LookUp($1, curr_scope, curtype);
					if (res==NULL){
						Insert(1, curtype, $1, curr_scope, yylineno);
						$$ -> sym = LookUp($1, curr_scope, curtype);}
					else {$$ ->sym = res;
						if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
						else {rescope=res -> value.varVal->scope;}
						if ((infunc>0) && (rescope > 0) && (func_initial_scope[infunc-1]>=rescope)) {printf("Error at Line-> %d: Cannot access %s at scope %d\n", yylineno, $1, rescope);
						}
						if (strcmp(res->type,"userfunc") == 0) {lvalfunc=1;/*printf("Error at line %d: Cannot use function %s as an lvalue\n", yylineno, $1);*/}
						if (strcmp(res->type,"libfunc") == 0) {libflag = 1;/*printf("Error at line %d: Cannot use LibFunc %s as an lvalue!\n", yylineno, $1);*/}
					}
				}
				|LOCAL ID {printf("LOCAL ID\n");$$=(expr*)newexpr(var_e);
					res = LookUp($2, curr_scope, curtype);
					if (res==NULL){
						if(curr_scope>0){Insert(1, "local", $2, curr_scope, yylineno);}
						else{Insert(1, "global", $2, 0, yylineno);}
					}
					else {
						if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
						else {rescope=res -> value.varVal->scope;}
						if (rescope!=curr_scope && (strcmp(res->type,"libfunc")!= 0)) {
							if(curr_scope>0){Insert(1, "local", $2, curr_scope, yylineno);}
							else{Insert(1, "global", $2, 0, yylineno);}
						}
						else if((strcmp(res->type,"libfunc")== 0) && (curr_scope != 0)) {
							printf("Error at line %d: Trying to shadow libfunc at scope: %d - token:%s\n", yylineno, curr_scope, $2);
						}
					}
					$$ -> sym = LookUp($2, curr_scope, curtype);
					}
				|DOUBLE_COLON ID {printf("DOUBLE_COLON ID\n");
					res = LookUp($2, curr_scope, "global");
					$$=(expr*)newexpr(var_e);
					$$ -> sym = res;
					if (res!=NULL){
						if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
						else {rescope=res -> value.varVal->scope;}
						if(rescope!= 0) {printf("Error at line %d: Did not find %s in global scope\n", yylineno, $2);}
					}
					else {printf("Error at line %d: Did not find %s in global scope\n", yylineno, $2);}
					}
				|member  {printf("MEMBER\n");}
				;

member:			lvalue '.' ID {printf("LVALUE.ID\n");$$ = member_item($1, $3, curr_scope, yylineno);}
				|lvalue '[' expr ']' {printf("LVALUE[Expression]\n");
					$1 = emit_iftableitem($1, curr_scope, yylineno);
					$$ = newexpr (tableitem_e);
					$$ -> sym = $1 -> sym;
					$$ -> index = $3;
					}

				|call '.' ID  {printf("CALL.ID\n");}
				|call '[' expr ']' {printf("CALL[expression]\n");}
				;

call:			call '(' elist ')' {printf("CALL(elist)\n");libflag=0;lvalfunc=0;
									$$= make_call($$, $3, curr_scope, yylineno);}
				|lvalue callsuffix {printf("LVALUE Callsuffix\n");libflag=0;lvalfunc=0;
					if($2->method==1) {
						expr* self = $1;
						$1 = emit_iftableitem( member_item(self, $2->name, curr_scope, yylineno),curr_scope, yylineno);
						add_front($2,self);
					}
					$$ = make_call($1, $2->elist, curr_scope, yylineno);
				}
				|'(' funcdef ')' '(' elist ')' {printf("(Function expression)(elist)\n");
					expr* func = newexpr(programfunc_e);
					func-> sym = $2;
					$$ = make_call(func, $5, curr_scope, yylineno);
				}
				;

callsuffix:		normcall {printf("Normcall\n");$$=$1;}
				|methodcall {printf("Methodcall\n");$$=$1;}
				;

normcall:		'(' elist ')' {printf("(Elist)\n");
					$$ = (struct strcall*)malloc(sizeof(struct strcall));
					$$-> method = 0;
					$$-> elist = $2;
				};

methodcall:		DOUBLE_DOT ID '(' elist ')' {printf("DOUBLE_DOT ID (Elist)\n");
					$$ = (struct strcall*)malloc(sizeof(struct strcall));
					$$-> method = 1;
					$$-> elist = $4;
					$$-> name = $2;}
				;

elist:			expr {printf("Expression\n");
					if ($$==NULL){
						if($1 != NULL) {
							$$=(expr*)newexpr(var_e);
							$$->type = $1->type;
							$$->sym = $1->sym;
						}
						else {
							$$ = NULL;
						}
					}
				}
				|elist ',' expr {printf("Elist,Expression\n");$3->next= $$; $$=$3;}
				|{$$=NULL;}
				;

objectdef:		'[' elist ']' {printf("[Elist]\n");
					expr *t = newexpr(newtable_e);
					t -> sym = newtemp(curr_scope, yylineno);
					emit(tablecreate, NULL, NULL, t, -1, yylineno);
					int i;
					struct expr* h = reverselist($2);
					for (i = 0; h; h = h -> next) {
						emit(tablesetelem, newexpr_constint(i++), h, t, -1, yylineno);
					}
					$$ = t;
				}
				|'[' indexed ']' {printf("[Indexed]\n");
					expr *t = newexpr(newtable_e);
					t -> sym = newtemp(curr_scope, yylineno);
					emit(tablecreate, NULL, NULL, t, -1, yylineno);
					int i;
					struct indexed* h = reverseindexed($2);
					for (i = 0; h; h = h -> next) {
						emit(tablesetelem, h->arg1, h->arg2, t, -1, yylineno);
					}
					$$ = t;
				}
				;

indexed:		indexedelem {printf("Indexed Element\n");
					if ($$==NULL){
						if($1 != NULL) {
							$$=$1;
						}
						else {
							$$ = NULL;
						}
					}	
				}
				|indexed ',' indexedelem {printf("Indexed, Indexed Element\n");$3->next = $$; $$ = $3;}
				;

indexedelem:	'{' expr ':' expr '}' {printf("{Expression : Expression}\n");
					struct indexed *indexed = (struct indexed*)malloc(sizeof(struct indexed));
					indexed->arg1 = $2;
					indexed->arg2 = $4;
					indexed->next = NULL;
					$$ = indexed;
				}
				;

block:			'{' {curtype = "local";curr_scope++;} statements stmt '}'
				{if (curr_scope == 1){curtype = "global";}printf("BLOCK\n");Hide(curr_scope);curr_scope--;}
				|'{''}' {printf("BLOCK\n");Hide(curr_scope+1);}
				;
statements:		statements stmt
				|
				;

funcname:       ID {
					$$ = $1;
				}
				| 
				{
					$$ = generate_unknown();
					printf("Anonymous Function\n");
				}
				;

funcprefix:		FUNCTION funcname {
					printf("FUNCDEF\n");infunc++;func_initial_scope[infunc] = curr_scope+1;
						res = LookUp($2, curr_scope, curtype);
						if (res==NULL){
							$$ = (struct SymbolTableEntry *)Insert2(0, "userfunc", $2, curr_scope, yylineno); 
								$$ -> value.funcVal -> iaddress = nextquad()+2;
								emit(jump, NULL, NULL, NULL, -1, yylineno);
								emit(funcstart, lvalue_expr($$), NULL, NULL, -1 ,yylineno);
								addlocal(functionLocalOffset);
								enterscopespace(); 
								resetformalargsoffset();}
						else {
							if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
							else {rescope=res -> value.varVal->scope;}
							if (strcmp(res->type,"libfunc")==0) {
								printf("Error at line %d: User function conflicts with libfunc %s\n", yylineno, $2);
							}
							else if(rescope== curr_scope) {
								printf("Error at line %d: User function uses already declared name %s\n", yylineno, $2);
							}
							else {
								$$ = (struct SymbolTableEntry *)Insert2(0, "userfunc", $2, curr_scope, yylineno); 
								$$ -> value.funcVal -> iaddress = nextquad();
								emit(jump, NULL, NULL, NULL, -1, yylineno);
								emit(funcstart, lvalue_expr($$), NULL, NULL, -1 ,yylineno);
								addlocal(functionLocalOffset);
								enterscopespace(); 
								resetformalargsoffset();}
							}
						}
					;

funcargs:		'(' idlist ')' {
								enterscopespace(); 
								resetfunctionlocaloffset();
				};

funcbody:		block{
						exitscopespace();
				};

funcdef:		funcprefix 	funcargs funcbody {infunc--;func_initial_scope[infunc] = 0;
												exitscopespace();		
											$1-> value.funcVal -> totallocals = functionLocalOffset;
											functionLocalOffset = readlocal();
											$$ = $1;
											emit(funcend, lvalue_expr($1), NULL, NULL, -1 , yylineno);
											patchlabel($$->value.funcVal ->iaddress-2, nextquad()+1);
											struct retlist* trailer = bclist->retlist;
											/*printf("\n\n\n%dWTF\n\n\n",trailer->next->q);*/
											while (trailer->q!=-1){
												if (trailer->q > $1->value.funcVal->iaddress && trailer->active == 1) { patchlabel(trailer->q-1,nextquad()+1);trailer->active=0;}
												trailer=trailer->next;
											}
											}
				;

const:			INTCONST 	{printf("INTCONST\n");$$ = (expr*)newexpr(constint_e);$$->numConst = $1;}
				|REALCONST 	{printf("REALCONST\n");$$ = (expr*)newexpr(constdouble_e);$$->numConst = $1;}
				|STRING 	{printf("STRING\n");$$=(expr*)newexpr(conststring_e);$$->strConst=$1;}
				|NIL 	{printf("NIL\n");$$=(expr*)newexpr(nil_e);}
				|TRUE 	{printf("TRUE\n");$$=(expr*)newexpr(constbool_e);$$->boolConst=1;}
				|FALSE 	{printf("FALSE\n");$$=(expr*)newexpr(constbool_e);$$->boolConst=0;}
				;

idlist:			ID {printf("idlist ID\n");
					res = LookUp($1, curr_scope+1, curtype);
					if (res==NULL){
						Insert(1, "formal", $1, curr_scope+1, yylineno);}
					else {
						if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
						else {rescope=res -> value.varVal->scope;}
						if(strcmp(res->type,"libfunc")==0) {
							printf("Formal argument conflicts with libfunc: %s\n", $1);
						}
						else if (rescope == (curr_scope+1)){
							printf("Formal redeclaration for variable: %s\n", $1);
						}
						else {
							Insert(1, "formal", $1, curr_scope+1, yylineno);
						}
					}
					}
				|idlist ',' ID {printf("idlist ID\n");
					res = LookUp($3, curr_scope+1, curtype);
					if (res==NULL){
						Insert(1, "formal", $3, curr_scope+1, yylineno);}
					else {
						if (res -> value.funcVal != NULL){rescope=res -> value.funcVal->scope;}
						else {rescope=res -> value.varVal->scope;}
						if(strcmp(res->type,"libfunc")==0) {
							printf("Formal argument conflicts with libfunc: %s\n", $3);
						}
						else if (rescope == (curr_scope+1)){
							printf("Formal redeclaration for variable: %s\n", $3);
						}
						else {
							Insert(1, "formal", $3, curr_scope+1, yylineno);
						}
					}
					}
				|
				;

ifprefix:		IF '(' expr ')'	{ 	emit(if_eq, newexpr_constbool(1), $3, NULL, nextquad()+3, yylineno);
								$$ =  nextquad();
								emit(jump, NULL, NULL, NULL, -1, yylineno);
							}
				;

elseprefix: 	ELSE 	{$$ = nextquad();
						emit(jump, NULL, NULL, NULL, -1, yylineno);}
				;

ifstmt:			ifprefix stmt elseprefix stmt {patchlabel($1, $3+2);
												patchlabel($3, nextquad()+1);
												printf("IF statement ELSE statement\n");}
				|ifprefix stmt  {patchlabel($1, nextquad()+1);printf("IF statement\n");}
				;

whilestart: WHILE 			{$$ = nextquad()+1;}
whilecond: '(' expr ')'			{emit(if_eq, newexpr_constbool(1), NULL, $2, nextquad()+3, yylineno);
							$$ = nextquad();
							emit(jump, NULL, NULL, NULL, -1, yylineno);}
whilestmt:			 whilestart whilecond {++loopcounter;elflag=1;} stmt	{--loopcounter;elflag=0;emit(jump, NULL, NULL, NULL, $1, yylineno);
											patchlabel($2, nextquad()+1);printf("WHILE statement\n");
											struct breaklist* trailer = bclist->breaklist;
											while (trailer->q!=-1){
												if (trailer->q > $2 && trailer->active == 1) { patchlabel(trailer->q-1,nextquad()+1);trailer->active=0;}trailer=trailer->next;
											}
											struct contlist* trailer2 = bclist->contlist;
											while (trailer2->q!=-1){
												if (trailer2->q > $2 && trailer2->active == 1) { patchlabel(trailer2->q-1,$1);trailer2->active=0;}trailer2=trailer2->next;
											}
											}

N: 				{ $$ = nextquad(); emit(jump, NULL, NULL, NULL, -1, yylineno); }
				;
M:				{ $$ = nextquad(); }
				;

forprefix: 		FOR '(' {elflag=1;} elist ';' M expr ';'
				{
				$$ = (struct forpr*)malloc(sizeof(struct forpr));
				$$->test = $6;
				$$->enter = nextquad()+1;
				emit(if_eq, $7, newexpr_constbool(1), NULL, -1, yylineno);
				}
				;

forstmt:		forprefix N elist ')' N stmt N{elflag=0;
				patchlabel($1->enter-1, $5+2);
				patchlabel($2, nextquad()+1);
				patchlabel($5, $1->test+1);
				patchlabel($7, $2+2);
				struct breaklist* trailer = bclist->breaklist;
				while (trailer->q!=-1){
					if (trailer->q > $1->enter && trailer->active == 1) { patchlabel(trailer->q-1,nextquad()+1);trailer->active=0;}trailer=trailer->next;
				}
				struct contlist* trailer2 = bclist->contlist;
				while (trailer2->q!=-1){
					if (trailer2->q > $1->enter && trailer2->active == 1) { patchlabel(trailer2->q-1,$1->enter+2);trailer2->active=0;}trailer2=trailer2->next;
				}
				printf("FOR statement");}
				;

returnstmt:		RETURN ';' 	{printf("RETURN \n"); emit(ret,NULL,NULL,NULL,-1,yylineno);}
				|RETURN expr ';' {printf("RETURN EXPRESSION\n");emit(ret,NULL,NULL,$2,-1,yylineno);}
				;

%%


int yyerror(char* yaccProvidedMessage){
	fprintf(stderr, "%s: at line %d before token: %s\n", yaccProvidedMessage, yylineno, yytext);
	fprintf(stderr, "INPUT NOT VALID\n");
}
int main(int argc, char** argv){
	bclist = (struct statement*)malloc(sizeof(struct statement));
	bclist-> breaklist = (struct breaklist*)malloc(sizeof(struct breaklist));
	bclist->breaklist->q = -1;
	bclist-> contlist = (struct contlist*)malloc(sizeof(struct contlist));
	bclist->contlist->q = -1;
	bclist-> retlist = (struct retlist*)malloc(sizeof(struct retlist));
	bclist->retlist->q = -1;
	int i;
	curtype = malloc(10);
	infunc = 0;
	for (i = 0; i < 20; i++) {
		func_initial_scope[i] = 0;
	}
	linec = 0;
	curtype = "global";
	init();
	if(argc > 1){
		if(!(yyin = fopen(argv[1], "r"))){
			fprintf(stderr, "Cannot read file: %s\n", argv[1]);
			return 1;
		}
	}else{
		yyin = stdin;
	}
	yyparse();
	Print();
	printquads();
	generateAll();
	return 0;
}