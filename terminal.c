#include "terminal.h"
#include <assert.h>

instruction* instructions = (instruction*) 0;
unsigned int totalInstr = 0;
unsigned int currInstr = 0;
unsigned currQuad;
unsigned currprocessedquadNum = 0;
unsigned nop_pos;

incomplete_jump* ij_head;
unsigned ij_total = 0;

double  *numtable;
unsigned totaltablenum = 0;

char  **strtable;
unsigned totaltablestr = 0;

unsigned totalvartable = 0;
vartable* vartablehead = NULL;

unsigned localtotalvartable = 0;
localvartable* localvartablehead = NULL;

unsigned formaltotalvartable = 0;
formalvartable* formalvartablehead = NULL;

struct userfunc *functbl;
unsigned totalfuncs = 0;

char **libFuncTbl;
unsigned totalLibFuncs = 0;

void generate(vmopcode op, quad* q) {
	instruction *t = (struct instruction *)malloc(sizeof(struct instruction));
	t->opcode = op;
	
	if(q->arg2 !=NULL){make_operand(q->arg1, &t->arg1);}
	if(q->arg2 !=NULL){make_operand(q->arg2, &t->arg2);}
	else {reset_operand(&t->arg2);}
	if(q->result !=NULL){make_operand(q->result, &t->result);}
	else {reset_operand(&t->result);}
	q->address = nextinstructionlabel();
	vmemit(t);
}

void make_numberoperand (vmarg* arg, double val){
	arg->val = consts_newnumber(val);
	arg->type = number_a;
}

void make_booloperand (vmarg* arg, unsigned val){
	arg->val = val;
	arg->type = bool_a;
}

void make_retvaloperand (vmarg* arg){
	arg->type = retval_a;
}


void make_operand (expr* e, vmarg* arg) {
	switch (e->type) {
		case var_e:
		case tableitem_e:
		case arithexpr_e:
		case boolexpr_e:
		case newtable_e:
		case assignexpr_e: {
			assert(e->sym);
			arg->val = e->sym->offset;

			switch (e->sym->space) {
				case programvar: arg->type = global_a;new_var(strdup(e->sym->value.varVal->name), arg->val); break;
				case functionlocal: arg->type = local_a;new_localvar(strdup(e->sym->value.varVal->name), arg->val); break;
				case formalarg: arg->type = formal_a;new_formalvar(strdup(e->sym->value.varVal->name), arg->val); break;
				default: assert(0);
			}
			break;
		}
		case constbool_e: {
			arg->val = e->boolConst;
			arg->type = bool_a; break;
		}
		case conststring_e: {
			arg->val = consts_newstring(e->strConst);
			arg->type = string_a; break;
		}
		case constint_e:case constdouble_e: {
			arg->val = consts_newnumber(e->numConst);
			arg->type = number_a; break;
		}
		case programfunc_e: {
			arg->type = userfunc_a;
			arg->val = totalfuncs-1;
			break;
		}
		case libraryfunc_e: {
			arg->type = libfunc_a;
			arg->val = new_LibFunc(strdup(e->sym->value.funcVal->name));
			break;
		}
		default: assert(0);
	}
}

typedef void (*generator_func_t)(quad*);

generator_func_t generators[] = {
  generate_add
};
unsigned currprocessedquad(void) {
	return currprocessedquadNum;
}
void generateAll(void) {
	numtable = NULL;
	int i;
	for (currprocessedquadNum = 0,i = 0; currprocessedquadNum<currQuad; ++currprocessedquadNum, i++) {
		switch(quads[i].op) {
			case add:
				generate_add(&quads[i]);
				break;
			case sub:
				generate_sub (&quads[i]); 
				break;
			case mul:
				generate_mul (&quads[i]); 
				break;
			case divi:
				generate_divi(&quads[i]); 
				break;
			case mod:
				generate_mod (&quads[i]); 
				break;
			case tablecreate:
				generate_newtable (&quads[i]);
				break;
			case tablegetelem:
				generate_tablegetelem(&quads[i]); 
				break;
			case tablesetelem:
				generate_tablesetelem(&quads[i]); 
				break;
			case assign:
				generate_assign(&quads[i]);
				break;
			// case nop:
			// 	generate_nop (&quads[i]);
			// 	break;
			case jump:
			 	generate_relational(jump_v,&quads[i]); 
			 	break;
			case if_eq:
				generate_relational(jeq_v,&quads[i]);
				break;
			case if_noteq:
				generate_relational (jne_v,&quads[i]);
				break;
			case if_greater:
				generate_relational (jgt_v,&quads[i]);
				break;
			case if_greatereq:
				generate_relational (jge_v,&quads[i]); 
				break;
			case if_less:
				generate_relational (jlt_v,&quads[i]); 
				break;
			case if_lesseq:
				generate_relational (jle_v,&quads[i]); 
				break;
			case not:
				generate_not (&quads[i]);
				break;
			case or:
				generate_or (&quads[i]); 
				break;
			case and:
				generate_and (&quads[i]); 
				break;
			case param:
				generate_param (&quads[i]);
				break;
			case call:
				generate_call (&quads[i]);
				break;
			case getretvar:
				generate_getretval (&quads[i]);
				break;
			// case puncstart:
			// 	generate_puncstart (&quads[i]); 
			// 	break;
			case ret:
				generate_return (&quads[i]);
				break;
			case funcstart:
				generate_funcstart (&quads[i]);
				break;
			case funcend:
				generate_funcend (&quads[i]);
				break;
			default: break;
		}
	}
	patch_incomplete_jumps();
	printf("\n - - - Instructions - - -\n");
	for (i = 0; i<currInstr; i++) {
		printf("%s", vmopcodeToStr(instructions[i].opcode));
		if (instructions[i].opcode != jump_v) {
			if(instructions[i].result.val != -1 && instructions[i].result.type != retval_a){printf("\tType:%d, %d", instructions[i].result.type,instructions[i].result.val);}
			else if (instructions[i].result.type == retval_a){printf("\t10(retval)");}
			if (instructions[i].result.type == global_a){printf(":%s", find_var(instructions[i].result.val));}
			if (instructions[i].result.type == userfunc_a){printf(":%s", functbl[instructions[i].result.val].id);}
			if (instructions[i].result.type == local_a){printf(":%s", find_localvar(instructions[i].result.val));}
			if (instructions[i].result.type == formal_a){printf(":%s", find_formalvar(instructions[i].result.val));}

			if(instructions[i].arg1.val != -1){printf("\tType:%d, %d",instructions[i].arg1.type,instructions[i].arg1.val);}
			if (instructions[i].arg1.type == number_a){printf(":%f", numtable[instructions[i].arg1.val]);}
			if (instructions[i].arg1.type == string_a){printf(":%s", strtable[instructions[i].arg1.val]);}
			if (instructions[i].arg1.type == global_a){printf(":%s", find_var(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == formal_a){printf(":%s", find_formalvar(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == local_a){printf(":%s", find_localvar(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == libfunc_a){printf(":%s", libFuncTbl[instructions[i].arg1.val]);}

			if(instructions[i].arg2.val != -1){printf("\tType:%d, %d",instructions[i].arg2.type,instructions[i].arg2.val);}
			if (instructions[i].arg2.type == number_a){printf(":%f", numtable[instructions[i].arg2.val]);}
			if (instructions[i].arg2.type == string_a){printf(":%s", strtable[instructions[i].arg2.val]);}
			if (instructions[i].arg2.type == global_a){printf(":%s", find_var(instructions[i].arg2.val));}
			if (instructions[i].arg2.type == local_a){printf(":%s", find_localvar(instructions[i].arg2.val));}
			if (instructions[i].arg2.type == formal_a){printf(":%s", find_formalvar(instructions[i].arg2.val));}

			switch(instructions[i].opcode)
				case jeq_v:case jne_v:case jle_v:case jge_v: case jlt_v:case jgt_v:
					printf("\t%d",instructions[i].result.val);
		}
		else {
			printf("\t%d", instructions[i].result.val);
		}
		printf("\n");
		nop_pos = totalInstr - 1;
	}

	FILE *fpointer = fopen("instructions.txt", "w");


	fprintf(fpointer, "magicnumber	%u\n", 340200501);

	fprintf(fpointer, "\nstrings" );

	if(totaltablestr != 0){
		
		for(i=0; i<totaltablestr; i++){
			fprintf(fpointer, ",%s", strtable[i]);
		}

	}

	fprintf(fpointer, "\nnumbers" );


	if(totaltablenum != 0){
				for(i=0; i<totaltablenum; i++){
			fprintf(fpointer, ",%f", numtable[i]);
		}
	}

	/*fprintf(fpointer, "\nuserfunctions\n" );

	if(totalfuncs != 0){
		
		for (i = 0; i < totalfuncs; i++)
		{
			fprintf(fpointer, "%u\n", functbl[i].address );
			fprintf(fpointer, "%u\n", functbl[i].localSize);
			fprintf(fpointer, "%s\n", functbl[i].id);
		}
	}*/

	/*fprintf(fpointer, "libfunctions\n");

	if(totalLibFuncs != 0){
		
		for (i = 0; i < totalLibFuncs; i++)
		{
			fprintf(fpointer, "%s\n", libFuncTbl[i] );
		}
	}*/


	fprintf(fpointer, "\nInstructions\n");
	for (i = 0; i<currInstr; i++) {
		fprintf(fpointer,"%d", instructions[i].opcode);
		if (instructions[i].opcode != jump_v) {
			if(instructions[i].result.val != -1 && instructions[i].result.type != retval_a){fprintf(fpointer," %d %d", instructions[i].result.type,instructions[i].result.val);}
			else if (instructions[i].result.type == retval_a){fprintf(fpointer," 10");}
			/*if (instructions[i].result.type == global_a){fprintf(fpointer,":%s", find_var(instructions[i].result.val));}
			if (instructions[i].result.type == userfunc_a){fprintf(fpointer,":%s", functbl[instructions[i].result.val].id);}
			if (instructions[i].result.type == local_a){fprintf(fpointer,":%s", find_localvar(instructions[i].result.val));}
			if (instructions[i].result.type == formal_a){fprintf(fpointer,":%s", find_formalvar(instructions[i].result.val));}*/

			if(instructions[i].arg1.val != -1){fprintf(fpointer," %d %d",instructions[i].arg1.type,instructions[i].arg1.val);}
			/*if (instructions[i].arg1.type == number_a){fprintf(fpointer,":%f", numtable[instructions[i].arg1.val]);}
			if (instructions[i].arg1.type == string_a){fprintf(fpointer,":%s", strtable[instructions[i].arg1.val]);}
			if (instructions[i].arg1.type == global_a){fprintf(fpointer,":%s", find_var(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == formal_a){fprintf(fpointer,":%s", find_formalvar(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == local_a){fprintf(fpointer,":%s", find_localvar(instructions[i].arg1.val));}
			if (instructions[i].arg1.type == libfunc_a){fprintf(fpointer,":%s", libFuncTbl[instructions[i].arg1.val]);}*/

			if(instructions[i].arg2.val != -1){fprintf(fpointer," %d %d",instructions[i].arg2.type,instructions[i].arg2.val);}
			/*if (instructions[i].arg2.type == number_a){fprintf(fpointer,":%f", numtable[instructions[i].arg2.val]);}
			if (instructions[i].arg2.type == string_a){fprintf(fpointer,":%s", strtable[instructions[i].arg2.val]);}
			if (instructions[i].arg2.type == global_a){fprintf(fpointer,":%s", find_var(instructions[i].arg2.val));}
			if (instructions[i].arg2.type == local_a){fprintf(fpointer,":%s", find_localvar(instructions[i].arg2.val));}
			if (instructions[i].arg2.type == formal_a){fprintf(fpointer,":%s", find_formalvar(instructions[i].arg2.val));}*/

			switch(instructions[i].opcode)
				case jeq_v:case jne_v:case jle_v:case jge_v: case jlt_v:case jgt_v:
					fprintf(fpointer," %d",instructions[i].result.val);
		}
		else {
			fprintf(fpointer," %d", instructions[i].result.val);
		}

		fprintf(fpointer," NOP\n");
		nop_pos = totalInstr - 1;
	}

	//fprintf(fpointer,"NOP ");
	
}
char* find_formalvar(unsigned pos) {
	formalvartable* trailer = formalvartablehead;
	while (trailer != NULL && trailer->pos != pos) {
		trailer = trailer ->next;
	}
	return trailer -> name;
}
void new_formalvar(char* name, unsigned pos) {
	formalvartable* new = (struct formalvartable*)malloc(sizeof(struct formalvartable));
	new -> name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	new -> name = name;
	new->next = NULL;
	if (formalvartablehead == NULL) {
		formalvartablehead = new;
		new -> pos = pos;
	}
	else {
		formalvartable* trailer = formalvartablehead;
		while (trailer->next != NULL) {
			trailer = trailer -> next;
		}
		trailer->next = new;
		trailer->next->pos = pos;
	}
	formaltotalvartable += 1;
}
char* find_localvar(unsigned pos) {
	int i = 0;
	localvartable* trailer = localvartablehead;
	while (trailer != NULL && trailer->pos != pos) {
		trailer = trailer ->next;
	}
	return trailer -> name;
}
void new_localvar(char* name, unsigned pos) {
	localvartable* new = (struct localvartable*)malloc(sizeof(struct localvartable));
	new -> name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	new -> name = name;
	new->next = NULL;
	if (localvartablehead == NULL) {
		localvartablehead = new;
		new -> pos = pos;
	}
	else {
		localvartable* trailer = localvartablehead;
		while (trailer->next != NULL) {
			trailer = trailer -> next;
		}
		trailer->next = new;
		trailer->next->pos = pos;
	}
	localtotalvartable += 1;
}
char* find_var(unsigned pos) {
	int i = 0;
	vartable* trailer = vartablehead;
	while (trailer != NULL && trailer->pos != pos) {
		trailer = trailer ->next;
	}
	return trailer -> name;
}
void new_var(char* name, unsigned pos) {
	vartable* new = (struct vartable*)malloc(sizeof(struct vartable));
	new -> name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	new -> name = name;
	new->next = NULL;
	if (vartablehead == NULL) {
		vartablehead = new;
		new -> pos = pos;
	}
	else {
		vartable* trailer = vartablehead;
		while (trailer->next != NULL) {
			trailer = trailer -> next;
		}
		trailer->next = new;
		trailer->next->pos = pos;
	}
	totalvartable += 1;
}

unsigned nextinstructionlabel(void) {
	return currInstr;
}

void expand_instructions(void){
	assert(totalInstr==currInstr);
	instruction *t = (instruction*)malloc(NEW_SIZE_INSTR);
	if(instructions){
		memcpy(t, instructions, CURR_SIZE_INSTR);
		free(instructions);
	}
	instructions = t;
	totalInstr += EXPAND_SIZE_INSTR;
}

void vmemit(instruction* t){
	if(currInstr == totalInstr){
		expand_instructions();
	}

	instruction* n = instructions + currInstr++;
	n->opcode = t->opcode;
	n->arg1 = t->arg1;
	n->arg2 = t->arg2;
	n->result = t->result;
	n->srcLine = t->srcLine;
}
char* vmopcodeToStr(vmopcode op){
	char *str = (char*)malloc(18*sizeof(char));
	if (op == add_v){
		str = "ADD";
	}
	else if (op == assign_v) {
		str = "ASSIGN";
	}
	else if (op == div_v) {
		str = "DIV";
	}
	else if (op == mul_v) {
		str = "MUL";
	}
	else if (op == sub_v) {
		str = "SUB";
	}
	else if (op == mod_v) {
		str = "MOD";
	}
	else if (op == and_v) {
		str = "AND";
	}
	else if (op == or_v) {
		str = "OR";
	}
	else if (op == not_v) {
		str = "NOT";
	}
	else if (op == jump_v) {
		str = "JUMP";
	}
	else if (op == jgt_v) {
		str = "IF_GREATER";
	}
	else if (op == jeq_v) {
		str = "IF_EQUALS";
	}
	else if (op == jne_v) {
		str = "IF_NOT_EQUAL";
	}
	else if (op == jeq_v) {
		str = "IF_GREATER_EQ";
	}
	else if (op == jle_v) {
		str = "IF_LESS_EQ";
	}
	else if (op == jlt_v) {
		str = "IF_LESS";
	}
	// else if (op == jump) {
	// 	str = "JUMP";
	// }
	else if(op == funcenter_v){
		str = "FUNCTSTART";
	}
	else if(op == funcexit_v){
		str = "FUNCEND";
	}
	else if (op == newtable_v){
		str = "TABLECREATE";
	}
	else if (op == tablegetelem_v){
		str = "TABLEGETELEM";
	}
	else if (op == tablesetelem_v){
		str = "TABLESETELEM";
	}
	// else if (op == getretvar_v){
	// 	str = "GETRETVAR";
	// }
	else if (op == call_v){
		str = "CALL";
	}
	else if (op == pusharg_v){
		str = "PUSHARG";
	}
	// else if (op == ret_v){
	// 	str = "RETURN";
	// }
	return str;
}
void generate_add(quad* q) {
	generate(add_v, q);
}
void generate_sub(quad* q) {
	generate(sub_v, q);
}
void generate_mul(quad* q) {
	generate(mul_v, q);
}
void generate_divi(quad* q) {
	generate(div_v, q);
}
void generate_mod (quad* q){
	generate(mod_v, q);
}
void generate_newtable (quad* q){
	generate(newtable_v, q);
}
void generate_tablegetelem (quad* q){
	generate(tablegetelem_v, q);
}
void generate_tablesetelem (quad* q){
	generate(tablesetelem_v, q);
}
void generate_assign(quad* q){
	generate(assign_v,q);
}
//void generate_nop (quad* q){}
void add_incomplete_jump(unsigned instrNo, unsigned iaddress) {
	incomplete_jump* new = (struct incomplete_jump*)malloc(sizeof(struct incomplete_jump));
	new->instrNo = instrNo;
	new->iaddress = iaddress;
	new->next = NULL;
	if (ij_total == 0) {
		ij_head = new;
	}
	else {
		incomplete_jump* trailer = ij_head;
		int i;
		for (i = 1; i< ij_total; i++) {
			trailer = trailer->next;
		}
		trailer->next = new;
	}
	ij_total += 1;
}
void patch_incomplete_jumps(void) {
	incomplete_jump* x = ij_head;
	while (x!=NULL) {
		if (x->iaddress == currQuad+1) {
			instructions[x->instrNo].result.val = currInstr+1;
		}
		else {
			instructions[x->instrNo].result.val = quads[x->iaddress].address;
		}
		x = x->next;
	}
}
void generate_relational(vmopcode op,quad* q){
	instruction *t = (struct instruction*)malloc(sizeof(struct instruction));
	t->opcode = op;
	if(q->arg1!=NULL){make_operand(q->arg1, &t->arg1);}else{reset_operand(&t->arg1);}
	if(q->arg2!=NULL){make_operand(q->arg2, &t->arg2);}else{reset_operand(&t->arg2);}

	t->result.type = label_a;
	if (q->label < currprocessedquad()) {
		t->result.val = quads[q->label].address;
	}
	else {
		add_incomplete_jump(nextinstructionlabel(), q->label);
	}
	quads[currprocessedquadNum].address = nextinstructionlabel();
	vmemit(t);
}
void generate_not (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	q->address = nextinstructionlabel();

	t->opcode = jeq_v;
	make_operand(q->arg1, &t->arg1);
	make_booloperand(&t->arg2, 0);
	t->result.val = nextinstructionlabel() + 3;

	vmemit(t);

	t->opcode = assign_v;
	make_booloperand(&t->arg2,0);
	make_operand(q->result, &t->result);
	make_operand(q->result,&t->result);
	vmemit(t);

	t->opcode = jump_v;
	reset_operand(&t->arg1);
	reset_operand(&t->arg2);
	t->result.type = label_a;
	t->result.val = nextinstructionlabel() + 2;

	vmemit(t);


	t->opcode = assign_v;
	make_booloperand(&t->arg1, 1);
	reset_operand(&t->arg2);
	make_operand(q->result,&t->result);
	vmemit(t);
}
void generate_or (quad* q){
	q->address = nextinstructionlabel();
	instruction *t = malloc(sizeof(struct instruction));

	t->opcode = jeq_v;
	make_operand(q->arg1, &t->arg1);
	make_booloperand(&t->arg2,1);
	t->result.type = label_a;
	t->result.val = nextinstructionlabel() + 4;
	vmemit(t);

	t->opcode = assign_v;
	make_booloperand(&t->arg1, 0);
	reset_operand(&t->arg2);
	make_operand(q->result, &t->result);
	vmemit(t);

	t->opcode = jump_v;
	reset_operand(&t->arg1);
	reset_operand(&t->arg2);
	t->result.val = nextinstructionlabel() + 2;
	vmemit(t);

	t->opcode = assign_v;
	make_booloperand(&t->arg1, 1);
	reset_operand(&t->arg2);
	make_operand(q->result, &t->result);
	vmemit(t);
}
void generate_and (quad* q){

	q->address = nextinstructionlabel();
	instruction *t = malloc(sizeof(struct instruction));

	t->opcode = jeq_v;
	make_operand(q->arg1, &t->arg1);
	make_booloperand(&t->arg2,1);
	t->result.type = label_a;
	t->result.val = nextinstructionlabel() + 4;
	vmemit(t);

	t->opcode = assign_v;
	make_booloperand(&t->arg1, 0);
	reset_operand(&t->arg2);
	make_operand(q->result, &t->result);
	vmemit(t);

	t->opcode = jump_v;
	reset_operand(&t->arg1);
	reset_operand(&t->arg2);
	t->result.val = nextinstructionlabel() + 2;
	vmemit(t);

	t->opcode = assign_v;
	make_booloperand(&t->arg1, 1);
	reset_operand(&t->arg2);
	make_operand(q->result, &t->result);
	vmemit(t);
}
void generate_param (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	q -> address = nextinstructionlabel();
	t->opcode = pusharg_v;
	make_operand(q->result, &t->arg1);
	reset_operand(&t->result);
	reset_operand(&t->arg2);
	vmemit(t);
}


void generate_call (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	q->address = nextinstructionlabel();
	t->opcode = call_v;
	make_operand(q->result, &t->arg1);
	reset_operand(&t->arg2);
	reset_operand(&t->result);
	vmemit(t);
}
void generate_getretval (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	q->address = nextinstructionlabel();
	t->opcode= assign_v;
	make_operand(q->result, &t->result);
	make_retvaloperand(&t->arg1);
	reset_operand(&t->arg2);
	vmemit(t);
}
//void generate_puncstart (quad* q){}

void generate_return (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	fstack *f;
	q->address = nextinstructionlabel();
	t->opcode = assign_v;
	make_retvaloperand(&t->result);
	make_operand(q->result, &t->arg1);
	reset_operand(&t->arg2);
	vmemit(t);

	f = fstacktop();
	append(f->funcreturns, nextinstructionlabel());

	t->opcode = jump_v;
	t->result.type = label_a;
	vmemit(t);
	
}
void generate_funcstart (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	struct SymbolTableEntry *f;
	fstack *s;
	f = q->arg1->sym;
	f->value.funcVal -> iaddress = nextinstructionlabel();
	q-> address = nextinstructionlabel();

	s = func_add(f->value.funcVal->name, f->value.funcVal->iaddress, f->value.funcVal->totallocals);
	pushfunc(s);

	t -> opcode = funcenter_v;
	userfuncs_newfunc(q->arg1->sym);
	make_operand(q->arg1, &t->result);
	reset_operand(&t->arg1);
	reset_operand(&t->arg2);
	vmemit(t);


}
void generate_funcend (quad* q){
	instruction *t = malloc(sizeof(struct instruction));
	fstack *f;
	f = fstackpop();
	backpatcth_ret(f->funcreturns, nextinstructionlabel()+2);

	q->address = nextinstructionlabel();
	t-> opcode = funcexit_v;

	make_operand(q->arg1, &t->result);
	reset_operand(&t->arg1);
	reset_operand(&t->arg2);
	vmemit(t);

}


fstack *func_add(const char *id, unsigned address, unsigned totallocals){
	fstack *s = malloc(sizeof(struct fstack));

	s->name=id;
	s->iaddress=address;
	s->totallocals =totallocals;
	s->funcreturns = NULL;
	s->next = NULL;
	return s;
}

void pushfunc(fstack *s){
	fstack *tmp = fshead;
	if(fshead == NULL){
		
		fshead = s;
		fshead -> next = NULL;
	}
	else{
		s -> next = tmp;
		fshead = s;
	}
}


fstack *fstacktop(){
	return fshead;
}


void append(funcreturn *r, unsigned n){
	
	funcreturn *tmp = fstacktop()->funcreturns;

	if(tmp == NULL){
		funcreturn *newRet = (struct funcreturn*)malloc(sizeof(struct funcreturn));
		newRet -> target_label = n;
		fstacktop()->funcreturns= newRet;
		fstacktop()->funcreturns->next=NULL;
	}
	else{
		while(tmp -> next != NULL){
			tmp = tmp -> next;
		}
		tmp -> next = (struct funcreturn*)malloc(sizeof(struct funcreturn));
		tmp -> next -> target_label = n;
		tmp -> next ->next=NULL;
	}
}


void backpatcth_ret(funcreturn *f, unsigned n){

	funcreturn *tmp = f;
	while(tmp != NULL){
		instructions[tmp->target_label].result.type = label_a;
		instructions[tmp->target_label].result.val = n;
		tmp = tmp -> next;
	}

}



fstack *fstackpop(){

	fstack *tmp = fshead;
	fshead = fshead -> next;
	return tmp;
}



unsigned consts_newstring(char* str) {
	int i;
	if (totaltablestr == 0) {
		strtable = (char**)malloc(sizeof(char*)*strlen(str));
	}
	else {
		strtable = (char**)realloc(strtable, (totaltablestr + 1)* strlen(str)* sizeof(char*));
	}
	strtable[totaltablestr] = (char*)malloc(sizeof(char)*strlen(str));
	strtable[totaltablestr] = str;
	totaltablestr += 1;
	return totaltablestr-1;
}

unsigned consts_newnumber(double num) {
	if (totaltablenum == 0) {
		numtable = (double*)malloc(sizeof(double));
	}
	else {
		numtable = (double*)realloc(numtable,(totaltablenum + 1)*sizeof(double));
	}
	numtable[totaltablenum] = num;
	totaltablenum += 1;
	return totaltablenum - 1;
}


void reset_operand(vmarg *arg){
	arg->val = -1;
}


unsigned userfuncs_newfunc(struct SymbolTableEntry *sym){
	//printf("\n%d\n",totalfuncs);
	if(totalfuncs == 0)
		functbl = (struct userfunc*)malloc(sizeof(struct userfunc));
	else
		functbl = (struct userfunc*)realloc(functbl,(totalfuncs + 1)*sizeof(struct userfunc));

	functbl[totalfuncs].address = sym -> value.funcVal -> iaddress;
	functbl[totalfuncs].localSize = sym ->value.funcVal -> totallocals;
	functbl[totalfuncs].id = (char*)malloc(sizeof(char)*strlen(sym->value.funcVal->name));
	strcpy(functbl[totalfuncs].id, sym->value.funcVal->name);
	totalfuncs += 1;

	return totalfuncs - 1;
}

unsigned new_LibFunc(char *s){
	int i;
	for(i=0; i<totalLibFuncs; i++){
		if(strcmp(s,libFuncTbl[i]) == 0){
			return i;
		}
	}

	if(totalLibFuncs == 0){
		libFuncTbl = (char**)malloc(sizeof(char*)*strlen(s)+1);
	}
	else{
		libFuncTbl = (char**)realloc(libFuncTbl, (totalLibFuncs + 1)* strlen(s)* sizeof(char*));
	}

	libFuncTbl[totalLibFuncs] =  (char*)malloc(sizeof(char)*(1+strlen(s)));
	strcpy(libFuncTbl[totalLibFuncs] ,s);
	totalLibFuncs+=1;

	return totalLibFuncs -1;

}
