#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "parser.tab.h"


extern int yylineno;
void yyerror(const char *);
void yyerror2(const char *type, const char *msg);

struct SymNode *propTable = NULL, *varTable = NULL;
char stype[6][10] = {"int", "float", "struct", "var", "array", "field"};

struct SymNode *createSymNode(enum SymType symtype, struct SymNode *type, const char *name, struct SymNode *next, int lineno) {
    struct SymNode *temp;
    if (temp = lookupSym(propTable, name)) {
        fprintf(stderr, "Error Type 3 at line %d: The type '%s' has been already defined at line %d.\n", lineno,  name, temp->lineno);
        return NULL;
    }
    if (temp = lookupSym(varTable, name)) {
        fprintf(stderr, "Error Type 3 at line %d: The variable '%s' has been already defined at line %d.\n", lineno,  name, temp->lineno);
        return NULL;
    }
    struct SymNode *n = (struct SymNode *) malloc(sizeof(struct SymNode));
    n->symtype = symtype;
    n->type = type;
    if (name) {
        strcpy(n->name, name);
    }
    n->size = 0;
    n->next = next;
    n->lineno = lineno;
    return n;
}

void initSymTable() {
    propTable = createSymNode(S_FLOAT, NULL, "float", NULL, 0);
    propTable = createSymNode(S_INT, NULL, "int", propTable, 0);
}

struct SymNode *lookupSym(struct SymNode *n, const char *s) {
    struct SymNode *p = n;
    while (p != NULL) {
        if (strcmp(p->name, s) == 0) {
            return p;
        }
        p = p->next;
    }
}

void addTableItem(struct SymNode **table, struct SymNode *n) {
    if (!n) return;
    struct SymNode *p = n;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = *table;
    *table = n;
}

void addProp(struct SymNode *n) {
    if (!n) return;
#ifdef DEBUG
    printf("== New Prop: %s\n", n->name);
#endif
    addTableItem(&propTable, n);
}

void addVar(struct SymNode *n) {
    if (!n) return;
#ifdef DEBUG
    printf("== New Var: %s\n", n->name);
#endif
    addTableItem(&varTable, n);
}

bool istype(struct ast *t, const char *type) {
    return strcmp(t->val.text, type) == 0;
}


struct SymNode *sym_specifier(struct ast *t);
struct SymNode *sym_var_dec(struct SymNode *type, struct ast *t);

struct SymNode *sym_dec(struct SymNode *type, struct ast *t) {
    if (istype(t, "Dec")) {
        if (t->size == 1) {
            // VarDec
            return sym_var_dec(type, t->childs[0]);
        } else if (t->size == 2) {
            // VarDec ASSIGNOP Exp
            // TODO: check assignment
            return sym_var_dec(type, t->childs[0]);
        }
    }
}

struct SymNode *sym_dec_list(struct SymNode *type, struct ast *t) {
    if (istype(t, "DecList")) {
        if (t->size == 1) {
            // Dec
            return sym_dec(type, t->childs[0]);
        } else if (t->size == 2) {
            // Dec COMMA DecList
            struct SymNode *n = sym_dec_list(type, t->childs[1]);
            n->next = sym_dec(type, t->childs[0]);
            return n;
        }
    }
    return NULL;
}

struct SymNode *sym_def(struct ast *t) {
    if (istype(t, "Def")) {
        // Specifier DecList SEMI
        struct SymNode *type = sym_specifier(t->childs[0]);
        struct SymNode *n = sym_dec_list(type, t->childs[1]);
        return n;
    }
    return NULL;
}

struct SymNode *sym_def_list(struct ast *t) {
    if (istype(t, "DefList")) {
        if (t->size == 2) {
            // Def DefList
            struct SymNode *n = sym_def(t->childs[0]);
            struct SymNode *n2 = sym_def_list(t->childs[1]);
            if (n2) {
                addTableItem(&n, n2);
            }
            return n;
        }
    }
    return NULL;
}

struct SymNode *to_struct_field(struct SymNode *t) {
    struct SymNode *p = t;
    while (p) {
        p->symtype = S_STRUCT_FIELD;
        p = p->next;
    }
    return t;
}

struct SymNode *sym_struct_specifier(struct ast *t) {
    if (istype(t, "StructSpecifier")) {
        if (t->size == 1) {
            // STRUCT Tag
            struct SymNode *n = lookupSym(propTable, t->childs[0]->childs[0]->val.text);
            if (n->symtype != S_STRUCT) {
                return NULL;
            }
            return n;
        } else if (t->size == 2) {
            // STRUCT OptTag LC DefList RC
            struct SymNode *n = sym_def_list(t->childs[1]);
            struct SymNode *ret = NULL;
            to_struct_field(n);
            addProp(ret = createSymNode(S_STRUCT, n, t->childs[0]->childs[0]->val.text, NULL, t->childs[0]->childs[0]->lineno));
            return ret;
        }
    }
    return NULL;
}

struct SymNode *sym_specifier(struct ast *t) {
    if (istype(t, "Specifier Type")) {
        // TYPE
        return lookupSym(propTable, t->childs[0]->val.text);
    } else if (istype(t, "Specifier Struct")) {
        // StructSpecifier
        return sym_struct_specifier(t->childs[0]);
    }
    return NULL;
}

struct SymNode *sym_var_dec(struct SymNode *type, struct ast *t) {
    if (istype(t, "VarDec")) {
        if (t->size == 1) {
            // ID
            return createSymNode(S_VAR, type, t->childs[0]->val.text, NULL, t->childs[0]->lineno);
        } else if (t->size == 2) {
            // VarDec LB INT RB
            struct SymNode *n = sym_var_dec(type, t->childs[0]);
            struct SymNode *ret = createSymNode(S_ARRAY, n, t->childs[0]->val.text, NULL, t->childs[0]->lineno);
            ret->size = t->childs[1]->val.n;
            return ret;
        }
    }
    return NULL;
}

void sym_ext_dec_list(struct SymNode *type, struct ast *t) {
    if (istype(t, "ExtDecList")) {
        if (t->size == 1) {
            // VarDec
            addVar(sym_var_dec(type, t->childs[0]));
        } else if (t->size == 2) {
            // VarDec COMMA ExtDecList
            addVar(sym_var_dec(type, t->childs[0]));
            sym_ext_dec_list(type, t->childs[1]);
        }
    }
}

bool sym_ExtDef(struct ast *t) {
    if (istype(t, "ExtDef")) {
        if (t->size == 2) {
            // Specifier ExtDecList SEMI
            struct SymNode *type = sym_specifier(t->childs[0]);
            sym_ext_dec_list(type, t->childs[1]);
        } else if (t->size == 1) {
            // Specifier SEMI
            sym_specifier(t->childs[0]);
        } else if (t->size == 3) {
            // Specifier FunDec CompSt
        }
        return true;
    }
    return false;
}

void printSymbol(struct SymNode *t) {
    if (t == NULL) return;
    printf("pointer:\t%ld\nsymtype:\t%s\nname:\t\t%s\nsize:\t\t%d\nnext:\t\t%ld\ntype:\t\t%ld\n", (long) t,  stype[t->symtype], t->name, t->size, (long) t->next, (long) t->type);
}

void traceSymbol(struct SymNode *t, int level, bool next, bool type) {
    if (t == NULL) return;
    for (int i = 0; i < level; ++i) printf(">>");
    printf("\n");
    printSymbol(t);
    if (type || level == 0) {
        if (t->symtype == S_STRUCT) {
            traceSymbol(t->type, level + 1, true, type);
        } else {
            traceSymbol(t->type, level + 1, false, type);
        }
    }
    if (level == 0) {
        printf("\n---------\n");
    }
    if (next) {
        struct SymNode *p = t;
        while (p != NULL) {
            p = p->next;
            traceSymbol(p, level, false, type);
        }
    }
}

void _semantic(struct ast *t) {
    if (t == NULL) return;

    if (!sym_ExtDef(t)) {
        for(int i = 0; i < t->size; ++i) {
            _semantic(t->childs[i]);
        }
    }

}

void semantic(struct ast *t) {
    _semantic(t);

    printf("\n\n=== Prop ===\n\n");
    traceSymbol(propTable, 0, true, true);
    printf("\n\n=== Var ===\n\n");
    traceSymbol(varTable, 0, true, false);

}
