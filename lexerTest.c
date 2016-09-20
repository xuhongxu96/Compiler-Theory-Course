#include <stdio.h>
#include "lexer.c"

int main(int argc, char *argv[]) {
    FILE *fp;
    fp = fopen(argv[1], "r");
    struct Lexer *lexer = createLexer(fp);
    lex(lexer);
    return 0;
}
