%{
#include <stdlib.h>
#include "global.h"

void yyerror(char *);

int handle_string(const char *text);

%}

%%


"/*"([^*]|\**[^*/])*\**"*/"  	{													 // comments
									char *tmp;
									tmp = yytext;
									// count lines in comments
									while(*tmp != 0) {
										if(*tmp == '\n') line_number++;
										tmp++;
									}
								} 


[0-9]+ 							{ yylval.vInt = atoi(yytext); return NUM; }			// constants

"+"								return '+';											// symbols
"-"								return '-';
"*"								return '*';
"/"								return '/';
"<"								return '<';
"<="							return LE;
">"								return '>';
">="							return GE;
"=="							return EQ;
"!="							return NE;
"="								return '=';
";"								return ';';
","								return ',';
"("								return '(';
")"								return ')';
"["								return '[';
"]"								return ']';
"{"								return '{';
"}"								return '}';


[a-zA-Z][a-zA-Z]* 				return handle_string(yytext);						// keywords and ids


[ \t\r]+        				; // white characters								// other
[\n]							line_number++;
.               				errorLiSSS("Unknown character '", yytext, "'");

%%

// ========= Keywords and Ids =========

typedef struct keyword_entry_struct {
	const char* name;
	int id;
} keyword_entry;

// all available keywords and their ids
keyword_entry keywords[] = {
	{"else", ELSE},
	{"if", IF},
	{"int", INT},
	{"while", WHILE},
	{"print", PRINT}
};
int keywords_count = 5;

// Check if text is keyword or variable name
// - returns the keyword id or the ID id
int handle_string(const char *text) {
	int i;
	
	// check all keywords
	for(i=0; i<keywords_count; i++) {
	
		if(comp(keywords[i].name, text)) {
			// so text is keyword
			return keywords[i].id;
		}
	}
	
	// so text is variable name
	strcpy(yylval.vStr, yytext);
	return ID;
}

// Terminates scanner on end of file (flex)
int yywrap(void) {
    return 1;
}
