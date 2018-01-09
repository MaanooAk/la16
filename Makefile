
.phony: build
build: compiler

compiler:
	cd source; yacc -d yacc.fake.y
	# y.tab.h
	# y.tab.c

	cd source; flex lex.fake.l
	# lex.yy.c

	cd source; gcc main.c -o ../compiler
	# compiler.exe

	rm -f source/y.tab.h
	rm -f source/y.tab.c
	rm -f source/lex.yy.c


.phony: test
test: compiler
	./compiler samples/1.txt 1.mixal -asm -overflow

.phony: clean
clean:
	rm -f compiler
	rm -f *.mixal
	rm -f source/y.tab.h
	rm -f source/y.tab.c
	rm -f source/lex.yy.c


