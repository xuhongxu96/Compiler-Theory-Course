make: lex.l parser.y
	bison -d parser.y
	flex lex.l
	gcc parser.tab.c lex.yy.c -oparser
clean:
	rm lex.yy.c
	rm parser.tab.*
	rm parser
