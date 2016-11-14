#pragma once
#include "tree.h"
#include <stdbool.h>

enum SymType {
    S_INT, S_FLOAT, S_STRUCT,
    S_VAR, S_ARRAY,
    S_STRUCT_FIELD
};

struct SymNode {
    enum SymType symtype;
    struct SymNode *type;
    char name[255];
    int size;
    int lineno;
    struct SymNode *next;
};


void initSymTable();
struct SymNode *lookupSym(struct SymNode *n, const char *s);
void semantic(struct ast *t);
void traceSymbol(struct SymNode *, int, bool, bool);
void printSymbol(struct SymNode *);
