#include <stdio.h>

#include "global.h"
#include "y.tab.c"
#include "lex.yy.c"

#include "exec.c"

extern FILE *yyin;
extern node *root;


int handleArguments(int argc, char *argv[]);
int checkFile(FILE *f, const char* filename);


int main(int argc, char *argv[]) {
	
	// handle arguments
	if(handleArguments(argc, argv)) return 0;
	
	// lexical and syntax analysis	
	yyin = fopen(argv[1], "r");	
	if(yyin == NULL) {
		printf("Input file error: Can't open file '%s'\n", argv[1]);
		return 0;
	}	
	yyparse();	
	fclose(yyin);
	
	// print syntax tree
	if(os.tree) printTree(root);
	
	// oprimize the syntax tree
	optimizeTree(root);
	
	// exit on errors
	if(stats.errors > 0 || !stats.parsedall) {
		
		// error recovey failed (for debuging)
		if(!stats.parsedall) printf("\nCould not parse the whole file (error recovey failed)\n");
		
		printf("\nExit message: Can't continue to code generation\n");
		return 0;
	}
		
	// generate and write mixal code
	{
		fout = fopen("asm.mixal", "w");
		
		if(checkFile(fout, "asm.mixal")) return 0;
			
		printMixal(root);
		fclose(fout);
	}
	
	// optimize mixal code
	{
		FILE *in;
		FILE *out;
		
		in = fopen("asm.mixal", "r");
		out = fopen("asm.opt.mixal", "w");
		
		if(checkFile(in, "asm.mixal")) return 0;
		if(checkFile(out, "asm.opt.mixal")) return 0;
		
		optimizeMixal(in, out);
		
		fclose(in);
		fclose(out);
	}
	
	// run assembler
	{	
		char buf[128];
		sprintf(buf, "cp asm.opt.mixal %s", argv[2]); // just copy (no assembler);
		system(buf); 
	}
	
	// delete mixal code files
	if(!os.keepasm) {
		remove("asm.mixal");
		remove("asm.opt.mixal");
	}
	
	return 0;
}

// Handle the arguments
// 
// - returns 1 if program can't continue
int handleArguments(int argc, char *argv[]) {
	int i, arg_error;
	
	// init
	arg_error = 0;
	
	// check if help is requested
	if(argc == 2 && comp(argv[1], "-?")) {
		arg_error = 1;
	}
	
	// check required arguments
	if(argc < 3 && !arg_error) {
		printf("Argument error: Missing argument\n\n");
		arg_error = 1;
	}
	
	// check optional arguments	
	os.keepasm = os.tree = os.overflow = os.nocomments = 0;
	
	for(i=3; i<argc; i++) {
		if(comp(argv[i], "-asm")) os.keepasm = 1;
		else if(comp(argv[i], "-tree")) os.tree = 1;
		else if(comp(argv[i], "-overflow")) os.overflow = 1;
		else if(comp(argv[i], "-nocomments")) os.nocomments = 1;
		else {
			printf("Argument error: '%s' not recognised\n\n", argv[i]);
			arg_error = 1;
		}
	}
	
	// show usage
	if(argc < 3 || arg_error) {
		printf("Usage: %s input_filename output_filename [-asm] [-tree] [-overflow] [-nocomments]\n\n", argv[0]);
		printf("-asm:        Keep the mixal assembly code file.\n");
		printf("-tree:       Print the syntax tree.\n");
		printf("-overflow:   Compiled program: Exit on overflow.\n");
		printf("-nocomments: Compiled program: Hide all comments.\n");
		return 1;
	}
	
	return 0;
}

// Check if file is ready to use
// 
// - returns 1 if program can't continue
int checkFile(FILE *f, const char* filename) {
	
	if(f == NULL) {
		printf("File error: Can't open file '%s'\n", filename);
		return 1;
	}
	
	return  0;
}
