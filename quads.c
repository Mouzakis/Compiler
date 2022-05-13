#include "quads.h"

quad* quads = (quad*) 0;
unsigned totalq=0;
unsigned int currQuad=0;
unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;
int tempcounter = 0;


unsigned nextquad(void){
	return currQuad;
}
scopespace_t currscopespace(void){
	if(scopeSpaceCounter == 1){
		return programvar;
	}
	else {
		if(scopeSpaceCounter % 2 == 0){
			return formalarg;
		}
		else {
			return functionlocal;
		}
	}
}

unsigned currscopeoffset(void){
	switch(currscopespace()){
		case programvar: return programVarOffset;
		case functionlocal: return functionLocalOffset;
		case formalarg: return formalArgOffset;
		default: assert(0);
	}
}

void incurrscopeoffset(void){
	switch(currscopespace()){
		case programvar: ++programVarOffset;break;
		case functionlocal: ++functionLocalOffset;break;
		case formalarg: ++formalArgOffset;break;
		default: assert(0);
	}
}

void restorecurrscopeoffset(unsigned n){
	switch(currscopespace()){
		case programvar    :   programVarOffset = n;      break;
		case functionlocal :   functionLocalOffset = n;   break;
		case formalarg     :   formalArgOffset = n;       break;
		default            :   assert(0);
	}
}

void addlocal(unsigned n){
	locals_s *tmp = (locals_s*)malloc(sizeof(locals_s));
	tmp -> functionLocalOffset = n;

	if(hlocal == NULL){
		tmp -> next = NULL;
		hlocal = tmp;
	}

	tmp -> next = hlocal;
	hlocal = tmp;
}


unsigned readlocal(void){
	unsigned n;
	locals_s *tmp;
	tmp = hlocal;
	hlocal = hlocal -> next;
	n = tmp -> functionLocalOffset;
	free(tmp);	
}

void enterscopespace(void){
	++scopeSpaceCounter;
}

void exitscopespace(void){
	assert(scopeSpaceCounter > 1);
	--scopeSpaceCounter;
}


void resetformalargsoffset(void) {
	formalArgOffset = 0;
}

void resetfunctionlocaloffset(void) {
	functionLocalOffset = 0;
}

void expand(void){
	assert(totalq==currQuad);
	quad* p = (quad*)malloc(NEW_SIZE);
	if(quads){
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	totalq += EXPAND_SIZE;
}

void emit(iopcode op, expr *arg1, expr* arg2, expr *result, unsigned label, unsigned line){
	if(currQuad == totalq){
		expand();
	}

	quad* p = quads + currQuad++;
	p->op = op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;
}

expr *newexpr(expr_t t){
	expr *e = (expr*)malloc(sizeof(expr));
	memset(e, 0, sizeof(expr));
	e->type = t;
	
	return e;
}
expr* newexpr_constbool(unsigned char b){
	expr* e = newexpr(constbool_e);
	e->boolConst = b;
	return e;
}

expr* newexpr_constint(double num) {
	expr* e = newexpr(constint_e);
	e->numConst = num;
	return e;
}

expr* lvalue_expr(struct SymbolTableEntry *sym){
		expr *tmp = (expr*)malloc(sizeof(expr));

		 if(strcmp(sym -> type ,"programfunc_s"))
		 	tmp -> type = programfunc_e;
		

		 if(strcmp(sym -> type, "libraryfunc_s"))
		 	tmp -> type = libraryfunc_e;
		
		 if(strcmp(sym -> type , "var_e"))
		 	tmp -> type == var_e;

		 tmp -> sym = sym;
		 tmp -> next = NULL;		
		 return tmp;
}

char* newtempname(void) {
	char *temp = (char*)malloc(sizeof(char)*4);
	sprintf(temp, "_t%d", tempcounter);
	tempcounter++;
	return temp;
}
void resettemp(void){ tempcounter = 0; }

struct SymbolTableEntry *newtemp(int scope, int line){
	if (scope == 0){return Insert2(1,"global", newtempname(), scope, line);}
	else {return Insert2(1,"local", newtempname(), scope, line);}
}

void patchlabel(unsigned q, unsigned label) {
	quads[q].label = label;
}

void checkuminus(expr* e) {
	if (e->type == constbool_e ||
		e->type == conststring_e ||
		e->type == nil_e ||
		e->type == newtable_e ||
		e->type == programfunc_e ||
		e->type == libraryfunc_e ||
		e->type == boolexpr_e)
	{
		printf("Error - Illegal expression to unary -");
	}
}

char* opToString(enum iopcode op) {
	char *str = (char*)malloc(18*sizeof(char));
	if (op == add){
		str = "ADD";
	}
	else if (op == assign) {
		str = "ASSIGN";
	}
	else if (op == divi) {
		str = "DIV";
	}
	else if (op == mul) {
		str = "MUL";
	}
	else if (op == sub) {
		str = "SUB";
	}
	else if (op == mod) {
		str = "MOD";
	}
	else if (op == and) {
		str = "AND";
	}
	else if (op == or) {
		str = "OR";
	}
	else if (op == not) {
		str = "NOT";
	}
	else if (op == if_greater) {
		str = "IF_GREATER";
	}
	else if (op == if_eq) {
		str = "IF_EQUALS";
	}
	else if (op == if_noteq) {
		str = "IF_NOT_EQUAL";
	}
	else if (op == if_greatereq) {
		str = "IF_GREATER_EQ";
	}
	else if (op == if_lesseq) {
		str = "IF_LESS_EQ";
	}
	else if (op == if_less) {
		str = "IF_LESS";
	}
	else if (op == jump) {
		str = "JUMP";
	}
	else if(op == funcstart){
		str = "FUNCTSTART";
	}
	else if(op == funcend){
		str = "FUNCEND";
	}
	else if(op == uminus) {
		str = "UMINUS";
	}
	else if (op == tablecreate){
		str = "TABLECREATE";
	}
	else if (op == tablegetelem){
		str = "TABLEGETELEM";
	}
	else if (op == tablesetelem){
		str = "TABLESETELEM";
	}
	else if (op == getretvar){
		str = "GETRETVAR";
	}
	else if (op == call){
		str = "CALL";
	}
	else if (op == param){
		str = "PARAM";
	}
	else if (op == ret){
		str = "RETURN";
	}
	return str;
}

void printquads(void) {
	FILE *f = NULL;
	int i;
    f = fopen("quads.txt" ,"w");
	fprintf(f,"Quad#	Opcode	Result	Arg1	Arg2	Label\n\n");
	printf("Quads %d\n", currQuad);
	for(i=0; i<currQuad; i++){
		fprintf(f,"%d:	%s",i+1,opToString(quads[i].op));
		if(quads[i].result != NULL){
			switch(quads[i].result -> type) {
				case constint_e:
					fprintf(f,"	%d", (int)quads[i].result->numConst);
					break;
				case constdouble_e:
					fprintf(f,"	%f", quads[i].result->numConst);
					break;
				case  constbool_e:
					if (quads[i].result->boolConst == 0) {
						fprintf(f,"	%s","FALSE");
					}
					else {
						fprintf(f,"	%s","TRUE");
					}
					break;
				case conststring_e:
					fprintf(f,"	\"%s\"",quads[i].result->strConst);
					break;
				default :
					if(quads[i].result->sym != NULL){
						fprintf(f,"	%s", quads[i].result->sym->value.varVal->name);
					}
					else{
						fprintf(f,"	%s", "NULL");
					}
					break;
			}
		}

		if(quads[i].arg1 != NULL){
			switch(quads[i].arg1 -> type) {
				case constint_e:
					fprintf(f,"	%d", (int)quads[i].arg1->numConst);
					break;
				case constdouble_e:
					fprintf(f,"	%f", quads[i].arg1->numConst);
					break;
				case  constbool_e:
					if (quads[i].arg1->boolConst == 0) {
						fprintf(f,"	%s","FALSE");
					}
					else {
						fprintf(f,"	%s","TRUE");
					}
					break;
				case conststring_e:
					fprintf(f,"	\"%s\"",quads[i].arg1->strConst);
					break;
				default :
					if(quads[i].arg1->sym != NULL){
						fprintf(f,"	%s", quads[i].arg1->sym->value.varVal->name);
					}
					else{
						fprintf(f,"	%s", "NULL");
					}
					break;
			}
		}
		if(quads[i].arg2 != NULL){
			switch(quads[i].arg2 -> type) {
				case constint_e:
					fprintf(f,"	%d", (int)quads[i].arg2->numConst);
					break;
				case constdouble_e:
					fprintf(f,"	%f", quads[i].arg2->numConst);
					break;
				case  constbool_e:
					if (quads[i].arg2->boolConst == 0) {
						fprintf(f,"	%s","FALSE");
					}
					else {
						fprintf(f,"	%s","TRUE");
					}
					break;
				case conststring_e:
					fprintf(f,"	\"%s\"",quads[i].arg2->strConst);
					break;
				default :
					if(quads[i].arg2->sym != NULL){
						fprintf(f,"	%s", quads[i].arg2->sym->value.varVal->name);
					}
					else{
						fprintf(f,"	%s", "NULL");
					}
					break;
			}
		}

		if (quads[i].label != -1) {
			fprintf(f,"\t%d", quads[i].label);
		}
		fprintf(f,"	[Line %d]\n", quads[i].line);
	}
	fclose(f);
}

expr* emit_iftableitem (expr* e, int scope, int line){
	if (e->type != tableitem_e){
		return e;
	}
	else {
		expr* result = newexpr(var_e);
		result -> sym = newtemp(scope, line);
		emit(tablegetelem, e, e->index, result, -1, line);
		return result;
	}
}

expr* newexpr_conststring (char* s){
	expr* e = newexpr(conststring_e);
	e->strConst = strdup(s);
	return e;
}

expr* member_item(expr* lvalue, char* name, int scope, int line){
	lvalue = emit_iftableitem(lvalue, scope, line);
	expr* item = newexpr(tableitem_e);
	item->sym = lvalue -> sym;
	item->index = newexpr_conststring(name);
	return item;
}

expr* make_call(expr* lvalue, expr* elist, int scope, int line){
	expr* func = emit_iftableitem(lvalue, scope, line);
	expr* e = elist;
	while(e != NULL){
		emit(param, NULL, NULL, e, -1, line);
		e = e -> next;
	}
	emit(call, NULL, NULL, func, -1, line);
	expr* result = newexpr(var_e);
	result -> sym = newtemp(scope, line);
	emit(getretvar, NULL, NULL, result, -1, line);
	return result;
}
void add_front(struct strcall * c, expr* el) {
	el->next = c->elist;
	c->elist = el;
}
expr* reverselist(expr* head) {
	int count = 0;
	expr* prev = NULL;
	expr* next = NULL;
	expr* trailer = head;
	while (trailer != NULL) {
		next = trailer -> next;
		trailer->next = prev;
		prev = trailer;
		trailer = next;
	}
	return prev;
}

struct indexed* reverseindexed(struct indexed* head) {
	int count = 0;
	struct indexed* prev = NULL;
	struct indexed* next = NULL;
	struct indexed* trailer = head;
	while (trailer != NULL) {
		next = trailer -> next;
		trailer->next = prev;
		prev = trailer;
		trailer = next;
	}
	return prev;
}