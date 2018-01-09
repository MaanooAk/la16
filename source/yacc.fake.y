%{
#include <stdio.h>
#include <stdlib.h>
#include "global.h"

extern int line_number;

int yylex(void);
void yyerror(char *s);

node * root;
%}

%union {
	int vInt;
	char vStr[81];

	node *vNode;
};


%token <vInt> NUM
%token <vStr> ID

%nonassoc BEFORE_ELSE
%nonassoc ELSE IF INT WHILE PRINT

%token '=' ';' '(' ')' '[' ']' '{' '}'

%left EQ NE
%left '>' GE '<' LE
%left '+' '-'
%left '*' '/'


%type <vNode> nProgram
%type <vNode> nDeclarationList nDeclaration nType
%type <vNode> nStatementList nStatement
%type <vNode> nExpressionStatement nCompoundStatement nSelectionStatement nIterationStatement nPrintStatement
%type <vNode> nLocalDeclarationList nExpression nVar nSimpleExpression nExpressionLevel1 nExpressionLevel2 nFactor
%type <vNode> nDeclarationListWrap nLocalDeclarationListWrap

%type <vStr> nErrorOperator

%start nProgram

%%

nProgram:
                  nDeclarationListWrap nStatementList            { root = create_node_act_2(A_PROGRAM, $1, $2); popSymbols(); stats.parsedall = 1; }
                ;

nDeclarationListWrap:
                  nDeclarationList                               { $$ = $1; pushSymbolSeparator(); }
                ;

nDeclarationList:
                  nDeclarationList nDeclaration                  { $$ = create_node_act_2(A_DECLARATIONLIST, $1 ,$2); }
                | nDeclaration                                   { $$ = create_node_act_1(A_DECLARATIONLIST, $1); }
                // errors
                | error ';'                                      { $$ = 0; errorLiS("At least one variable must be declared"); yyerrok; }
                ;

nDeclaration:
                  nType ID ';'                                    { $$ = create_node_act_2(A_DECLARATION, $1, create_node_new_variable($2)); }
                | nType ID '[' NUM ']' ';'                        { $$ = create_node_act_2(A_DECLARATION, $1, create_node_new_variable_array($2, $4)); }
                // fake errors
                | nType ID '[' '-' NUM ']' ';'                    { $$ = 0; errorLiS("Array size can't be negative."); }
                // errors
                | nType ID error ';'                              { $$ = 0; errorLiSSS("Mising ';' after declaration of '",$2,"'"); yyerrok; }
                | nType ID '[' NUM ']' error ';'                  { $$ = 0; errorLiSSS("Mising ';' after declaration of '",$2,"'"); yyerrok; }
                | nType ID '[' error ';'                          { $$ = 0; errorLiSSS("Expected size of array '",$2,"'"); yyerrok; }
                ;

nType:
                  INT                                            { $$ = create_node_type(TYPE_INTEGER); }
                ;

nStatementList:
                  nStatementList nStatement                       { $$ = create_node_act_2(A_STATEMENTLIST, $1 ,$2); }
                |                                                 { $$ = create_node_act_0(A_STATEMENTLIST); }
                ;

nStatement:
                  nExpressionStatement                           { $$ = create_node_act_1(A_STATEMENT, $1); }
                | nCompoundStatement                             { $$ = create_node_act_1(A_STATEMENT, $1); }
                | nSelectionStatement                            { $$ = create_node_act_1(A_STATEMENT, $1); }
                | nIterationStatement                            { $$ = create_node_act_1(A_STATEMENT, $1); }
                | nPrintStatement                                { $$ = create_node_act_1(A_STATEMENT, $1); }
                // errors
                ;

nCompoundStatement:
                  '{' nLocalDeclarationListWrap nStatementList '}'     { $$ = create_node_act_2(A_COMPOUNDSTATEMENT, $2, $3); popSymbols(); }
                // erros
                | '{' nLocalDeclarationListWrap nStatementList error   { $$ = 0; errorLiS("Mising closing '}'"); }
                ;

nLocalDeclarationListWrap:
                  nLocalDeclarationList                            { $$ = $1; pushSymbolSeparator(); }
                ;

nLocalDeclarationList:
                  nLocalDeclarationList nDeclaration            { $$ = create_node_act_2(A_LOCALDECLARATIONLIST, $1 ,$2); }
                |                                               { $$ = create_node_act_0(A_LOCALDECLARATIONLIST); }
                ;

nExpressionStatement:
                  nExpression ';'                                { $$ = create_node_act_1(A_EXPRESSIONSTATEMENT, $1); }
                | ';'                                            { $$ = create_node_act_0(A_EXPRESSIONSTATEMENT); }
                // errors
                | nExpression error    ';'                            { $$ = 0; errorLiSSS("Mising ';' after expression", "", ""); yyerrok; }
                ;

nSelectionStatement:
                  IF '(' nExpression ')' nStatement    %prec BEFORE_ELSE   { $$ = create_node_act_2(A_SELECTIONSTATEMENT, $3, $5); }
                | IF '(' nExpression ')' nStatement ELSE nStatement        { $$ = create_node_act_3(A_SELECTIONSTATEMENT, $3, $5, $7); }
                // errors
                | IF '(' nExpression ')' error ';'                         { $$ = 0; errorLiSSS("Expected expression of '", "if", "' statement"); }
                | IF '(' nExpression ')' nStatement ELSE error ';'         { $$ = 0; errorLiSSS("Expected expression of '", "else", "' statement"); }
                | IF error nExpression    ')'                              { $$ = 0; errorLiSSS("Expected '(' after '", "if", "'"); }
                | IF '(' nExpression error nStatement %prec BEFORE_ELSE    { $$ = 0; errorLiSSS("Expected ')' after '", "if", "' condition"); }
                | IF '(' nExpression error ELSE nStatement                 { $$ = 0; errorLiSSS("Expected ')' after '", "if", "' condition"); }
                ;

nIterationStatement:
                  WHILE '(' nExpression ')' nStatement           { $$ = create_node_act_2(A_ITERATIONSTATEMENT, $3, $5); }
                // errors
                | WHILE '(' nExpression ')' error ';'            { $$ = 0; errorLiSSS("Expected expression of '", "while", "' statement"); }
                | WHILE error nExpression ')'                    { $$ = 0; errorLiSSS("Expected '(' after '", "while", "'"); }
                | WHILE '(' nExpression error nStatement         { $$ = 0; errorLiSSS("Expected ')' after '", "while", "' condition"); }
                ;

nPrintStatement:
                  PRINT nExpression ';'                          { $$ = create_node_act_1(A_PRINTSTATEMENT, $2); }
                // errors
                | PRINT error ';'                                { $$ = 0; errorLiSSS("Expected expression of '", "print", "' statement");  yyerrok; }
                ;

nExpression:
                  nVar '=' nExpression                           { $$ = create_node_act_2(A_EXPRESSION, $1, $3); }
                | nSimpleExpression                              { $$ = $1; }
                // errors
                ;

nVar:
                  ID                                             { $$ = create_node_act_1(A_VAR, create_node_variable($1)); }
                | ID '[' nExpression ']'                         { $$ = create_node_act_2(A_VAR, create_node_variable_array($1), $3); }
                // errors
                | ID '[' nExpression error ';'                   { $$ = 0; errorLiS("Mising closing ']'"); yyerrok;  yyerrok; }
                ;

nSimpleExpression:
                  nExpressionLevel1 '<' nExpressionLevel1        { $$ = create_node_act_2(A_L, $1, $3); }
                | nExpressionLevel1 '>' nExpressionLevel1        { $$ = create_node_act_2(A_G, $1, $3); }
                | nExpressionLevel1 GE nExpressionLevel1         { $$ = create_node_act_2(A_GE, $1, $3); }
                | nExpressionLevel1 LE nExpressionLevel1         { $$ = create_node_act_2(A_LE, $1, $3); }
                | nExpressionLevel1 NE nExpressionLevel1         { $$ = create_node_act_2(A_NE, $1, $3); }
                | nExpressionLevel1 EQ nExpressionLevel1         { $$ = create_node_act_2(A_EQ, $1, $3); }
                | nExpressionLevel1                              { $$ = $1; }
                // errors
                | nExpressionLevel1 '<' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", "<", "'"); yyerrok; }
                | nExpressionLevel1 '>' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", ">", "'"); yyerrok; }
                | nExpressionLevel1 GE error ';'                 { $$ = 0; errorLiSSS("Missing 2nd operand of '", ">=", "'"); yyerrok; }
                | nExpressionLevel1 LE error ';'                 { $$ = 0; errorLiSSS("Missing 2nd operand of '", "<=", "'"); yyerrok; }
                | nExpressionLevel1 NE error ';'                 { $$ = 0; errorLiSSS("Missing 2nd operand of '", "!=", "'"); yyerrok; }
                | nExpressionLevel1 EQ error ';'                 { $$ = 0; errorLiSSS("Missing 2nd operand of '", "==", "'"); yyerrok; }
                | nExpressionLevel1 '+' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", "+", "'"); yyerrok; }
                | nExpressionLevel1 '-' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", "-", "'"); yyerrok; }
                ;

nExpressionLevel1:
                  nExpressionLevel1 '+' nExpressionLevel2        { $$ = create_node_act_2(A_ADD, $1, $3); }
                | nExpressionLevel1 '-' nExpressionLevel2        { $$ = create_node_act_2(A_SUB, $1, $3); }
                | nExpressionLevel2                              { $$ = $1; }
                // errors
                ;

nExpressionLevel2:
                  nExpressionLevel2 '*' nFactor                  { $$ = create_node_act_2(A_MULT, $1, $3); }
                | nExpressionLevel2 '/' nFactor                  { $$ = create_node_act_2(A_DIV, $1, $3); }
                | nFactor                                        { $$ = $1; }
                // errors
                | nExpressionLevel2 '*' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", "*", "'"); yyerrok; }
                | nExpressionLevel2 '/' error ';'                { $$ = 0; errorLiSSS("Missing 2nd operand of '", "/", "'"); yyerrok; }
                ;

nFactor:
                  NUM                                         { $$ = create_node_const($1); }
                | nVar                                        { $$ = $1; }
                | '(' nExpression ')'                         { $$ = $2; }
                // errors
                | '(' nExpression error ';'                   { $$ = 0; errorLiS("Mising closing ')' of expression"); yyerrok; }
                | error nErrorOperator                        { $$ = 0; errorLiSSS("Missing 1st operand before '",$2,"' operator"); yyerrok; }
                ;


nErrorOperator:
                  EQ                                        { strcpy($$, "=="); }
                | NE                                        { strcpy($$, "!="); }
                | '>'                                       { strcpy($$, ">"); }
                | GE                                        { strcpy($$, ">="); }
                | '<'                                       { strcpy($$, "<"); }
                | LE                                        { strcpy($$, "<="); }
                | '+'                                       { strcpy($$, "+"); }
                | '-'                                       { strcpy($$, "-"); }
                | '*'                                       { strcpy($$, "*"); }
                | '/'                                       { strcpy($$, "/"); }
                ;

%%

void yyerror(char *s) {
	stats.errors+=1;
	//printf("yyerror #%d Error [line %d]: %s\n", stats.errors, line_number, s);
}

