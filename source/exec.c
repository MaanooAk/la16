#include <stdio.h>

// ========= Print the tree =========


// Prints an array of spaces (given number x2)
void printInside(int count) {
	while(count-- > 0) printf("  ");
}

// == javascript code gen == (function(text){var l=text.split(" ");text="{\"\", \"\", ";for(var i=0;i<l.length;i+=1){text+="\""+l[i]+"\", ";};return text+"}"})("PROGRAM DECLARATIONLIST DECLARATION STATEMENTLIST STATEMENT COMPOUNDSTATEMENT LOCALDECLARATIONLIST EXPRESSIONSTATEMENT SELECTIONSTATEMENT ITERATIONSTATEMENT PRINTSTATEMENT EXPRESSION L G LE GE EQ NE ADD SUB MULT DIV VAR") 

char *A_num[] = {"", "", "PROGRAM", "DECLARATIONLIST", "DECLARATION", "STATEMENTLIST", "STATEMENT", "COMPOUNDSTATEMENT", "LOCALDECLARATIONLIST", "EXPRESSIONSTATEMENT", "SELECTIONSTATEMENT", "ITERATIONSTATEMENT", "PRINTSTATEMENT", "EXPRESSION", "L", "G", "LE", "GE", "EQ", "NE", "ADD", "SUB", "MULT", "DIV", "VAR"};

// Print the syntax tree from the given node downwards
void subPrintTree(node * n, int depth) {
	
	
	if(!n) {
		printInside(depth);
		printf("- Nothing\n");
		return;
	}
	
	if(n->type == TYPE_ACT) {
		int i;
		
		printInside(depth);
		printf("+ %s\n", A_num[n->act]);
		
		for(i=0; i<n->children_count; i+=1) {
			subPrintTree(n->children[i], depth+1);
		}
		
	}else if(n->type == TYPE_SYMBOL) {
		
		printInside(depth);
		if(n->symbol->where == WHERE_MEMORY) {
			printf("- %s\n", n->symbol->name);
		}else{
			printf("- %d\n", n->symbol->value);
		}
	}
	
}

// Print the syntax tree from the given node downwards
// (must be called only with the root)
// 
// - node *n: the root PROGRAM node
void printTree(node * n) {
	printf("Tree:\n");
	subPrintTree(n, 0);
}


// ========= Optimize the tree ==========


// Optimize the syntax tree from the given node downwards
void optimizeTree(node *n) {
	
	if (!n) return;
		
	if(n->type == TYPE_SYMBOL) {
		// nothing
	}
		
	if(n->type == TYPE_ACT) {
		int i;
		
		// optimize all children
		for(i=0; i<n->children_count; i+=1) {
			optimizeTree(n->children[i]);
		}
		
		switch(n->act) {
		case A_L:
		case A_G:
		case A_LE:
		case A_GE:
		case A_EQ:
		case A_NE:
		case A_ADD:
		case A_SUB:
		case A_MULT:
		case A_DIV:
			
			// if both operands are constants do the operation here
			
			if(n->children[0]->type == TYPE_SYMBOL && n->children[0]->symbol->where == WHERE_CONSTANT &&
				n->children[1]->type == TYPE_SYMBOL && n->children[1]->symbol->where == WHERE_CONSTANT) {
				int v1, v2;
				
				v1 = n->children[0]->symbol->value;
				v2 = n->children[1]->symbol->value;
				
				switch(n->act) {
				case A_L: v1=v1<v2?1:0; break;			// true value may not be 1, so we force 1 by the inline if 
				case A_G: v1=v1>v2?1:0; break;
				case A_LE: v1=v1<=v2?1:0; break;
				case A_GE: v1=v1>=v2?1:0; break;
				case A_EQ: v1=v1==v2?1:0; break;
				case A_NE: v1=v1!=v2?1:0; break;
				case A_ADD: v1=v1+v2; break;
				case A_SUB: v1=v1-v2; break;
				case A_MULT: v1=v1*v2; break;
				case A_DIV: 
					if(v2 == 0) {
						errorNoSSS(n, "Division by 0", "", "");
					}else{
						v1=v1/v2; 
					}
					break;
				}
				
				n->type = TYPE_SYMBOL;
				n->symbol = create_symbol_const(v1);
			}
			
			break;
		case A_SELECTIONSTATEMENT:
		case A_ITERATIONSTATEMENT:
			
			// if condition is constant show warning 
			
			if(n->children[0]->type == TYPE_SYMBOL && n->children[0]->symbol->where == WHERE_CONSTANT) {
				int v;
				
				v = n->children[0]->symbol->value;
				
				if(n->act == A_ITERATIONSTATEMENT && v == 1) {					
					// no warning for while(1)
					
				}else{					
					warningNoSSS(n, "Condition is constant, always ", (v?"true":"false"), "");				
				}
				
				
			}
			
			break;
		case A_VAR:
		
			if(n->children_count == 2) {
				// check for negative index
				
				if(n->children[1]->type == TYPE_SYMBOL && n->children[1]->symbol->where == WHERE_CONSTANT && n->children[1]->symbol->value < 0) {
					
					warningNoSSS(n, "Negative array index will cause runtime error", "", "");
					
				}
				
			}
			
			break;
		}
		
	}
	
	
}


// ========= Generate MIXAL  =========


// print Comment
void printCo(const char* text) {
	if(!os.nocomments) fprintf(fout, "*\t\t\t\t\t%s\n", text);
}

// print Label
void printLa(int label) {
	fprintf(fout, "L%d\tNOP\t\n", label); 
}

// print Label Operation Number
void printLaOpNu(const char* label, const char* operation, int num) {
	fprintf(fout, "%s\t%s\t%d\n", label, operation, num);
}
// print Label Operation Operand
void printLaOpOp(const char* label, const char* operation, const char* operand) {
	fprintf(fout, "%s\t%s\t%s\n", label, operation, operand);
}

// print Variable Operation Number
void printVaOpNu(int index, const char* operation, int num) {
	fprintf(fout, "V%d\t%s\t%d\n", index, operation, num);
}

// print Operation Operand
void printOpOp(const char* operation, const char* operand) {
	fprintf(fout, "\t%s\t%s\n", operation, operand);
}
// print Operation Number
void printOpNu(const char* operation, int num) {
	fprintf(fout, "\t%s\t%d\n", operation, num);
}
// print Operation Operand Number
void printOpOpNu(const char* operation, const char* operand, int num) {
	fprintf(fout, "\t%s\t%s%d\n", operation, operand, num);
}
// print Operation Variable
void printOpVa(const char* operation, int index) {
	fprintf(fout, "\t%s\tV%d\n", operation, index);
}
// print Operation Array
void printOpAr(const char* operation, int index) {
	fprintf(fout, "\t%s\tV%d,1\n", operation, index);
}
// print Operation Label
void printOpLa(const char* operation, int index) {
	fprintf(fout, "\t%s\tL%d\n", operation, index);
}
// print Operation Literal
void printOpLi(const char* operation, int value) {
	fprintf(fout, "\t%s\t=%d=\n", operation, value);
}


int next_index;			// the next variable index
int next_location; 		// the next variable location
int next_label;			// the next label index
int next_buf; 			// the next buffer location index

int gen(node *n);

// Generates and prints mixal code on the fout stream
// (must be called only with the root)
// 
// - node *n: the root PROGRAM node
// - int print_comments: 1 if comments will be printed
void printMixal(node *n) {
	
	next_index = 1;
	next_location = 2000;
	next_label = 1;
	next_buf = 0;
	
	gen(root);
}

// decleration of subfunction of the gen() 
void gen_toA(symbol *s);
void gen_notA(symbol *s);
void gen_operation(symbol *s, const char* opMemory, const char* opConst);
void gen_toA_bynotA(symbol *s);
void gen_to1_bynotA(symbol *s);
void gen_to1(symbol *s);
void gen_checkindex();
void gen_checkoverflow(int where);

// Generates code for a given node all of its children
// 
// node *n: the node
int gen(node *n) {

    if (!n) return 0;
		
	if(n->type == TYPE_SYMBOL) {
		// nothing
	}
		
	if(n->type == TYPE_ACT) {
		int size, i;
		int tmp1, tmp2, tmp3, tmp4;
		
		switch(n->act) {
		case A_PROGRAM:
						
			// variables for printing
			printLaOpNu("BUF1","CON",0);
			printLaOpNu("BUF2","CON",0);
			printLaOpNu("BUF3","CON",0);
			// variables for main functionality
			printLaOpNu("TMP","EQU",3000);
			printLaOpNu("BUF","EQU",3001);
			
			// symbols for global variables
			printCo("global variables");
			gen(n->children[0]);
			
			// define the start
			printCo("start");
			printLaOpNu("","ORIG",100); // 100 > 75 = max OUT device blocking size
			printLaOpOp("START","NOP","");
			
			// all statements
			printCo("statments");
			gen(n->children[1]);
			
			// define the end
			printCo("end");
			printLaOpNu("","ENTA",0); // when no erros, set rA to 0 
			printLaOpOp("","HLT","");
			printLaOpNu("ERR","ENTA",1); // when erros stopped execution, set rA to 1
			printLaOpOp("","HLT","");
			printLaOpOp("","END","START");
			
			break;
		
		case A_DECLARATIONLIST:
		case A_LOCALDECLARATIONLIST:
		case A_STATEMENTLIST:
		case A_STATEMENT:
		case A_EXPRESSIONSTATEMENT:
			
			// continue ex in all children
			for(i=0; i<n->children_count; i++) {
				gen(n->children[i]);
			}
						
			break;	
		case A_DECLARATION:
						
			switch(n->children[0]->symbol->type) {
			case TYPE_INTEGER:
				
				// create symbol with format Vi (Variable i) pointing to a free memory address
				
				n->children[1]->symbol->type = TYPE_INTEGER;
				n->children[1]->symbol->location = next_index;
				
				size = n->children[1]->symbol->size;
				
				printVaOpNu(next_index,"EQU",next_location);
				
				// if variable is an array dont allow the next cells to be used by others
				
				next_index++;
				if(size == SIZE_SINGLE) {
					next_location += 1;
				}else{
					next_location += size;
				}
				
				if(next_location > 3000) {
					generrorS("Memory overflow");
				}
								
				break;
			}
			
			break;
		case A_COMPOUNDSTATEMENT:
			
			printCo("compound start");
			tmp1 = next_location;
			//tmp2 = next_index;
			
			gen(n->children[0]); // local declarations
			gen(n->children[1]); // statements
			
			// allow new variables to take the same locations 
			// because the compound has ended
			
			next_location = tmp1;			
			//next_index = tmp2; // cant redefine symbol
			printCo("compound end");
			
			break;
		case A_EXPRESSION:
			
			if(n->children_count == 2) {
				// assigment of variable
								
				if(n->children[0]->children_count == 1) {
					// variable
					
					gen(n->children[1]);
					gen_toA(n->children[1]->symbol);
					
					gen(n->children[0]);
					
					printOpVa("STA",n->children[0]->symbol->location);
					
					n->symbol = n->children[0]->symbol;
					
				}else{
					// variable array cell					
					
					//* calculate index first
					
					gen(n->children[0]->children[1]); // calculate the index 
					gen_notA(n->children[0]->children[1]->symbol); // r1 is used for array cell index
					
					gen(n->children[1]); // calculate the value					
					gen_toA(n->children[1]->symbol);
					
					gen_to1_bynotA(n->children[0]->children[1]->symbol);
					gen_checkindex(n->children[0]->children[1]->symbol);
					printOpAr("STA",n->children[0]->children[0]->symbol->location);
					
					//*/
					
					/* calculate value first
					
					gen(n->children[1]); // calculate the value
					gen_notA(n->children[1]->symbol);
					
					gen(n->children[0]->children[1]); // calculate the index 
					gen_to1(n->children[0]->children[1]->symbol); // r1 is used for array cell index
					
					gen_toA_bynotA(n->children[1]->symbol);
					printOpAr("STA",n->children[0]->children[0]->symbol->location);
					
					//*/
					
					n->symbol = create_symbol_tmp_register();
				}
				
			}else{
				// just an expresion
				
				gen(n->children[0]);
				n->symbol = n->children[0]->symbol;
			}
			
			break;
		case A_VAR:
			
			if(n->children_count == 1) {
				// variable
				
				n->symbol = n->children[0]->symbol;
				
			}else{				
				// variable array
				
				gen(n->children[1]); // calculate the index
				gen_to1(n->children[1]->symbol); // r1 is used for array cell index
				gen_checkindex(n->children[1]->symbol);
				
				printOpAr("LDA",n->children[0]->symbol->location);
				
				n->symbol = create_symbol_tmp_register();
			}
			
			break;
		case A_SELECTIONSTATEMENT:
			
			if(n->children_count == 2) {
				// if then
				
				tmp1 = next_label++; // end label
				
				gen(n->children[0]);
				gen_toA(n->children[0]->symbol);
				
				printOpLa("JAZ",tmp1); // if zero goto exit
				
				gen(n->children[1]);
				
				printLa(tmp1);
							
			}else{
				// if then else
				
				tmp1 = next_label++; // else label
				tmp2 = next_label++; // end label
				
				gen(n->children[0]);
				gen_toA(n->children[0]->symbol);
				
				printOpLa("JAZ",tmp1); // if zero goto second
				
				gen(n->children[1]);
				
				printOpLa("JMP",tmp2); // goto exit
				
				printLa(tmp1);
				
				gen(n->children[2]);
				
				printLa(tmp2);
				
			}
			
			break;
		case A_ITERATIONSTATEMENT:
			// while
			
			tmp1 = next_label++; // start label
			tmp2 = next_label++; // end label
			
			printLa(tmp1);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			printOpLa("JAZ",tmp2); // if zero goto exit
			
			gen(n->children[1]);
			
			printOpLa("JMP",tmp1); // goto start
			
			printLa(tmp2);
						
			break;
		case A_PRINTSTATEMENT:
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			printCo("print start");
			
			printOpOp("ENTX","44"); // set sign 44 (+)
			printOpOp("JANN","*+2"); // check if negative
			printOpOp("ENTX","45"); // set sign 45 (-)
			// rA has the value and rX has the sign
			printOpOp("JBUS", "*(18)"); // wait for device to be ready
			printOpOp("STX","BUF1");
			printOpOp("CHAR",""); // convert to printable
			printOpOp("STA", "BUF2"); // store printable to BUF2 and BUF3
			printOpOp("STX", "BUF3");
			printOpOp("OUT", "BUF1(18)"); // print words starting from BUF1 to device
			
			printCo("print end");
			
			break;
		case A_L:
		case A_G:
		case A_LE:
		case A_GE:
		case A_EQ:
		case A_NE:
			// all comparations
			
			gen(n->children[1]);			
			gen_notA(n->children[1]->symbol);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			gen_operation(n->children[1]->symbol, "CMPA", 0);
			
			printOpOp("ENTA", "1");
			
			// choose jump command 
			switch(n->act) {
				case A_L: printOpOp("JL", "*+2"); break;
				case A_G: printOpOp("JG", "*+2"); break;
				case A_LE: printOpOp("JLE", "*+2"); break;
				case A_GE: printOpOp("JGE", "*+2"); break;
				case A_EQ: printOpOp("JE", "*+2"); break;
				case A_NE: printOpOp("JNE", "*+2"); break;
			}
			
			printOpOp("ENTA", "0");
			
			n->symbol = create_symbol_tmp_register();	
			
			break;
		case A_ADD:
			// a + b
			
			gen(n->children[1]);
			gen_notA(n->children[1]->symbol);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			gen_operation(n->children[1]->symbol, "ADD", "INCA");
			
			n->symbol = create_symbol_tmp_register();			
			
			gen_checkoverflow(0);
			
			break;
		case A_SUB:
			// a - b
			
			gen(n->children[1]);
			gen_notA(n->children[1]->symbol);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			gen_operation(n->children[1]->symbol, "SUB", "DECA");
			
			n->symbol = create_symbol_tmp_register();			
				
			gen_checkoverflow(0);
			
			break;
		case A_MULT:
			// a * b
			
			gen(n->children[1]);
			gen_notA(n->children[1]->symbol);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			gen_operation(n->children[1]->symbol, "MUL", 0);
			
			gen_checkoverflow(1);
			
			// get only the less significant word
			printOpOp("STX","TMP");
			printOpOp("LDA","TMP");
			
			n->symbol = create_symbol_tmp_register();
			
			
			break;
		case A_DIV:
			// a / b
			
			gen(n->children[1]);
			gen_notA(n->children[1]->symbol);
			
			gen(n->children[0]);
			gen_toA(n->children[0]->symbol);
			
			// convert word to double word
			printOpOp("STA","TMP");
			printOpOp("LDX","TMP");
			printOpOp("ENTA","0");
			
			gen_operation(n->children[1]->symbol, "DIV", 0);
			
			gen_checkoverflow(0);
			
			n->symbol = create_symbol_tmp_register();
			
			break;			
		default:			
			fprintf(fout, "*UNKNOWN\n");		
		}
		
	}
	
    return 0;
}

// ========== Subfunction of the gen() function =========

// Symbol in rA.
// Moves a given symbol to rA.
// 
// - symbol *s: the symbol
void gen_toA(symbol *s) {
	
	if(s->where == WHERE_MEMORY) {
		
		if(s->size == SIZE_SINGLE) {
			printOpVa("LDA",s->location);
		}else{
			printOpAr("LDA",s->location);  // never used, can be removed
		}
		
	}else if(s->where == WHERE_CONSTANT) {
	
		if(s->value <= 4095) {
			printOpNu("ENTA",s->value);	
		}else if(s->value <= 999999999) {
			printOpLi("LDA",s->value);	
		}else {
			// cant complile literal >9 characters
			printOpLi("LDA",999999999);
			printOpLi("ADD",s->value-999999999);
		}
		
	}else{
		// ok
	}
	
}

// Symbol not in rA.
// Moves elseswhere content related to a given symbol from rA so it can be written over, also moves 
// elsewhere content of the rA if the the given symbol is a an array cell.
// To restore the state, must be followed by one of: gen_operation(), gen_toA_bynotA(), gen_to1_bynotA()
//
// - symbol *s: the symbol
void gen_notA(symbol *s) {
	
	if(s->where == WHERE_REGISTER) {
	
		if(next_buf) {
			printOpOpNu("STA","BUF+",next_buf);
		}else{
			printOpOp("STA","BUF");
		}		
		next_buf++;
		
		if(next_buf > 999) {
			generrorS("Memory overflow");
		}
		
	}
	
}

// Performs a basic operation with rA the given symbol using the operation given
//
// - symbol *s: the symbol
// - char* opMemory: the operation
// char* opConst: the operation in case the symbol is constant, 0 if not supported
void gen_operation(symbol *s, const char* opMemory, const char* opConst) {
	
	if(s->where == WHERE_MEMORY) {
		
		if(s->size == SIZE_SINGLE) {
			printOpVa(opMemory,s->location);
		}else{
			printOpAr(opMemory,s->location); // never used, can be removed
		}
		
	}else if(s->where == WHERE_CONSTANT) {
		
		if(s->value <= 4095) {
			
			if(opConst) {
				printOpNu(opConst,s->value);
			}else{
				printOpLi(opMemory,s->value);
			}
			
		}else if(s->value <= 999999999) {
			printOpLi(opMemory,s->value);	
		}else {
			// cant complile literal >9 characters
			printOpOp("STA","TMP");
			printOpLi("LDA",999999999);
			printOpLi("ADD",s->value-999999999);
			printOpNu("STA",next_location);
			printOpOp("LDA","TMP");
			printOpNu(opMemory,next_location);	
		}
		
				
	}else{
		
		next_buf--;		
		if(next_buf) {
			printOpOpNu(opMemory,"BUF+",next_buf);
		}else{
			printOpOp(opMemory,"BUF");
		}
	}
	
}

// Symbol in rA.
// Moves a given symbol to rA and restores the changes from notA()
// 
// - symbol *s: the symbol
void gen_toA_bynotA(symbol *s) {
	
	if(s->where == WHERE_MEMORY) {
		
		if(s->size == SIZE_SINGLE) {
			printOpVa("LDA",s->location);
		}else{
			printOpAr("LDA",s->location); // never used, can be removed
		}
			
	}else if(s->where == WHERE_CONSTANT) {
		
		gen_toA(s);
				
	}else{
		
		next_buf--;		
		if(next_buf) {
			printOpOpNu("LDA","BUF+",next_buf);
		}else{
			printOpOp("LDA","BUF");
		}
	}
	
}
// Symbol in r1.
// Moves a given symbol to r1 and restores the changes from notA()
// 
// - symbol *s: the symbol
void gen_to1_bynotA(symbol *s) {
	
	if(s->where == WHERE_MEMORY) {
		
		if(s->size == SIZE_SINGLE) {
			printOpVa("LD1",s->location);
		}else{
			printOpAr("LD1",s->location); // never used, can be removed
		}
			
	}else if(s->where == WHERE_CONSTANT) {
		printOpNu("ENT1",s->value);				
	}else{
		
		next_buf--;		
		if(next_buf) {
			printOpOpNu("LD1","BUF+",next_buf);
		}else{
			printOpOp("LD1","BUF");
		}
	}
	
}

// Symbol in r1.
// Moves a given symbol to r1.
// 
// - symbol *s: the symbol
void gen_to1(symbol *s) {
	
	if(s->where == WHERE_MEMORY) {
	
		if(s->size == SIZE_SINGLE) {
			printOpVa("LD1",s->location);
		}else{
			printOpAr("LD1",s->location); // never used, can be removed
		}
		
	}else if(s->where == WHERE_CONSTANT) {
		printOpNu("ENT1",s->value);
	}else{
		printOpOp("STA","TMP");
		printOpOp("LD1","TMP");
	}
	
}

// Checks a given symbol used as an index if its negative. In this case terminates the execution.
// (symbol must be constant or at r1)
// 
// -symbol *s: the given symbol
void gen_checkindex(symbol *s) {
	
	if(s->where == WHERE_CONSTANT) {
		
		if(s->value < 0) printOpOp("JMP","ERR");
		
	}else{		
		printOpOp("J1N","ERR");
	}

}

// Checks if the overflow has occured. In this case terminates the execution.
// 
// int where: 0 for overflow flag, 1 for rA non zero value
void gen_checkoverflow(int where) {
	if(!os.overflow) return;
	
	switch(where) {
	case 0:	
		printOpOp("JOV","ERR");
		break;
	case 1:
		printOpOp("JANZ","ERR");
	}
	
}


// ========= Optimize the tree ==========

typedef struct struct_mline {
	
	char p1[8];
	char p2[6];
	char p3[15];
	
	struct struct_mline *prev;
	struct struct_mline *next;
} mline;

mline * createMLine(mline *prev) {
	mline *ml;
	
	ml = (mline*) malloc(sizeof(mline));
	
	// link the two mline
	ml->prev = prev;
	prev->next = ml;
	
	ml->next = 0;
	
	return ml;
}

void deleteMLine(mline *ml) {
	
	if(ml->prev) ml->prev->next = ml->next;
	if(ml->next) ml->next->prev = ml->prev;
		
	free(ml);
	
}

// Optimizes the mixal code
// 
// FILE *in: input file
// FILE *out: ouput file
void optimizeMixal(FILE *in, FILE *out) {
	mline head;
	mline *ml;
	char line[60];
	char *linec;
	
	head.next = 0;
	head.prev = 0;
	
	ml = &head;
	
	while(fgets(line, 60, in)) {
		
		if(line[0] == '*') continue; // comment
		
		ml = createMLine(ml);
		
		linec = line;
		
		linec = strcpyt(ml->p1, linec);
		linec = strcpyt(ml->p2, linec);
		strcpyt(ml->p3, linec);
		
		// cool way
		//strcpyt(ml->p3, strcpyt(ml->p2, strcpyt(ml->p1, linec)));
	}
	
	ml = head.next;
	
	while(ml) {
		
		if(ml->prev) {

			// ...
			// ?1? NOP		...
			//>    ?2?	=>	?1? ?2?
			// ...			...
			if(comp(ml->prev->p2, "NOP") && !comp(ml->prev->p1, "") && comp(ml->p1, "")) {
				
				strcpy(ml->p1, ml->prev->p1);
				
				deleteMLine(ml->prev);			
			}
			
			// ...
			//     STA ?1?		...
			//>    LDA ?1?	=>	STA ?1?
			// ...				...
			if(comp(ml->prev->p2, "STA") && comp(ml->p2, "LDA") && comp(ml->prev->p3, ml->p3) && comp(ml->prev->p1, "") && comp(ml->p1, "")) {
				
				ml = ml->prev;
				deleteMLine(ml->next);			
			}
			
		}
		
		ml = ml->next;
	}
	
	ml = head.next;
	
	while(ml) {
		
		fprintf(out, "%s\t%s\t%s", ml->p1, ml->p2, ml->p3);
		
		ml = ml->next;
	}
	
}
