make: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc symbol.c tree.c parser.tab.c lex.yy.c -oparser
debug: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc -g symbol.c tree.c parser.tab.c lex.yy.c -oparser -DDEBUG
clean:
	rm lex.yy.c
	rm parser.tab.*
	rm parser
