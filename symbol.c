#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"
#include "parser.tab.h"


extern int yylineno;
void yyerror(const char *);
void yyerror2(const char *type, const char *msg);

struct SymNode *propTable = NULL, *varTable = NULL;
struct FunNode *funcTable = NULL;
char stype[][10] = {"int", "float", "struct", "var", "array", "field"};

struct SymNode *createSymNode(enum SymType symtype, struct SymNode *type, const char *name, struct SymNode *next, int lineno, struct SymNode *parent) {
    struct SymNode *temp;
    int lerrno = 3;
    if (symtype == S_STRUCT) lerrno = 16;
    if (temp = lookupSym(propTable, name)) {
        fprintf(stderr, "Error Type %d at line %d: The struct '%s' has been already defined at line %d.\n", lerrno, lineno,  name, temp->lineno);
        return NULL;
    }
    if (temp = lookupSym(varTable, name)) {
        fprintf(stderr, "Error Type %d at line %d: The variable '%s' has been already defined at line %d.\n", lerrno, lineno,  name, temp->lineno);
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
    n->param = false;
    n->parent = parent;
    return n;
}

void initSymTable() {
    propTable = createSymNode(S_FLOAT, NULL, "float", NULL, 0, NULL);
    propTable = createSymNode(S_INT, NULL, "int", propTable, 0, NULL);
}

struct FunNode *lookupFunc(const char *s) {
    struct FunNode *p = funcTable;
    while (p != NULL) {
        if (strcmp(p->name, s) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

struct SymNode *lookupSym(struct SymNode *n, const char *s) {
    struct SymNode *p = n;
    while (p != NULL) {
        if (strcmp(p->name, s) == 0) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void addFunc(struct FunNode *f) {
    if (!f) return;
    f->next = funcTable;
    funcTable = f;
}

struct SymNode *addTableItem(struct SymNode **table, struct SymNode *n) {
    if (!n) return *table;
    struct SymNode *p = n;
    while (p->next != NULL) {
        p = p->next;
    }
    p->next = *table;
    return *table = n;
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
void sym_comp_st(struct ast *t);

struct ExpType initExpType(struct SymNode *leftVal, struct SymNode *prop) {
    struct ExpType type;
    type.isLeftVal = (leftVal != NULL);
    type.leftVal = leftVal;
    type.prop = prop;
    return type;
}

struct ExpType sym_exp(struct ast *t) {
    if (istype(t, "Exp Assign")) {
        // Exp ASSIGNOP Exp
        struct ExpType type = sym_exp(t->childs[0]);
        if (!type.isLeftVal) {
            fprintf(stderr, "Error Type 6 at line %d: Assigned to a right value.", t->lineno);
            return initExpType(NULL, NULL);
        }
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type.prop != type2.prop) {
            fprintf(stderr, "Error Type 5 at line %d: Unmatched type '%s' and '%s'.", t->lineno, type.prop->name, type2.prop->name);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp And") 
            || istype(t, "Exp Or")) {
        // Exp AND/OR Exp
        struct ExpType type = sym_exp(t->childs[0]);
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type.prop->symtype != S_INT || type2.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: Boolean operator can only be applied to int values.", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp Relop")
            || istype(t, "Exp Plus")
            || istype(t, "Exp Minus")
            || istype(t, "Exp Star")
            || istype(t, "Exp Div")) {
        // Exp xxx Exp
        struct ExpType type = sym_exp(t->childs[0]);
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type.prop != type2.prop || (type.prop->symtype != S_INT && type.prop->symtype != S_FLOAT)) {
            fprintf(stderr, "Error Type 7 at line %d: Arithmetical operator can only be applied to int or float values.", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp")) {
        // LP Exp RP
        return sym_exp(t->childs[0]);
    } else if (istype(t, "Exp UMinus")) {
        // MINUS Exp
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT && type.prop->symtype != S_FLOAT) {
            fprintf(stderr, "Error Type 7 at line %d: Arithmetical operator can only be applied to int or float values.", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp Not")) {
        // NOT Exp
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: Boolean operator can only be applied to int values.", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp Call")) {
        if (t->size == 2) {
            // ID LP Args RP
            struct FunNode *f = lookupFunc(t->childs[0]->val.text);
            // TODO: sym_args()
        } else if (t->size == 1) {
            // ID LP RP
            struct FunNode *f = lookupFunc(t->childs[0]->val.text);
            if (!f) {
                fprintf(stderr, "Error Type 2 at line %d: Function undefined.", t->lineno);
                return initExpType(NULL, NULL);
            }
            if (f->size != 0) {
                fprintf(stderr, "Error Type 9 at line %d: Function needs %d params.", t->lineno, f->size);
                return initExpType(NULL, NULL);
            }
            return initExpType(NULL, f->ret);
        }
        return initExpType(NULL, NULL);
    } else if (istype(t, "Exp Array")) {
        // Exp LB Exp RB
        // TODO: Array
    } else if (istype(t, "Exp Dot")) {
        // Exp DOT ID
        // TODO: Struct
    } else if (istype(t, "Exp ID")) {
        // ID
        struct SymNode *n = lookupSym(varTable, t->childs[0]->val.text);
        if (!n) {
            fprintf(stderr, "Error Type 17 at line %d: Undefined variable '%s'.\n", t->lineno,  t->childs[0]->val.text);
        }
        return initExpType(n, n->type);
    } else if (istype(t, "Exp Int")) {
        return initExpType(NULL, lookupSym(propTable, "int"));
    } else if (istype(t, "Exp Float")) {
        return initExpType(NULL, lookupSym(propTable, "float"));
    }
}

struct SymNode *sym_dec(struct SymNode *type, struct ast *t, bool canAssign) {
    if (istype(t, "Dec")) {
        if (t->size == 1) {
            // VarDec
            return sym_var_dec(type, t->childs[0]);
        } else if (t->size == 2) {
            // VarDec ASSIGNOP Exp
            if (!canAssign) {
                fprintf(stderr, "Error Type 15 at line %d: Assignment in struct.", t->lineno);
                return NULL;
            }
            struct SymNode *n = sym_var_dec(type, t->childs[0]);
            struct ExpType exptype = sym_exp(t->childs[1]);
            if (exptype.prop != n->type) {
                fprintf(stderr, "Error Type 5 at line %d: Unmatched type '%s' and '%s'.", t->lineno, exptype.prop->name, n->type->name);
                // TODO: clean n
                return NULL;
            }
            return n;
        }
    }
}

struct SymNode *sym_dec_list(struct SymNode *type, struct ast *t, bool canAssign) {
    if (istype(t, "DecList")) {
        if (t->size == 1) {
            // Dec
            return sym_dec(type, t->childs[0], canAssign);
        } else if (t->size == 2) {
            // Dec COMMA DecList
            struct SymNode *n = sym_dec_list(type, t->childs[1], canAssign);
            addTableItem(&n, sym_dec(type, t->childs[0], canAssign));
            return n;
        }
    }
    return NULL;
}

struct SymNode *sym_def(struct ast *t, bool canAssign) {
    if (istype(t, "Def")) {
        // Specifier DecList SEMI
        struct SymNode *type = sym_specifier(t->childs[0]);
        struct SymNode *n = sym_dec_list(type, t->childs[1], canAssign);
        return n;
    }
    return NULL;
}

struct SymNode *sym_def_list(struct ast *t, bool canAssign) {
    if (istype(t, "DefList")) {
        if (t->size == 2) {
            // Def DefList
            struct SymNode *n = sym_def(t->childs[0], canAssign);
            struct SymNode *n2 = sym_def_list(t->childs[1], canAssign);
            if (n2) {
                addTableItem(&n, n2);
            }
            return n;
        }
    }
    return NULL;
}

struct SymNode *to_struct_field(struct SymNode *t, struct SymNode *parent) {
    struct SymNode *p = t;
    while (p) {
        p->symtype = S_STRUCT_FIELD;
        p->parent = parent;
        p = p->next;
    }
    return t;
}

bool checkDup(struct SymNode *n) {
    if (!n) return false;
    struct SymNode *p1 = n, *p2;
    bool ret = false;
    while (p1 != NULL) {
        p2 = p1->next;
        while (p2 != NULL) {
            if (strcmp(p1->name, p2->name) == 0) {
                fprintf(stderr, "Error Type 15 at line %d: Duplicated struct member '%s' at line %d.\n", p1->lineno, p1->name, p2->lineno);
                ret = true;
            }
            p2 = p2->next;
        }
        p1 = p1->next;
    }
    return ret;
}

struct SymNode *sym_struct_specifier(struct ast *t) {
    if (istype(t, "StructSpecifier")) {
        if (t->size == 1) {
            // STRUCT Tag
            struct SymNode *n = lookupSym(propTable, t->childs[0]->childs[0]->val.text);
            if (!n) {
                fprintf(stderr, "Error Type 17 at line %d: Undefined struct '%s'.\n", t->lineno,  t->childs[0]->childs[0]->val.text);
                return NULL;
            }
            if (n->symtype != S_STRUCT) {
                return NULL;
            }
            return n;
        } else if (t->size == 2) {
            // STRUCT OptTag LC DefList RC
            struct SymNode *n = sym_def_list(t->childs[1], false);
            if (checkDup(n)) {
                // TODO: clean n
                return NULL;
            }
            struct SymNode *ret = NULL;
            addProp(ret = createSymNode(S_STRUCT, n, t->childs[0]->childs[0]->val.text, NULL, t->childs[0]->childs[0]->lineno, NULL));
            to_struct_field(n, ret);
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
            return createSymNode(S_VAR, type, t->childs[0]->val.text, NULL, t->childs[0]->lineno, NULL);
        } else if (t->size == 2) {
            // VarDec LB INT RB
            struct SymNode *n = sym_var_dec(type, t->childs[0]);
            struct SymNode *ret = createSymNode(S_ARRAY, n, n->name, NULL, t->childs[0]->lineno, NULL);
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

struct FunNode *createFunNode(const char *name, struct SymNode *ret, int size, struct SymNode *param, int lineno) {
    struct FunNode *temp;
    if (temp = lookupFunc(name)) {
        fprintf(stderr, "Error Type 4 at line %d: The function '%s' has been already defined at line %d.\n", lineno,  name, temp->lineno);
        return NULL;
    }
    struct FunNode *f = (struct FunNode *) malloc(sizeof(struct FunNode));
    strcpy(f->name, name);
    f->ret = ret;
    f->size = size;
    f->param = param;
    f->next = NULL;
    f->lineno = lineno;
    return f;
}

struct SymNode *sym_param_dec(struct ast *t) {
    if (istype(t, "ParamDec")) {
        // Specifier VarDec
        struct SymNode *type = sym_specifier(t->childs[0]);
        struct SymNode *n = sym_var_dec(type, t->childs[1]);
        return n;
    }
    return NULL;
}

struct SymNode *sym_var_list(struct ast *t, int *ct) {
    if (istype(t, "VarList")) {
        if (t->size == 2) {
            // ParamDec COMMA VarList
            int l;
            struct SymNode *n = sym_param_dec(t->childs[0]);
            struct SymNode *n2 = sym_var_list(t->childs[1], &l);
            *ct = l + 1;
            addTableItem(&n, n2);
            return n;
        } else if (t->size == 1) {
            // ParamDec
            *ct = 1;
            return sym_param_dec(t->childs[0]);
        }
    }
}

struct FunNode *sym_fun_dec(struct ast *t) {
    if (istype(t, "FunDec")) {
        if (t->size == 2) {
            // ID LP VarList RP
            int ct;
            struct SymNode *n = sym_var_list(t->childs[1], &ct);
            addVar(n);
            struct FunNode *f = createFunNode(t->childs[0]->val.text, NULL, ct, n, t->lineno);
            return f;
        } else if (t->size == 1) {
            // ID LP RP
            struct FunNode *f = createFunNode(t->childs[0]->val.text, NULL, 0, NULL, t->lineno);
            return f;
        }
    }
}

void sym_stmt(struct ast *t) {
    if (istype(t, "Stmt Exp")) {
        // Exp SEMI
        sym_exp(t->childs[0]);
    } else if (istype(t, "Stmt Comp")) {
        // CompSt
        sym_comp_st(t->childs[0]);
    } else if (istype(t, "Stmt Return")) {
        // RETURN Exp SEMI
        sym_exp(t->childs[0]);
    } else if (istype(t, "Stmt If")) {
        // IF LP Exp RP Stmt
        sym_exp(t->childs[0]);
        sym_stmt(t->childs[1]);
    } else if (istype(t, "Stmt IfElse")) {
        // IF LP Exp RP Stmt ELSE Stmt
        sym_exp(t->childs[0]);
        sym_stmt(t->childs[1]);
        sym_stmt(t->childs[2]);
    } else if (istype(t, "Stmt While")) {
        // WHILE LP Exp RP Stmt
        sym_exp(t->childs[0]);
        sym_stmt(t->childs[1]);
    }
}

void sym_stmt_list(struct ast *t) {
    if (istype(t, "StmtList")) {
        if (t->size == 2) {
            // Stmt StmtList
            sym_stmt(t->childs[0]);
            sym_stmt_list(t->childs[1]);
        }
    }
}

void sym_comp_st(struct ast *t) {
    if (istype(t, "CompSt")) {
        // LC DefList StmtList RC
        struct SymNode *n = sym_def_list(t->childs[0], true);
        addVar(n);
        sym_stmt_list(t->childs[1]);
    }
}

bool sym_ext_def(struct ast *t) {
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
            struct SymNode *ret = sym_specifier(t->childs[0]);
            struct FunNode *f = sym_fun_dec(t->childs[1]);
            f->ret = ret;
            addFunc(f);
            sym_comp_st(t->childs[2]);
        }
        return true;
    }
    return false;
}

void printSymbol(struct SymNode *t) {
    if (t == NULL) return;
    printf("pointer:\t%ld\nparent:\t\t%ld\nsymtype:\t%s\nname:\t\t%s\nsize:\t\t%d\nnext:\t\t%ld\ntype:\t\t%ld\n", (long) t, (long) t->parent, stype[t->symtype], t->name, t->size, (long) t->next, (long) t->type);
}

void printFunc(struct FunNode *f) {
    printf("name:\t\t%s\nret:\t\t%ld\n", f->name, (long) f->ret);
    printf("\n--\n");
    printSymbol(f->ret);
    printf("\n--\n");
    printf("size:\t\t%d\nparam:\t\t%ld\n", f->size, (long) f->param);
    printf("\n--\n");
    struct SymNode *p = f->param;
    for (int i = 0; i < f->size; ++i) {
        printSymbol(p);
        printf("\n--\n");
        p = p->next;
    }
    printf("\n");
}

void traceFunc(struct FunNode *f) {
    if (f == NULL) return;
    printFunc(f);
    printf("======\n");
    traceFunc(f->next);
}

void traceSymbol(struct SymNode *t, int level, bool next, bool type) {
    if (t == NULL) return;
    for (int i = 0; i < level; ++i) printf(">>");
    printf("\n");
    printSymbol(t);
    if (t->symtype == S_ARRAY || type || level == 0) {
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

void sym_ext_def_list(struct ast *t) {
    if (istype(t, "ExtDefList")) {
        if (t->size == 2) {
            // ExtDef ExtDefList
            sym_ext_def(t->childs[0]);
            sym_ext_def_list(t->childs[1]);
        }
    }
}

void sym_program(struct ast *t) {
    if (istype(t, "Program")) {
        // ExtDefList
        sym_ext_def_list(t->childs[0]);
    }
}

void _semantic(struct ast *t) {
    if (t == NULL) return;
    sym_program(t);
}

void semantic(struct ast *t) {
    _semantic(t);

    printf("\n\n=== Prop ===\n\n");
    traceSymbol(propTable, 0, true, true);
    printf("\n\n=== Var ===\n\n");
    traceSymbol(varTable, 0, true, false);
    printf("\n\n=== Func ===\n\n");
    traceFunc(funcTable);

    // TODO: clean symbol table
    // TODO: clean func table
}
