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

struct FunNode *curFunc = NULL;
bool inStruct = false;

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

void cleanSymNode(struct SymNode *n) {
    struct SymNode *p = n;
    while (p != NULL) {
        n = p;
        p = p->next;
        if (n->symtype != S_STRUCT && n->symtype != S_INT && n->symtype != S_FLOAT) {
            cleanSymNode(n->type);
            free(n);
        }
    }
}

void cleanVar() {
    cleanSymNode(varTable);
}

void cleanProp() {
    struct SymNode *p = propTable;
    while (p != NULL) {
        struct SymNode *t = p;
        if (p->symtype == S_STRUCT) {
            cleanSymNode(p->type);
        }
        p = p->next;
        free(t);
    }
}

void cleanFunNode(struct FunNode *n) {
    struct FunNode *p = n;
    while (p != NULL) {
        n = p;
        p = p->next;
        free(n);
    }
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
    addTableItem(&propTable, n);
}

void addVar(struct SymNode *n) {
    if (!n) return;
    addTableItem(&varTable, n);
}

bool istype(struct ast *t, const char *type) {
    if (t == NULL) return false;
#ifdef DEBUG
    if (strcmp(t->val.text, type) == 0) {
        printf("%s", type);
        printf("\n");
    }
#endif
    return strcmp(t->val.text, type) == 0;
}


struct SymNode *sym_specifier(struct ast *t);
struct SymNode *sym_var_dec(struct SymNode *type, struct ast *t);
void sym_comp_st(struct ast *t);
struct ExpType sym_exp(struct ast *t);

struct ExpType initExpType(struct SymNode *leftVal, struct SymNode *prop) {
    struct ExpType type;
    type.isLeftVal = (leftVal != NULL);
    type.leftVal = leftVal;
    type.prop = prop;
    return type;
}

void printExpType(struct ExpType type) {
    printf("LeftVal:\t%ld\nprop:\t\t%ld\n", (long) type.leftVal, (long) type.prop);
}

struct SymNode *lookupStruct(struct SymNode *p, const char *f) {
    struct SymNode *t = p->type;
    while (t != NULL) {
        if (strcmp(t->name, f) == 0) {
            return t;
        }
        t = t->next;
    }
    return NULL;
}

bool sym_args(struct ast *t, struct SymNode *n) {
    bool ret = true;
    if (istype(t, "Args")) {
#ifdef DEBUG
        printSymbol(n);
#endif
        struct ExpType type;
        if (t->size == 2) {
            // Exp COMMA Args
            type = sym_exp(t->childs[0]);
            ret = sym_args(t->childs[1], n->next);
        } else if (t->size == 1) {
            // Exp
            type = sym_exp(t->childs[0]);
            if (n->next != NULL) {
                fprintf(stderr, "Error Type 9 at line %d: Function lacks params.\n", t->lineno);
                return false;
            }
        }
        if (n == NULL) {
            fprintf(stderr, "Error Type 9 at line %d: Function needs params '%s'.\n", t->lineno, n->name);
            return false;
        }
        if (type.prop == n->type) {
            return ret;
        } else {
            if (n->type && type.prop) {
                fprintf(stderr, "Error Type 9 at line %d: Unmatched type of param '%s' (needs '%s', but provided '%s').\n", t->lineno, n->name, n->type->name, type.prop->name);
            } else {
                fprintf(stderr, "Error Type 9 at line %d: Unmatched type of param '%s'.\n", t->lineno, n->name);
            }
            return false;
        }
    }
}

struct ExpType sym_exp(struct ast *t) {
    if (istype(t, "Exp Assign")) {
        // Exp ASSIGNOP Exp
        struct ExpType type = sym_exp(t->childs[0]);
        if (!type.isLeftVal) {
            fprintf(stderr, "Error Type 6 at line %d: Assigned to a right value.\n", t->lineno);
            return initExpType(NULL, NULL);
        }
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type.prop != type2.prop) {
            char n1[256], n2[256];
            strcpy(n1, type.prop->name);
            strcpy(n2, type2.prop->name);
            if (type.prop->parent && type.prop->parent->symtype == S_ARRAY) {
                strcat(n1, "*");
            }
            if (type2.prop->parent && type2.prop->symtype == S_ARRAY) {
                strcat(n2, "*");
            }
            fprintf(stderr, "Error Type 5 at line %d: Unmatched type '%s' and '%s'.\n", t->lineno, n1, n2);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp And") 
            || istype(t, "Exp Or")) {
        // Exp AND/OR Exp
        struct ExpType type = sym_exp(t->childs[0]);
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type.prop->symtype != S_INT || type2.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: Boolean operator can only be applied to int values.\n", t->lineno);
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
            fprintf(stderr, "Error Type 7 at line %d: Arithmetical operator can only be applied to int or float values.\n", t->lineno);
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
            fprintf(stderr, "Error Type 7 at line %d: Arithmetical operator can only be applied to int or float values.\n", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp Not")) {
        // NOT Exp
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: Boolean operator can only be applied to int values.\n", t->lineno);
            return initExpType(NULL, NULL);
        }
        return initExpType(NULL, type.prop);
    } else if (istype(t, "Exp Call")) {
        if (t->size == 2) {
            // ID LP Args RP
            struct FunNode *f = lookupFunc(t->childs[0]->val.text);
            if (!f) {
                if (lookupSym(varTable, t->childs[0]->val.text)) {
                    fprintf(stderr, "Error Type 11 at line %d: '%s' is not a function.\n", t->lineno, t->childs[0]->val.text);
                } else {
                    fprintf(stderr, "Error Type 2 at line %d: Function '%s' undefined.\n", t->lineno, t->childs[0]->val.text);
                }
                return initExpType(NULL, NULL);
            }
            sym_args(t->childs[1], f->param);
            return initExpType(NULL, f->ret);
        } else if (t->size == 1) {
            // ID LP RP
            struct FunNode *f = lookupFunc(t->childs[0]->val.text);
            if (!f) {
                if (lookupSym(varTable, t->childs[0]->val.text)) {
                    fprintf(stderr, "Error Type 11 at line %d: '%s' is not a function.\n", t->lineno, t->childs[0]->val.text);
                } else {
                    fprintf(stderr, "Error Type 2 at line %d: Function '%s' undefined.\n", t->lineno, t->childs[0]->val.text);
                }
                return initExpType(NULL, NULL);
            }
            if (f->size != 0) {
                fprintf(stderr, "Error Type 9 at line %d: Function needs %d params.\n", t->lineno, f->size);
                return initExpType(NULL, f->ret);
            }
            return initExpType(NULL, f->ret);
        }
        return initExpType(NULL, NULL);
    } else if (istype(t, "Exp Array")) {
        // Exp LB Exp RB
        struct ExpType type1 = sym_exp(t->childs[0]);
        if (type1.leftVal == NULL || type1.leftVal->symtype != S_ARRAY) {
            fprintf(stderr, "Error Type 10 at line %d: '[]' must be used for an array.\n", t->lineno);
            return initExpType(NULL, NULL);
        }
        struct ExpType type2 = sym_exp(t->childs[1]);
        if (type2.prop == NULL || type2.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 12 at line %d: Index of array must be an int value.\n", t->lineno);
            return initExpType(type1.leftVal->type, type1.leftVal->type->type);
        }
        return initExpType(type1.leftVal->type, type1.leftVal->type->type);
    } else if (istype(t, "Exp Dot")) {
        // Exp DOT ID
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_STRUCT) {
            fprintf(stderr, "Error Type 13 at line %d: '.' applied to a non-struct variable.\n", t->lineno);
            return initExpType(NULL, NULL);
        }
        struct SymNode *field = lookupStruct(type.prop, t->childs[1]->val.text);
        if (!field) {
            fprintf(stderr, "Error Type 14 at line %d: struct field '%s' is undefined.\n", t->lineno, t->childs[1]->val.text);
            return initExpType(NULL, NULL);
        }
        return initExpType(field, field->type);
    } else if (istype(t, "Exp ID")) {
        // ID
        struct SymNode *n = lookupSym(varTable, t->childs[0]->val.text);
        if (!n) {
            fprintf(stderr, "Error Type 1 at line %d: Undefined variable '%s'.\n", t->lineno,  t->childs[0]->val.text);
            return initExpType(NULL, NULL);
        }
        return initExpType(n, n->type);
    } else if (istype(t, "Exp Int")) {
        return initExpType(NULL, lookupSym(propTable, "int"));
    } else if (istype(t, "Exp Float")) {
        return initExpType(NULL, lookupSym(propTable, "float"));
    }
}

struct SymNode *sym_dec(struct SymNode *type, struct ast *t) {
    if (istype(t, "Dec")) {
        if (t->size == 1) {
            // VarDec
            return sym_var_dec(type, t->childs[0]);
        } else if (t->size == 2) {
            // VarDec ASSIGNOP Exp
            if (inStruct) {
                fprintf(stderr, "Error Type 15 at line %d: Assignment in struct.\n", t->lineno);
                return NULL;
            }
            struct SymNode *n = sym_var_dec(type, t->childs[0]);
            struct ExpType exptype = sym_exp(t->childs[1]);
            if (exptype.prop != n->type) {
                fprintf(stderr, "Error Type 5 at line %d: Unmatched type '%s' and '%s'.\n", t->lineno, exptype.prop->name, n->type->name);
                cleanSymNode(n);
                return NULL;
            }
            return n;
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
            addTableItem(&n, sym_dec(type, t->childs[0]));
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

struct SymNode *to_struct_field(struct SymNode *t, struct SymNode *parent) {
    struct SymNode *p = t;
    while (p) {
        // p->symtype = S_STRUCT_FIELD;
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
            inStruct = true;
            struct SymNode *n = sym_def_list(t->childs[1]);
            inStruct = false;
            if (checkDup(n)) {
                cleanSymNode(n);
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
            n->parent = ret;
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
    struct FunNode *f = (struct FunNode *) malloc(sizeof(struct FunNode));
    struct FunNode *temp;
    if (temp = lookupFunc(name)) {
        fprintf(stderr, "Error Type 4 at line %d: The function '%s' has been already defined at line %d.\n", lineno,  name, temp->lineno);
        f->name[0] = '\0';
    } else {
        strcpy(f->name, name);
    }
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
        struct ExpType type = sym_exp(t->childs[0]);
        if (curFunc->ret != type.prop) {
            fprintf(stderr, "Error Type 8 at line %d: Return type unmatched.\n", t->lineno);
        }
    } else if (istype(t, "Stmt If")) {
        // IF LP Exp RP Stmt
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: If condition must be int value.\n", t->lineno);
        }
        sym_stmt(t->childs[1]);
    } else if (istype(t, "Stmt IfElse")) {
        // IF LP Exp RP Stmt ELSE Stmt
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: If condition must be int value.\n", t->lineno);
        }
        sym_stmt(t->childs[1]);
        sym_stmt(t->childs[2]);
    } else if (istype(t, "Stmt While")) {
        // WHILE LP Exp RP Stmt
        struct ExpType type = sym_exp(t->childs[0]);
        if (type.prop->symtype != S_INT) {
            fprintf(stderr, "Error Type 7 at line %d: If condition must be int value.\n", t->lineno);
        }
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
        struct SymNode *n = sym_def_list(t->childs[0]);
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
            curFunc = f;
            sym_comp_st(t->childs[2]);
            curFunc = NULL;
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
        if (p) {
            p = p->next;
        } else {
            printf("ERR PARAM\n");
        }
        printf("\n--\n");
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
    initSymTable();
    _semantic(t);

    printf("\n\n=== Prop ===\n\n");
    traceSymbol(propTable, 0, true, true);
    printf("\n\n=== Var ===\n\n");
    traceSymbol(varTable, 0, true, false);
    printf("\n\n=== Func ===\n\n");
    traceFunc(funcTable);

    cleanVar();
    cleanProp();
    cleanFunNode(funcTable);
}
