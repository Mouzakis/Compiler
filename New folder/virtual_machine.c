
#include "virtual_machine.h"


#define AVM_ENDING_PC codeSize

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

int executionFinished = 0;
unsigned pc = 0;
unsigned currLine = 0;
unsigned codeSize = 0;
instruction *code = (instruction*) 0;



double  *constnumbers;
unsigned totalconstnumbers = 0;

char  **conststrs;
unsigned totalconststrs = 0;

unsigned totalActuals = 0;

static void avm_initstack (void){
	int i;

	for (i=0; i<AVM_STACKSIZE; ++i){

		AVM_WIPEOUT(stack[i]);
		stack[i].type = undef_m;
	}
}




struct avm_memcell* avm_translate_operand (vmarg* arg, avm_memcell* reg){
	switch (arg->type){
		case global_a:
			return &stack[AVM_STACKSIZE-1-arg -> val];
		case local_a:
			return &stack[topsp-arg->val];
		case formal_a:
			return &stack[topsp+AVM_STACKENV_SIZE+1+arg -> val];
		case retval_a:
			return &retval;
		case number_a:
			reg->type = number_m;
			//reg->data.numVal = consts_getnumber(arg->val);
			return reg;
		case string_a:
			reg->type = string_m;
			//reg->data.strVal = strdup(consts_getstring(arg->val));
			return reg;
		case bool_a:
			reg->type = bool_m;
			reg->data.boolVal = arg->val;
			return reg;
		case nil_a:
			reg->type = nil_m;
			return reg;
		case userfunc_a:
			reg->type = userfunc_m;
			reg->data.funcVal = arg->val;
			return reg;
		case libfunc_a:
			reg->type = libfunc_m;
			//reg->data.libfuncVal = libfuncs_getused(arg->val);
			return reg;
		default:
			assert(0);
	}
}



void avm_initialize (void){
	avm_initstack();

	// avm_registerlibfunc("print", libfunc_print);
	// avm_registerlibfunc("typeof", libfunc_typeof);

	/* Same for all the rest library functions. */
}






void avm_tableincrefcounter (avm_table* t){
	++t -> refCounter;
}


void avm_tabledecrefcounter (avm_table* t){

	assert(t -> refCounter > 0);
	if(!--t -> refCounter){
		avm_tabledestroy(t);
	}
}



void avm_tablebucketsinit (avm_table_bucket** p){

	int i;

	for ( i=0; i<AVM_TABLE_HASHSIZE; ++i){
		p[i] = (avm_table_bucket*) 0;
	}
}
void memclear_string (avm_memcell* m){}
void memclear_table(avm_memcell* m){}

avm_table* avm_tablenew (void){
	avm_table* t = (avm_table*) malloc (sizeof(avm_table));
	AVM_WIPEOUT(*t);
	t -> refCounter = t -> total = 0;
	avm_tablebucketsinit(t -> numIndexed);
	avm_tablebucketsinit(t -> strIndexed);
	return t;
}

void avm_memcellclear (avm_memcell* m){
	if(m->type != undef_m) {
		memclear_func_t f = memclearFuncs[m->type];
		if(f)
			(*f) (m);
		m->type = undef_m;
	 }
}
void avm_tablebucketsdestroy (avm_table_bucket** p){
	for(unsigned i=0; i<AVM_TABLE_HASHSIZE; ++i, ++p){
		for(avm_table_bucket* b = *p; b; ){
			avm_table_bucket* del = b;
			b = b -> next;
			avm_memcellclear (&del -> key);
			avm_memcellclear (&del -> value);
			free (del);
		}
		p[i] = (avm_table_bucket*) 0;
	}
}


void avm_tabledestroy (avm_table* t){
	avm_tablebucketsdestroy(t->strIndexed);
	avm_tablebucketsdestroy(t->numIndexed);
	free(t);
}




void execute_assign (instruction* instr){
	avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*) 0);
	avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);

	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv == &retval));
	assert(rv); //should do similar assertion tests here
	assert(lv);
	avm_assign(lv, rv);
}



void avm_assign (avm_memcell* lv, avm_memcell* rv){
	if (lv == rv){ /* Same cells? Destructive to assign! */
		return ;
	}

	if (lv->type == table_m &&  /* Same cells? no need to assign */
		rv->type == table_m &&
		lv->data.tableVal == rv->data.tableVal){
		return ;
	}

	if (rv->type == undef_m){	/* From undefined r-value? warn! */
		avm_warning("assigning from 'undef' content!","","");
	}

	avm_memcellclear(lv);		/* Clear old cell contents. */

	memcpy(lv, rv, sizeof(avm_memcell));	/* In C++ dispatch instead. */

	/* Now take care of copied calues or reference counting. */
	if (lv->type == string_m){
		lv->data.strVal = strdup(rv->data.strVal);
	}
	else {
		if (lv->type == table_m){
			avm_tableincrefcounter(lv->data.tableVal);
		}
	}
}
void avm_error(char* format, char* s, char*s1){

	printf("%s\n", format);

}
void avm_warning (char* format, char* ts, char* is){}
void avm_calllibfunc (char* id){
/*	library_func_t f = avm_getlibraryfunc (id);
	if(!f){
		avm_error("unsuported lib func '%s' called!", id,"");
		executionFinished = 1;
	}
	else{
		/* Notice that enter function and exit function
			are called manually!
		*/
//		topsp = top;	/* Enter function sequence. No stack locals. */
//		totalActuals = 0;
//		(*f) ();	/* Call library function. */
//		if(!executionFinished){	/* An error may naturally occur inside. */
//			execute_funcexit((instruction*) 0);	/* Return sequence. */
//		}
//	}
}

char* avm_tostring(avm_memcell* cell){

}

void execute_call (instruction* instr){
	avm_memcell* func = avm_translate_operand(&instr->result, &ax);
	assert(func);
	avm_callsaveenvironment();
	char* s;
	switch (func->type){
		case userfunc_m :
			pc = func->data.funcVal;
			assert(pc < AVM_ENDING_PC);
			assert(code[pc].opcode == funcenter_v);
			break;
		case string_m : avm_calllibfunc(func->data.strVal); break;
		case libfunc_m : avm_calllibfunc(func->data.libfuncVal); break;

		default:
			s = avm_tostring(func);
			avm_error("call: cannot bind '%s' to function!","" ,"");
			free(s);
			executionFinished = 1;
			break;
	}
}

void avm_dec_top (void){
	if (!top) {		/* Stack overflow */
		avm_error("stack overflow","","");
		executionFinished = 1;
	}
	else {
		--top;
	}
}

void avm_push_envvalue (unsigned val){
	stack[top].type = number_m;
	stack[top].data.numVal = val;
	avm_dec_top();
}

void avm_callsaveenvironment (void){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc+1);
	avm_push_envvalue(top + totalActuals + 2);
	avm_push_envvalue(topsp);
}


void execute_funcenter(instruction* instr){
	avm_memcell* func = avm_translate_operand(&instr->result, &ax);
	assert(func);
	assert(pc == func->data.funcVal);	/* Func address should match PC. */

	/* Callee actions here. */
	totalActuals = 0;
	//userfunc* funcInfo = avm_getfuncinfo (pc);
	topsp = top;
	//top = top - funcInfo->localSize;
}


unsigned avm_get_envvalue (unsigned i){
	assert(stack[i].type = number_m);
	unsigned val = (unsigned) stack[i].data.numVal;
	assert(stack[i].data.numVal == ((double) val));
	return val;
}

void execute_funcexit (instruction* unused){
	unsigned oldTop = top;
	top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);

	while(++oldTop <= top){ /* Intentionally ignoring first. */
		avm_memcellclear(&stack[oldTop]);
	}
}






unsigned avm_totalactuals (void){
	return avm_get_envvalue (topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell* avm_getactual(unsigned i){
	assert(i<avm_totalactuals());
	return &stack[topsp + AVM_STACKENV_SIZE +1 + i];
}

void execute_pusharg (instruction* instr){
	avm_memcell* arg = avm_translate_operand(&instr->arg1, &ax);
	assert(arg);

	/* This is actually stack[top] = arg, but we have to use avm_assign. */
	avm_assign(&stack[top], arg);
	++totalActuals;
	avm_dec_top();
}



arithmetic_func_t arithmeticFuncs[] = {
	//add_impl,
	//sub_impl,
	//mul_impl,
	//div_impl,
	//mod_impl
};



void execute_arithmetic (instruction* instr){
	avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*) 0);
	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv == &retval));
	assert(rv1 && rv2);

	if (rv1->type != number_m || rv2->type != number_m){
		avm_error("not a number in arithmetic!","","");
		executionFinished = 1;
	}
	else{
		arithmetic_func_t op = arithmeticFuncs[instr->opcode - add_v];
		avm_memcellclear(lv);
		lv->type = number_m;
		lv->data.numVal = (*op) (rv1->data.numVal, rv2->data.numVal);
	}
}






unsigned char number_tobool (avm_memcell* m) { return m->data.numVal != 0;}
unsigned char string_tobool (avm_memcell* m) { return m->data.strVal[0] != 0;}
unsigned char bool_tobool (avm_memcell* m) { return m->data.boolVal;}
unsigned char table_tobool (avm_memcell* m) { return 1;}
unsigned char userfunc_tobool (avm_memcell* m) { return 1;}
unsigned char libfunc_tobool (avm_memcell* m) { return 1;}
unsigned char nil_tobool (avm_memcell* m) { return 0;}
unsigned char undef_tobool (avm_memcell* m) { assert(0); return 0;}


char* typeStrings[] = {
	"number",
	"string",
	"bool",
	"table",
	"userfunc",
	"libfunc",
	"nil",
	"undef"
};

void execute_jeq (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in equality!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		result = rv1->type == nil_m && rv2->type== nil_m;
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		//result = (avm_tobool(rv1), avm_tobool(rv2));
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}


void execute_jne (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in not equal!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		result = rv1->type == nil_m && rv2->type== nil_m;
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		//result = (avm_tobool(rv1) == avm_tobool(rv2));
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_jle (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in less equal!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		avm_error("'nil' involved in less equal!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		//result = (avm_tobool(rv1) == avm_tobool(rv2));
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_jge (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in greater equal!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		result = rv1->type == nil_m && rv2->type== nil_m;
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		// result = (avm_tobool(rv1) == avm_tobool(rv2));
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_jlt (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in less than!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		avm_error("'nil' involved in less than!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		// result = (avm_tobool(rv1) == avm_tobool(rv2));
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}

void execute_jgt (instruction* instr){
	assert(instr->result.type == label_a);

	avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
	avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);

	unsigned char result = 0;

	if(rv1->type == undef_m || rv2 ->type == undef_m){
		avm_error("'undef' involved in greater than!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == nil_m || rv2->type == nil_m){
		avm_error("'nil' involved in greater than!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type == bool_m || rv2->type == bool_m){
		avm_error("'bool' invloved in greater than!",typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else if(rv1->type != rv2->type){
		avm_error("%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
	}
	else{
		if(rv1->type == number_m){
			if(rv1->data.numVal == rv2->data.numVal){
				result = 1;
			}
		}
		else if(rv1->type == string_m){
			if(strcmp(rv1->data.strVal, rv2->data.strVal) == 0){
				result = 1;
			}
		}
		else if(rv1->type == userfunc_m){
			if(rv1->data.funcVal == rv2->data.funcVal){
				result = 1;
			}
		}
		else if(rv1->type == libfunc_m){
			if(strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal)== 0){
				result = 1;
			}
		}
		else if(rv1->type == table_m){
			if(rv1->data.tableVal == rv2->data.tableVal){
				result = 1;
			}
		}
	}
	if(!executionFinished && result){
		pc = instr->result.val;
	}
}






void execute_tablegetelem (instruction* instr){
	// avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*) 0);
	// avm_memcell* t = avm_translate_operand(&instr->arg1, (avm_memcell*) 0);
	// avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);

	// assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[top] || lv == &retval));
	// assert(t && &stack[AVM_STACKSIZE-1] >= t && t> &stack[top]);

	// avm_memcellclear(lv);
	// lv->type = nil_m; /* Default value. */

	// if(t->type != table_m){
	// 	avm_error("illegal use of type %s as table!", typeStrings[t->type],"");
	// }
	// else {
	// 	//avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);
	// 	if(content){
	// 		avm_assign(lv, content);
	// 	}
	// 	else{
	// 		char* ts = avm_tostring(t);
	// 		char* is = avm_tostring(i);
	// 		avm_warning("%s[%s] not found!", ts, is);
	// 		free(ts);
	// 		free(is);
	// 	}
	// }
}


void execute_tablesetelem (instruction* instr){
	// avm_memcell* t = avm_translate_operand(&instr->result, (avm_memcell*) 0);
	// avm_memcell* i = avm_translate_operand(&instr->arg1, &ax);
	// avm_memcell* c = avm_translate_operand(&instr->arg2, &bx);

	// assert(t && &stack[AVM_STACKSIZE-1] >= t && t> &stack[top]);
	// assert(i && c);

	// if(t->type != table_m){
	// 	avm_error("illegal use of type %s as table!", typeStrings[t->type],"");
	// }
	// else{
	// 	//avm_tablesetelem(t->data.tableVal, i, c);
	// }
}

void execute_uminus(instruction *i){}
void execute_and(instruction *i){}
void execute_or(instruction *i){}
void execute_not(instruction *i){}
//void execute_jne(instruction *i){}
// void execute_jle(instruction *i){}
// void execute_jge(instruction *i){}
// void execute_jlt(instruction *i){}
// void execute_jgt(instruction *i){}
void execute_newtable(instruction *i){}
void execute_nop(instruction *i){}

 execute_func_t executeFuncs[] = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	execute_uminus,
	execute_and,
	execute_or,
	execute_not,
	execute_jeq,
	execute_jne,
	execute_jle,
	execute_jge,
	execute_jlt,
	execute_jgt,
	execute_call,
	execute_pusharg,
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_nop
};
void execute_cycle (void){
	if(executionFinished)
		return;
	else
		if(pc == AVM_ENDING_PC){
			executionFinished = 1;
			return;
		}
		else{
			assert(pc < AVM_ENDING_PC);
			instruction* instr = code + pc;
			assert( instr -> opcode >= 0 && instr -> opcode <= AVM_MAX_INSTRUCTIONS);
			if (instr->srcLine)
				currLine = instr -> srcLine;
			unsigned oldPC = pc;

			(executeFuncs[instr->opcode])(instr);
			execute_arithmetic(instr);

			if (pc == oldPC)
				++pc;
		}
}
void read_instr() {
	int c;
	FILE* file = fopen("instructions.txt", "r"); /* should check the result */
    char line[256];
    char* tok;
    fgets(line, sizeof(line), file);
    tok = strtok(line,"\t");
    printf("%s : ",tok);
    if (strcmp(tok,"magicnumber")==0){
    	tok = strtok(NULL, line);
    	printf("%s", tok);
    }
    fgets(line, sizeof(line), file);
    tok = strtok(line,",");
    if (strcmp(tok,"strings")==0) {
    	while (tok = strtok(NULL,",")){
			conststrs = (char**)malloc((totalconststrs+1) * sizeof(char*));
			conststrs[totalconststrs] = (char*)malloc(sizeof(char)*strlen(tok));
			conststrs[totalconststrs] = strdup(tok);
			totalconststrs += 1;
		}
    }
	fgets(line, sizeof(line), file);
	tok = strtok(line,",");
	if (strcmp(tok,"numbers")==0) {
		while (tok = strtok(NULL,",")) {
			constnumbers = (double*)malloc(sizeof(double)*(totalconstnumbers+1));
			constnumbers[totalconstnumbers] = atof(tok);
			totalconstnumbers+=1;
		}
	}
	fgets(line, sizeof(line), file);
	tok = strtok(line," ");
	if (strcmp(tok,"instructions")==0) {
		int i = 0;
		while (1) {
			codeSize += 1;
			code = malloc(codeSize*sizeof(instruction));
			fgets(line, sizeof(line), file);
			tok = strtok(line," ");
			if (strcmp(tok,"NOP")==0){break;}
			code[i].opcode = atoi(tok);
			tok = strtok(NULL," ");
			code[i].result.type = atoi(tok);
			tok = strtok(NULL," ");
			code[i].result.val = atoi(tok);
			tok = strtok(NULL," ");
			code[i].arg1.type = atoi(tok);
			tok = strtok(NULL," ");
			code[i].arg1.val = atoi(tok);
			tok = strtok(NULL," ");
			code[i].arg2.type = atoi(tok);
			tok = strtok(NULL," ");
			code[i].arg2.val = atoi(tok);
		}
	}
    fclose(file);
}
void main() {
	avm_initialize();
	read_instr();
	while(executionFinished!=0){
		execute_cycle();
	}
}