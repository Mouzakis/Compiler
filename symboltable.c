#include "quads.h"

int unk = 0, total = 0;
Symtab *Hash[500];

int hashfunc(char *key){
	unsigned int i,n;
	for(i=0;key[i]!='\0';i++){
		n=n+(int)key[i];
	}
	n=n%500;
	return n;
}

void init(void){
	int i;
	for (i = 0; i < 500; i++) {
		Hash[i] = NULL;
	}
	Insert(0,"libfunc" ,"print", 0 , 0);
	Insert(0,"libfunc", "input", 0 , 0);
	Insert(0,"libfunc", "objectmemberkeys", 0 , 0);
	Insert(0,"libfunc", "objecttotalmembers", 0 , 0);
	Insert(0,"libfunc", "objectcopy", 0 , 0);
	Insert(0,"libfunc", "totalarguments", 0 , 0);
	Insert(0,"libfunc", "argument", 0 , 0);
	Insert(0,"libfunc", "typeof", 0 , 0);
	Insert(0,"libfunc", "strtonum", 0 , 0);
	Insert(0,"libfunc", "sqrt", 0 , 0);
	Insert(0,"libfunc", "cos", 0 , 0);
	Insert(0,"libfunc", "sin", 0 , 0);
}

void Insert(int var, char *type, char *name, unsigned int scope, unsigned int line){
	if (total < scope) {total = scope;}
	int key = hashfunc(name);
	Symtab *head=Hash[key];

	Symtab *newNode=(Symtab*)malloc(sizeof(Symtab));
	newNode->isActive=1;
	newNode->type = (char*)malloc((strlen(type)+1));
	newNode->type = type;
	newNode->next=NULL;
	if(var==1){
		Variable *newV=malloc(sizeof(Variable));
		newV ->name = malloc((strlen(name)+1));
		newV->name = name;
		newV->line=line;
		newV->scope=scope;
		newNode->value.varVal=newV;

	}
	else{
		Function *newF=malloc(sizeof(Function));
		newF ->name = malloc((strlen(name)+1));
		newF->name = name;
		newF->line=line;
		newF->scope=scope;
		newNode->value.funcVal=newF;
	}

	if(head==NULL){
		Hash[key]=newNode;
	}
	else{
		while(head->next !=NULL){
			head=head->next;
		}
		head->next=newNode;
	}
}
Symtab * Insert2(int var, char *type, char *name, unsigned int scope, unsigned int line){
	if (total < scope) {total = scope;}
	int key = hashfunc(name);
	Symtab *head=Hash[key];

	Symtab *newNode=(Symtab*)malloc(sizeof(Symtab));
	newNode->isActive=1;
	newNode->type = (char*)malloc((strlen(type)+1));
	newNode->type = type;
	newNode->next=NULL;
	if(var==1){
		Variable *newV=malloc(sizeof(Variable));
		newV ->name = malloc((strlen(name)+1));
		newV->name = name;
		newV->line=line;
		newV->scope=scope;
		newNode->value.varVal=newV;

	}
	else{
		Function *newF=malloc(sizeof(Function));
		newF ->name = malloc((strlen(name)+1));
		newF->name = name;
		newF->line=line;
		newF->scope=scope;
		newNode->value.funcVal=newF;
	}

	if(head==NULL){
		Hash[key]=newNode;
	}
	else{
		while(head->next !=NULL){
			head=head->next;
		}
		head->next=newNode;
	}
	return newNode;
}


Symtab * LookUp(char *name, unsigned int scope, char *type){
	int i;
	int key = hashfunc(name);
	Symtab *tmp = Hash[key];
	int libfunc =0;
	Symtab* duplicate = NULL;
	int vardupli = -2;
	Symtab *tmp2 = Hash[key];
	/*if (strcmp(name, "print")==0) {
		while(tmp2->next!=NULL){
			tmp2 = tmp2->next;
			if (tmp2->type,"libfunc")
		}
		return tmp;
	}
	else if (strcmp(name, "input")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"input")!=0){
			tmp = tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "objectmemberkeys")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"objectmemberkeys")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "objecttotalmembers")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"objecttotalmembers")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "objectcopy")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"objectcopy")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "totalarguments")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"totalarguments")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "argument")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"argument")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "typeof")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"typeof")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "strtonum")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"strtonum")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "sqrt")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"sqrt")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "cos")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"cos")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}
	else if (strcmp(name, "sin")==0) {
		while(strcmp(tmp->type,"libfunc")!=0 && strcmp(tmp->value.funcVal->name,"sin")!=0){
			tmp=tmp->next;
		}
		return tmp;
	}*/
	for(i=0;i<500;i++){
		tmp=Hash[i];
		while(tmp!=NULL){
			if(strcmp(tmp->type, "userfunc")==0 || (strcmp(tmp->type, "libfunc")==0)) {
				if ((strcmp(type, "local")!=0) && (strcmp(type, "global")!=0)) {
					if(strcmp(tmp->value.funcVal->name, name)==0 && tmp->isActive==1 && tmp->value.funcVal->scope==scope){
						if (tmp -> value.funcVal -> scope == scope) {return tmp;}
					}
				}
				else {
					if(strcmp(tmp->value.funcVal->name, name)==0 && tmp->isActive==1 && tmp->value.funcVal->scope<=scope) {
						if (strcmp(type, "local")==0) {
							if (duplicate!=NULL) {
								if (duplicate->value.varVal != NULL) {
									if ((tmp -> value.varVal -> scope) > duplicate->value.varVal->scope){
										duplicate = tmp;
									}
								}
								else {
									if ((tmp -> value.varVal -> scope) > duplicate->value.funcVal->scope){
										duplicate = tmp;
									}
								}
							}
							else {
								duplicate = tmp;
							}
						}
						else if (strcmp(type, "global")==0) {
							if (tmp->value.funcVal->scope == 0) {
								duplicate = tmp;
							}
						}
						else {
							return tmp;
						}
					}
				}
			}
			else{
				if(strcmp(tmp -> value.varVal -> name , name)==0 && tmp -> isActive == 1 && tmp -> value.varVal -> scope <= scope){
					if (duplicate != NULL) {
						if (duplicate->value.varVal != NULL) {
							if (strcmp(type, "global")==0) {
								if(tmp -> value.varVal -> scope == 0) {
									duplicate = tmp;
								}
							}
							else if ((tmp -> value.varVal -> scope) > duplicate->value.varVal->scope){
								duplicate = tmp;
							}
						}
						else {
							if (strcmp(type, "global")==0) {
								if(tmp -> value.varVal -> scope == 0) {
									duplicate = tmp;
								}
							}
							else if ((tmp -> value.varVal -> scope) > duplicate->value.funcVal->scope){
								duplicate = tmp;
							}
						}
					}
					else {
						duplicate = tmp;
					}
				}
			}
			tmp=tmp->next;
		}
	}
	return duplicate;
}

void Hide(int scope){
	Symtab *tmp;
	int i;
	for(i=0;i<500;i++){
		tmp=Hash[i];
		while(tmp!=NULL){
			if(tmp->value.varVal != NULL){
				if(tmp->value.varVal->scope==scope) {
					tmp->isActive=0;
				}
			}
			else{
				if(tmp->value.funcVal->scope==scope) {
					tmp->isActive=0;
				}
			}
			tmp=tmp->next;
		}
	}
}

void Print(void) {
	int sum = 0;
	int scope = 0;
	int scopeflag = 0, i;
	Symtab * trailer;
	do {
		for(i=0;i<500;i++){
			trailer=Hash[i];
			while(trailer!=NULL){
				if ((strcmp(trailer->type, "userfunc")!= 0 && strcmp(trailer->type, "libfunc")!= 0) ) {
				if(trailer->value.varVal->scope==scope) {
				sum++;
				if (scopeflag==0){
					printf("\n--------------- Scope #%d ---------------\n", scope);
					scopeflag=1;
				}
				printf("\"%s\" [%s variable] [line %d] [scope %d] \n", trailer -> value.varVal -> name, trailer -> type, trailer -> value.varVal -> line, trailer -> value.varVal -> scope);
				}
				}
				else if (trailer->value.funcVal->scope==scope){
					sum++;
					if(scopeflag==0){printf("\n--------------- Scope #%d ---------------\n", scope);scopeflag=1;}
					printf("\"%s\" [%s function] [line %d] [scope %d]\n", trailer -> value.funcVal -> name, trailer -> type, trailer -> value.funcVal -> line, trailer -> value.funcVal -> scope);
				}
				trailer = trailer -> next;
			}
		}
		scope++;
		scopeflag=0;
	}while(scope <= total);
}

char *generate_unknown(void) {
	char *func = malloc(sizeof(char)*4);
	sprintf(func,"_f%d",unk);
	unk++;
	return func;
}
