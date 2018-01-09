#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <string.h>

// ========= Global variables =========

FILE *fout;

// the current line of the input file
int line_number = 1;

// all options
struct struct_options {
	char keepasm;			// if mixal assembly code file will be deleted
	char tree;				// if tree will be printed
	char overflow;			// if overflows cause exit
	char nocomments;		// if comments will be printed
} os = {0, 0, 0, 0};

// stats
struct stats_struct {
	char parsedall;			// if the whole file was parsed
	int errors;				// errors detected
	int warnings;			// warnings detected
} stats = {0, 0, 0};

// ========= Utils =========


#ifndef NULL
#define NULL 				0
#endif

#define TRUE 				1
#define FALSE 				0

int comp(const char *a, const char *b) {
	while(*a && *a == *b) a++, b++;
	return *a == *b;
}

/*
int strcpy(char *a, const char *b) {
	while(*b) *a++ = *b++;
	*a = 0;
}
*/

char * strcpyt(char *a, char *b) {
	while(*b && *b!='\t') *a++ = *b++;
	*a = 0;
	b++;
	return b;
}


// ========= Symbol =========

#define TYPE_RAW 			1
#define TYPE_INTEGER 		2

#define WHERE_REGISTER 		1
#define WHERE_MEMORY 		2
#define WHERE_CONSTANT 		3

#define SIZE_SINGLE			-1

typedef struct struct_symbol {

	char *name;					// the name
	char type;					// TYPE_ + INTEGER
	char where;					// WHERE_ + REGISTER, MEMORY, CONSTANT

	int size;					// SIZE_SINGLE or number of cells

	union {
		int location;				// the address
		int value;					// the constant value
	};

	char tmp;					// TRUE, FALSE
	char read_only;				// TRUE, FALSE

} symbol;

// ========= Node =========

#define TYPE_ACT 			1
#define TYPE_SYMBOL 		2

// == javascript code gen == (function(text){var l=text.split(" ");text="";for(var i=0;i<l.length;i+=1){text+="#define A_"+l[i]+" "+(i+2)+"\n"};return text})("PROGRAM DECLARATIONLIST DECLARATION STATEMENTLIST STATEMENT COMPOUNDSTATEMENT LOCALDECLARATIONLIST EXPRESSIONSTATEMENT SELECTIONSTATEMENT ITERATIONSTATEMENT PRINTSTATEMENT EXPRESSION L G LE GE EQ NE ADD SUB MULT DIV VAR")

#define A_PROGRAM 2
#define A_DECLARATIONLIST 3
#define A_DECLARATION 4
#define A_STATEMENTLIST 5
#define A_STATEMENT 6
#define A_COMPOUNDSTATEMENT 7
#define A_LOCALDECLARATIONLIST 8
#define A_EXPRESSIONSTATEMENT 9
#define A_SELECTIONSTATEMENT 10
#define A_ITERATIONSTATEMENT 11
#define A_PRINTSTATEMENT 12
#define A_EXPRESSION 13
#define A_L 14
#define A_G 15
#define A_LE 16
#define A_GE 17
#define A_EQ 18
#define A_NE 19
#define A_ADD 20
#define A_SUB 21
#define A_MULT 22
#define A_DIV 23
#define A_VAR 24

typedef struct struct_node {

	char type;					// TYPE_ + ACT, SYMBOL

	int act;					// A_ + ...
	symbol *symbol;

	int children_count;			// the number of children/operands
	struct struct_node *children[4];

	int line_number;			// line numbder for errors and warnings

} node;


// =======================================
// =======================================
// =======================================


// ====== Errors and warnings (and notes) ======

// error Line String
void errorLiS(const char *s1) {
	stats.errors++;
	printf("[line %3d]   Error: %s\n", line_number, s1);
}
// error Line String String String
void errorLiSSS(const char *s1, const char *s2, const char *s3) {
	stats.errors++;
	printf("[line %3d]   Error: %s%s%s\n", line_number, s1, s2, s3);
}
// error Line String Integer String
void errorLiSIS(const char *s1, int i2, const char *s3) {
	stats.errors++;
	printf("[line %3d]   Error: %s%d%s\n", line_number, s1, i2, s3);
}
// error Node String String String
void errorNoSSS(node *n, const char *s1, const char *s2, const char *s3) {
	stats.errors++;
	printf("[line %3d]   Error: %s%s%s\n", n->line_number, s1, s2, s3);
}

// warning Line String String String
void warningLiSSS(const char *s1, const char *s2, const char *s3) {
	stats.warnings++;
	printf("[line %3d] Warning: %s%s%s\n", line_number, s1, s2, s3);
}
// warning Node String String String
void warningNoSSS(node *n, const char *s1, const char *s2, const char *s3) {
	stats.warnings++;
	printf("[line %3d] Warning: %s%s%s\n", n->line_number, s1, s2, s3);
}

// note String
void noteS(const char *s1) {
	printf("              Note: %s\n", s1);
}

// code generation exit error String
void generrorS(const char *s1) {
	printf("Code generation exit error: %s\n", s1);
}

// ====== Create symbols ======


symbol * create_symbol_const(int value) {
	symbol * s;

	s = (symbol*) malloc(sizeof(symbol));

	s->name = NULL;
	s->type = TYPE_INTEGER;
	s->where = WHERE_CONSTANT;

	if(value > 1073741823) {
		errorLiSIS("Integer value '", value, "' over the limit of 1073741823");
		value = 0;
	}

	s->value = value;

	s->tmp = TRUE;
	s->read_only = TRUE;

	return s;
}

symbol * create_symbol_variable(char* name, int size) {
	symbol * s;

	s = (symbol*) malloc(sizeof(symbol));

	s->name = malloc(81);
	strcpy(s->name, name);//s->name = name;

	s->type = TYPE_INTEGER;
	s->where = WHERE_MEMORY;

	s->size = size;

	s->location = 1; // temp value

	s->tmp = FALSE;
	s->read_only = FALSE;

	return s;
}

symbol * create_symbol_type(char type) {
	symbol * s;

	s = (symbol*) malloc(sizeof(symbol));

	s->name = NULL;
	s->type = type;
	s->where = WHERE_CONSTANT;

	s->value = 0;

	s->tmp = TRUE;
	s->read_only = TRUE;

	return s;
}

symbol * create_symbol_copy(symbol * o) {
	symbol * s;

	s = (symbol*) malloc(sizeof(symbol));

	s->name = NULL;
	s->type = o->type;
	s->where = o->where;
	s->size = o->size;

	if(s->where == WHERE_CONSTANT) s->value = o->value;
	else s->location = o->location;

	s->tmp = TRUE;
	s->read_only = TRUE;

	return s;
}

symbol * create_symbol_tmp_register() {
	symbol * s;

	s = (symbol*) malloc(sizeof(symbol));

	s->name = NULL;
	s->type = TYPE_RAW;
	s->where = WHERE_REGISTER;

	s->size = SIZE_SINGLE;

	s->tmp = TRUE;
	s->read_only = TRUE;

	return s;
}

// ====== Symbol table ======

#define MAX_VARIABLES 128

symbol* symbols_all[MAX_VARIABLES];
int symbols_all_index = 0;

symbol* symbols_active[MAX_VARIABLES];
int symbols_active_index = 0;
char symbols_active_used[MAX_VARIABLES];

void pushSymbol(symbol* s) {
	symbols_all[symbols_all_index++] = s;
	symbols_active[symbols_active_index] = s;
	symbols_active_used[symbols_active_index++] = 0;
}

void pushSymbolSeparator() {
	symbols_active[symbols_active_index++] = 0;
}

void popSymbols() {
	symbols_active_index--;
	while(symbols_active_index>0 && symbols_active[symbols_active_index-1] != 0) {
		symbols_active_index--;

		if(!symbols_active_used[symbols_active_index]) {
			warningLiSSS("Unused variable '", symbols_active[symbols_active_index]->name, "'");
		}
	}
}

symbol* findSymbol(char *name, char isarray) {
	int i;

	for(i=symbols_active_index-1; i>=0; i--) {
		if(symbols_active[i]) {
			if(comp(name, symbols_active[i]->name)) {

				symbols_active_used[i] = 1;
				return symbols_active[i];
			}
		}
	}

	errorLiSSS("Variable '", name, "' not defined");
	noteS("Not defined variables are only reported once");

	// push symbol so error is printed only one time
	pushSymbol(create_symbol_variable(name, (isarray?1:SIZE_SINGLE)));
	symbols_active_used[symbols_active_index-1] = 1;

	return 0;
}

symbol* findSymbolSameScope(char *name) {
	int i;

	for(i=symbols_active_index-1; i>=0; i--) {
		if(!symbols_active[i]) break; // end of scope

		if(comp(name, symbols_active[i]->name)) {

			return symbols_active[i];
		}
	}

	return 0;
}

// ====== Create nodes ======

node * create_node_act(int act, int count, node *c1, node *c2, node *c3, node *c4) {
	node *n;

	//if ((n = malloc(sizeof(node))) == NULL) yyerror("out of memory");
	n = malloc(sizeof(node));

	n->type = TYPE_ACT;
	n->act = act;
	n->symbol = NULL;
	n->children_count = count;

	if(c1) {
		n->line_number = c1->line_number;
	}else {
		n->line_number = line_number;
	}

	n->children[0] = c1;
	n->children[1] = c2;
	n->children[2] = c3;
	n->children[3] = c4;

	return n;
}

node * create_node_act_0(int act) {
	return create_node_act(act, 0, 0, 0, 0, 0);
}
node * create_node_act_1(int act, node * c1) {
	return create_node_act(act, 1, c1, 0, 0, 0);
}
node * create_node_act_2(int act, node * c1, node * c2) {
	return create_node_act(act, 2, c1, c2, 0, 0);
}
node * create_node_act_3(int act, node * c1, node * c2, node * c3) {
	return create_node_act(act, 3, c1, c2, c3, 0);
}
node * create_node_act_4(int act, node * c1, node * c2, node * c3, node * c4) {
	return create_node_act(act, 4, c1, c2, c3, c4);
}

node * create_node_symbol(symbol *sym) {
	node *n;

	//if ((n = malloc(sizeof(node))) == NULL) yyerror("out of memory");
	n = malloc(sizeof(node));

	n->type = TYPE_SYMBOL;
	n->act = 0;
	n->symbol = sym;
	n->children_count = 0;

	n->line_number = line_number;

	n->children[0] = 0;
	n->children[1] = 0;
	n->children[2] = 0;
	n->children[3] = 0;

	return n;
}

node * create_node_const(int value) {

	return create_node_symbol(create_symbol_const(value));
}

node * create_node_new_variable(char* name) {
	symbol* s;

	// check for re-definition
	s = findSymbolSameScope(name);
	if(s) {
		errorLiSSS("Variable '", name, "' has already been defined in this scope");
		return create_node_symbol(s);
	}

	s = create_symbol_variable(name, SIZE_SINGLE);
	pushSymbol(s);

	return create_node_symbol(s);
}
node * create_node_new_variable_array(char* name, int size) {
	symbol* s;

	// check for re-definition
	s = findSymbolSameScope(name);
	if(s) {
		errorLiSSS("Variable '", name, "' has already been defined in this scope");
		return create_node_symbol(s);
	}

	s = create_symbol_variable(name, size);
	pushSymbol(s);

	return create_node_symbol(s);
}

void yyerror(char *);


node * create_node_variable(char* name) {
	symbol* s;

	s = findSymbol(name, 0);

	if(s && s->size != SIZE_SINGLE) errorLiSSS("Variable '", name, "' is an array, index must be given");

	return create_node_symbol(s);
}
node * create_node_variable_array(char* name) {
	symbol* s;

	s = findSymbol(name, 1);

	if(s && s->size == SIZE_SINGLE) errorLiSSS("Variable '", name, "' is not an array, index can't be given");

	return create_node_symbol(s);
}


node * create_node_type(char type) {

	return create_node_symbol(create_symbol_type(type));
}


// ========= Delete from memory =========

void delete_symbol(symbol *s) {
	free(s);
}

void delete_node(node *n) {

	if(n->symbol) {
		if(n->symbol->tmp) delete_symbol(n->symbol);
	}

	free(n);
}



#endif
