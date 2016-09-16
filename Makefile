make:
	gcc lexerTest.c -olexer

debug:
	gcc -g lexerTest.c -olexer

clean: 
	rm lexer
	rm -rf lexer.dSYM
