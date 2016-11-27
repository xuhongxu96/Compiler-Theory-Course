make: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc trans.c symbol.c tree.c parser.tab.c lex.yy.c -oparser 
debug: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc -g trans.c symbol.c tree.c parser.tab.c lex.yy.c -oparser -DDEBUG -fsanitize=leak
clean:
	rm lex.yy.c
	rm parser.tab.*
	rm parser
