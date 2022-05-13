%{
#include <stdio.h>
#include "parser.h"
#include <string.h>
struct alpha_token_t {
  unsigned int     firstline;	
  unsigned int     numline;
  unsigned int     numToken;
  char          *content;
  char          *type;
  struct alpha_token_t *alpha_yylex;
};
struct alpha_token_t* curtoken;
struct alpha_token_t* headtoken;
struct alpha_token_t* lasttoken;
int tokens = 0;
int headflag = 1;
int firstline;
int linestarts [12];
int nestedcomments = 0;
char* str2;int str2counter;
void add_token(int line, char* content, char* type);
void add_multiline_token(int line, char* content, char* type, int firstline);
%}

id					[a-zA-Z][a-zA-Z_0-9_]*
integer				[0-9]*
real				([0-9]+[.][0-9]*|[0-9]*[.][0-9]+)
space				[\r\n \t\v]

keyword1  			"if"
keyword2 			"else"
keyword3 			"while"
keyword4 			"for"
keyword5 			"function"
keyword6 			"return" 
keyword7			"break"
keyword8			"continue"
keyword9			"and"
keyword10			"not"
keyword11			"or"
keyword12			"local"
keyword13			"true"
keyword14			"false"
keyword15			"nil"

op1					"="
op2					"+"
op3					"-"
op4					"*"
op5					"/"
op6					"%"
op7					"=="
op8					"!="
op9					"++"
op10				"--"
op11				">"
op12				"<"
op13				">="
op14				"<="

pun1				"{"
pun2 				"}"
pun3 				"["
pun4				"]"
pun5				"("
pun6				")"
pun7				";"
pun8				","
pun9				":"
pun10				"::"
pun11				"."
pun12				".."

%option yylineno
%option noyywrap

%x COMMENT

%x LINE_COMMENT

%%
{keyword1}			{return IF;}
{keyword2}			{return ELSE;}
{keyword3}			{return WHILE;}
{keyword4}			{return FOR;}
{keyword5}			{return FUNCTION;}
{keyword6}			{return RETURN;}
{keyword7}			{return BREAK;}
{keyword8}			{return CONTINUE;}
{keyword9}			{return AND;}
{keyword10}			{return NOT;}
{keyword11}			{return OR;}
{keyword12}			{return LOCAL;}
{keyword13}			{return TRUE;}
{keyword14}			{return FALSE;}
{keyword15}			{return NIL;}

{id}				{
						yylval.stringValue = strdup(yytext);
						return ID;
					}

{integer}			{
						yylval.intValue = atoi(yytext);
						return INTCONST;
					}
{real}				{
						yylval.realValue = atof(yytext);
						return REALCONST;}
{space}				{}



{op1}				{return yytext[0];}
{op2}				{return yytext[0];}
{op3}				{return yytext[0];}
{op4}				{return yytext[0];}
{op5}				{return yytext[0];}
{op6}				{return yytext[0];}
{op7}				{return EQUAL;}
{op8}				{return NOT_EQUAL;}
{op9}				{return INCREMENT;}
{op10}				{return DECREMENT;}
{op11}				{return yytext[0];}
{op12}				{return yytext[0];}
{op13}				{return GREATER_EQUAL;}
{op14}				{return LESS_EQUAL;}


{pun1}				{return yytext[0];}
{pun2}				{return yytext[0];}
{pun3}				{return yytext[0];}
{pun4}				{return yytext[0];}
{pun5}				{return yytext[0];}
{pun6}				{return yytext[0];}
{pun7}				{return yytext[0];}
{pun8}				{return yytext[0];}
{pun9}				{return yytext[0];}
{pun10}				{return DOUBLE_COLON;}
{pun11}				{return yytext[0];}
{pun12}				{return DOUBLE_DOT;}

"//" {BEGIN(LINE_COMMENT);}
<LINE_COMMENT>[^\n]*
<LINE_COMMENT><<EOF>>	{BEGIN(INITIAL);}
<LINE_COMMENT>"\n" {BEGIN(INITIAL); return LINE_COM;}

"\"" {str2counter = 0;str2=(char*)malloc(5001);firstline=yylineno;int c;
while ((c = input()) != EOF) {
	str2counter++;
	if ((char)c == '\\'){
		if ((c = input())!= EOF) {
			*str2 = '\\';
			str2counter++;
			str2++;
			*str2 = (char)c;
			printf("CHAR : %c\n",c);
		}
		else {printf("ERROR : ENCOUNTERED END OF FILE BEFORE THE END OF THE STRING!\n");return 0;}
	}
	else if((char)c == '\"'){
		*str2= '\0';
		str2-= (str2counter-1);
		yylval.stringValue = strdup(str2);
		free(str2);
		return STRING;
		break;
	}
	else {*str2 = (char)c;}
	str2++;
}
if (c == EOF) {printf("ERROR : ENCOUNTERED END OF FILE BEFORE THE END OF THE STRING!\n");return 0;}
}

"/*"	{BEGIN(COMMENT);firstline=yylineno;}
<COMMENT>"/*"	{linestarts[nestedcomments] = yylineno;nestedcomments++;}
<COMMENT>[^*/]*
<COMMENT><<EOF>>	{printf("ERROR : ENCOUNTERED END OF FILE BEFORE THE END OF THE COMMENT!\n");return 0;}
<COMMENT>"*/"	{BEGIN(INITIAL);}


%%

void add_multiline_token(int line, char* type, char* content, int firstline) {
	struct alpha_token_t* newtoken = (struct alpha_token_t*)malloc(sizeof(struct alpha_token_t));
	tokens += 1;
	newtoken -> firstline = firstline;
	newtoken -> numline = line;
	newtoken -> numToken = tokens;
	if (strcmp("STRING", type)==0){
		newtoken -> content = (char*)malloc(5001);
	}
	newtoken -> content = strdup(content);
	newtoken -> type = (char*)malloc(25);
	newtoken -> type = type;
	newtoken -> alpha_yylex = NULL;
	if (headtoken == NULL) {
		headtoken = newtoken;
	}
	if (curtoken != NULL) {
		curtoken -> alpha_yylex = newtoken;
	}
	curtoken = newtoken;
}

void add_token(int line, char* type, char* content){
	struct alpha_token_t* newtoken = (struct alpha_token_t*)malloc(sizeof(struct alpha_token_t));
	tokens += 1;
	newtoken -> numline = line;
	newtoken -> numToken = tokens;
	newtoken -> content = (char*)malloc(500);
	newtoken -> content = content;
	newtoken -> type = (char*)malloc(35);
	newtoken -> type = type;
	newtoken -> alpha_yylex = NULL;
	if (headtoken == NULL) {
		headtoken = newtoken;
	}
	if (curtoken != NULL) {
		curtoken -> alpha_yylex = newtoken;
	}
	curtoken = newtoken;
}
