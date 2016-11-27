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
    struct SymNode *parent;
    struct SymNode *type;
    char name[255];
    int size;
    int lineno;
    struct SymNode *next;
    bool param;
};

struct FunNode {
    char name[255];
    struct SymNode *ret;
    int size;
    struct SymNode *param;
    struct FunNode *next;
    int lineno;
};

struct ExpType {
    bool isLeftVal;
    struct SymNode *leftVal;
    struct SymNode *prop;
};


extern struct SymNode *propTable, *varTable;
extern struct FunNode *funcTable;
struct FloatNode {
    float v;
    struct FloatNode *n;
};
extern struct FloatNode *flconst, *fltail;

bool istype(struct ast *t, const char *type);
void initSymTable();
struct SymNode *createSymNode(enum SymType symtype, struct SymNode *type, const char *name, struct SymNode *next, int lineno, struct SymNode *parent);
struct SymNode *lookupSym(struct SymNode *n, const char *s);
void semantic(struct ast *t);
void traceSymbol(struct SymNode *, int, bool, bool);
void printSymbol(struct SymNode *);
struct ExpType initExpType(struct SymNode *leftVal, struct SymNode *prop);
struct SymNode *lookupStruct(struct SymNode *p, const char *f);
struct FunNode *lookupFunc(const char *s);
