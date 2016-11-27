#include "symbol.h"
#include "trans.h"
#include "tree.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void trans_comp_st(struct ast *t);

enum REG {EAX, XMM0, STRUCT};

struct ExpDesp {
    bool flag;
    char label[255];
    enum REG reg;
    struct SymNode *n;
};

struct ExpDesp initExp(bool flag, const char *l, enum REG reg, struct SymNode *n) {
    struct ExpDesp d;
    d.flag = flag;
    strcpy(d.label, l);
    d.reg = reg;
    d.n = n;
    return d;
}

int flct = 0;

struct FunNode *trans_curFunc = NULL;
void print_var(struct SymNode *v, const char *prefix);
struct ExpDesp trans_exp(struct ast *t, bool out);

void trans_args(struct ast *t, struct SymNode *param) {
    struct ExpDesp desp = trans_exp(t->childs[0], true);
    if (desp.reg == EAX) {
        printf("push\tdword [_%s]\n", param->name);
        printf("mov\t[_%s], eax\n", param->name);
    } else if (desp.reg == XMM0) {
        printf("lea\tesp, [esp-4]\n");
        printf("movss\t[esp], [_%s]\n", param->name);
        printf("movss\t[_%s], xmm0\n", param->name);
    }
    if (t->size == 2) {
        // Exp COMMA Args
        trans_args(t->childs[1], param->next);
    } else if (t->size == 1) {
        // Exp
    }
}

struct ExpDesp trans_exp(struct ast *t, bool out) {
    if (istype(t, "Exp Assign")) {
        // Exp ASSIGNOP Exp
        trans_exp(t->childs[1], true);
        struct ExpDesp desp = trans_exp(t->childs[0], false);
        if (desp.reg == EAX) {
            // INT
            printf("mov\t[%s], eax\n", desp.label);
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            // FLOAT
            printf("movss\t[%s], xmm0\n", desp.label);
            return initExp(false, "", XMM0, NULL);
        } else if (desp.reg == STRUCT) {
            printf("; struct assignment has not been implemented yet.\n");
        }
    } else if (istype(t, "Exp And")) {
        // Exp AND Exp
        trans_exp(t->childs[1], true);
        printf("push\teax\n");
        trans_exp(t->childs[0], true);
        printf("pop\tebx\n");
        printf("and\teax, ebx\n");
        return initExp(false, "", EAX, NULL);
    } else if (istype(t, "Exp Or")) {
        // Exp OR Exp
        trans_exp(t->childs[1], true);
        printf("push\teax\n");
        trans_exp(t->childs[0], true);
        printf("pop\tebx\n");
        printf("or\teax, ebx\n");
    } else if (istype(t, "Exp Relop")) {
        // Exp RELOP Exp
        struct ExpDesp desp = trans_exp(t->childs[0], true);
        if (desp.reg == EAX) {
            printf("push\teax\n");
        } else if (desp.reg == XMM0) {
            printf("lea\tesp, [esp-4]\n");
            printf("movss\t[esp], xmm0\n");
        }
        trans_exp(t->childs[2], true);
        if (desp.reg == EAX) {
            printf("pop\tebx\n");
            printf("cmp\tebx, eax\n");
        } else if (desp.reg == XMM0) {
            printf("movss\txmm1, [esp]\n");
            printf("lea\tesp, [esp+4]\n");
            printf("ucomiss\txmm1, xmm0\n");
        }
        static int relop_ct = 0;
        if (strcmp(t->childs[1]->val.text, "==") == 0) {
            printf("je\t__j_%d\n", relop_ct);
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        } else if (strcmp(t->childs[1]->val.text, "!=") == 0) {
            printf("jne\t__j_%d\n", relop_ct);
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        } else if (strcmp(t->childs[1]->val.text, "<") == 0) {
            if (desp.reg == EAX) {
                printf("jl\t__j_%d\n", relop_ct);
            } else if (desp.reg == XMM0) {
                printf("jb\t__j_%d\n", relop_ct);
            }
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        } else if (strcmp(t->childs[1]->val.text, ">") == 0) {
            if (desp.reg == EAX) {
                printf("jg\t__j_%d\n", relop_ct);
            } else if (desp.reg == XMM0) {
                printf("ja\t__j_%d\n", relop_ct);
            }
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        } else if (strcmp(t->childs[1]->val.text, "<=") == 0) {
            if (desp.reg == EAX) {
                printf("jle\t__j_%d\n", relop_ct);
            } else if (desp.reg == XMM0) {
                printf("jbe\t__j_%d\n", relop_ct);
            }
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        } else if (strcmp(t->childs[1]->val.text, ">=") == 0) {
            if (desp.reg == EAX) {
                printf("jge\t__j_%d\n", relop_ct);
            } else if (desp.reg == XMM0) {
                printf("jae\t__j_%d\n", relop_ct);
            }
            printf("jmp\t__nj_%d\n", relop_ct);
            printf("__j_%d:\n", relop_ct);
        }
        printf("mov\teax, 1\n");
        printf("jmp __endj_%d\n", relop_ct);
        printf("__nj_%d:\n", relop_ct);
        printf("mov\teax, 0\n");
        printf("__endj_%d:\n", relop_ct);
        relop_ct++;
        return initExp(false, "", EAX, NULL);
    } else if (istype(t, "Exp Plus")) {
        // Exp PLUS Exp
        struct ExpDesp desp = trans_exp(t->childs[0], true);
        if (desp.reg == EAX) {
            // INT
            printf("push\teax\n");
            trans_exp(t->childs[1], true);
            printf("pop\tebx\n");
            printf("add\teax, ebx\n");
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            // FLOAT
            printf("lea\tesp, [esp-4]\n");
            printf("movss\t[esp], xmm0\n");
            trans_exp(t->childs[1], true);
            printf("movss\txmm1, [esp]\n");
            printf("lea\tesp, [esp+4]\n");
            printf("addss\txmm0, xmm1\n");
            return initExp(false, "", XMM0, NULL);
        }
    } else if (istype(t, "Exp Minus")) {
        // Exp MINUS Exp
        struct ExpDesp desp = trans_exp(t->childs[1], true);
        if (desp.reg == EAX) {
            // INT
            printf("push\teax\n");
            trans_exp(t->childs[0], true);
            printf("pop\tebx\n");
            printf("sub\teax, ebx\n");
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            // FLOAT
            printf("lea\tesp, [esp-4]\n");
            printf("movss\t[esp], xmm0\n");
            trans_exp(t->childs[0], true);
            printf("movss\txmm1, [esp]\n");
            printf("lea\tesp, [esp+4]\n");
            printf("subss\txmm0, xmm1\n");
            return initExp(false, "", XMM0, NULL);
        }
    } else if (istype(t, "Exp Star")) {
        // Exp Star Exp
        struct ExpDesp desp = trans_exp(t->childs[1], true);
        if (desp.reg == EAX) {
            // INT
            printf("push\teax\n");
            trans_exp(t->childs[0], true);
            printf("pop\tebx\n");
            printf("imul\teax, ebx\n");
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            // FLOAT
            printf("lea\tesp, [esp-4]\n");
            printf("movss\t[esp], xmm0\n");
            trans_exp(t->childs[0], true);
            printf("movss\txmm1, [esp]\n");
            printf("lea\tesp, [esp+4]\n");
            printf("mulss\txmm0, xmm1\n");
            return initExp(false, "", XMM0, NULL);
        }
    } else if (istype(t, "Exp Div")) {
        // Exp DIV Exp
        struct ExpDesp desp = trans_exp(t->childs[1], true);
        if (desp.reg == EAX) {
            // INT
            printf("push\teax\n");
            trans_exp(t->childs[0], true);
            printf("pop\tebx\n");
            printf("mov\tedx, 0\n");
            printf("idiv\tebx\n");
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            // FLOAT
            printf("lea\tesp, [esp-4]\n");
            printf("movss\t[esp], xmm0\n");
            trans_exp(t->childs[0], true);
            printf("movss\txmm1, [esp]\n");
            printf("lea\tesp, [esp+4]\n");
            printf("divss\txmm1, xmm0\n");
            printf("movss\txmm0, xmm1\n");
            return initExp(false, "", XMM0, NULL);
        }
    } else if (istype(t, "Exp")) {
        // LP Exp RP
        return trans_exp(t->childs[0], true);
    } else if (istype(t, "Exp UMinus")) {
        // MINUS Exp
        struct ExpDesp desp = trans_exp(t->childs[0], true);
        if (desp.reg == EAX) {
            // INT
            printf("neg\teax\n");
            return initExp(false, "", EAX, NULL);
        } else if (desp.reg == XMM0) {
            printf("movss\txmm1, [__fl_neg]\n");
            printf("xorps\txmm0, xmm1\n");
            return initExp(false, "", XMM0, NULL);
        }
    } else if (istype(t, "Exp Not")) {
        // NOT Exp
        trans_exp(t->childs[0], true);
        printf("not\teax\n");
        return initExp(false, "", EAX, NULL);
    } else if (istype(t, "Exp Call")) {
        if (t->size == 2) {
            // ID LP Args RP
            struct FunNode *f = lookupFunc(t->childs[0]->val.text);
            trans_args(t->childs[1], f->param);
            printf("call\tf_%s\n", t->childs[0]->val.text);
            printf("add\tesp, %d\n", f->size * 4);
        } else if (t->size == 1) {
            // ID LP RP
            printf("call\tf_%s\n", t->childs[0]->val.text);
        }
        enum SymType type = lookupFunc(t->childs[0]->val.text)->ret->symtype;
        if (type == S_INT) {
            return initExp(false, "", EAX, NULL);
        } else if (type == S_FLOAT) {
            return initExp(false, "", XMM0, NULL);
        } 
    } else if (istype(t, "Exp Array")) {
        // Exp LB Exp RB
        // TODO: Arr
    } else if (istype(t, "Exp Dot")) {
        // Exp DOT ID
        struct ExpDesp desp = trans_exp(t->childs[0], false);
        strcat(desp.label, "_");
        strcat(desp.label, t->childs[1]->val.text);
        struct SymNode *field = lookupStruct(desp.n->type, t->childs[1]->val.text);
        if (field->type->symtype == S_INT) {
            if (out)
                printf("mov\teax, [%s]\n", desp.label);
            return initExp(true, desp.label, EAX, field);
        } else if (field->type->symtype == S_FLOAT){
            if (out)
                printf("movss\txmm0, [%s]\n", desp.label);
            return initExp(true, desp.label, XMM0, field);
        } else {
            return initExp(true, desp.label, EAX, field);
        }
    } else if (istype(t, "Exp ID")) {
        // ID
        char name[255] = "_";
        strcat(name, t->childs[0]->val.text);
        struct SymNode *n = lookupSym(varTable, t->childs[0]->val.text);
        if (n->type->symtype == S_INT) {
            if (out)
                printf("mov\teax, [_%s]\n", t->childs[0]->val.text);
            return initExp(true, name, EAX, n);
        } else if (n->type->symtype == S_FLOAT) {
            if (out)
                printf("movss\txmm0, [_%s]\n", t->childs[0]->val.text);
            return initExp(true, name, XMM0, n);
        } else {
            if (out) {
                // TODO
            }
            return initExp(true, name, STRUCT, n);
        }
    } else if (istype(t, "Exp Int")) {
        // INT
        printf("mov\teax, %d\n", t->childs[0]->val.n);
        return initExp(false, "", EAX, NULL);
    } else if (istype(t, "Exp Float")) {
        // FLOAT
        printf("movss\txmm0, [__fl_%d]\n", flct++);
        return initExp(false, "", XMM0, NULL);
    }
}

void trans_stmt(struct ast *t) {
    if (istype(t, "Stmt Exp")) {
        // Exp SEMI
        trans_exp(t->childs[0], true);
    } else if (istype(t, "Stmt Comp")) {
        // CompSt
        trans_comp_st(t->childs[0]);
    } else if (istype(t, "Stmt Return")) {
        // RETURN Exp SEMI
        trans_exp(t->childs[0], true);
        if (strcmp(trans_curFunc->name, "main") == 0)
            print_var(varTable, "");
        char pop_str[3000] = "";
        char cur_pop_str[3000] = "";
        struct SymNode *p = trans_curFunc->param;
        int i = 0;
        while (p) {
            if (p->type->symtype == S_INT) {
                sprintf(cur_pop_str, "mov\tebx, [ebp+%d]\nmov\t[_%s], ebx\n", i * 8 + 8, p->name);
            } else if (p->type->symtype == S_FLOAT) {
                sprintf(cur_pop_str, "movss\txmm1, [ebp+%d]\nmovss\t[_%s], xmm1\n", i * 8 + 8, p->name);
                i++;
            }
            strcat(cur_pop_str, pop_str);
            strcpy(pop_str, cur_pop_str);
            p = p->next;
            if (i >= trans_curFunc->size) break;
        }
        printf("%s", pop_str);
        printf("mov\tesp, ebp\n");
        printf("pop\tebp\n");
        printf("ret\n");
    } else if (istype(t, "Stmt If")) {
        // IF LP Exp RP Stmt
        static int if_ct = 0;
        trans_exp(t->childs[0], true);
        printf("cmp\teax, 0\n");
        printf("je\t__ifend_%d\n", if_ct);
        trans_stmt(t->childs[1]);
        printf("__ifend_%d:\n", if_ct);
        if_ct++;
    } else if (istype(t, "Stmt IfElse")) {
        // IF LP Exp RP Stmt ELSE Stmt
        static int ife_ct = 0;
        trans_exp(t->childs[0], true);
        printf("cmp\teax, 0\n");
        printf("je\t__ife_else_%d\n", ife_ct);
        trans_stmt(t->childs[1]);
        printf("jmp\t__ife_end_%d:\n", ife_ct);
        printf("__ife_else_%d:\n", ife_ct);
        trans_stmt(t->childs[2]);
        printf("__ife_end_%d:\n", ife_ct);
        ife_ct++;
    } else if (istype(t, "Stmt While")) {
        // WHILE LP Exp RP Stmt
        static int while_ct = 0;
        printf("__while_%d:\n", while_ct);
        trans_exp(t->childs[0], true);
        printf("cmp\teax, 0\n");
        printf("je\t__while_end_%d\n", while_ct);
        trans_stmt(t->childs[1]);
        printf("jmp\t__while_%d\n", while_ct);
        printf("__while_end_%d:\n", while_ct);
        while_ct++;
    }
}

void trans_stmt_list(struct ast *t) {
    if (t->size == 2) {
        // Stmt StmtList
        trans_stmt(t->childs[0]);
        trans_stmt_list(t->childs[1]);
    }
}

struct SymNode *trans_var_dec(struct ast *t) {
    if (t->size == 1) {
        // ID
        return lookupSym(varTable, t->childs[0]->val.text);
    } else if (t->size == 2) {
        // VarDec LB INT RB
        // can't assign to array
    }
}

void trans_dec(struct ast *t) {
    if (t->size == 1) {
        // VarDec
    } else if (t->size == 2) {
        // VarDec ASSIGNOP Exp
        struct SymNode *id = trans_var_dec(t->childs[0]);
        trans_exp(t->childs[1], true);
        printf("mov\t[_%s], eax\n", id->name);
    }
}

void trans_dec_list(struct ast *t) {
    if (t->size == 1) {
        // Dec
        trans_dec(t->childs[0]);
    } else if (t->size == 2) {
        // Dec COMMA DecList
        trans_dec(t->childs[0]);
        trans_dec_list(t->childs[1]);
    }
}

void trans_def(struct ast *t) {
    // Specifier DecList SEMI
    trans_dec_list(t->childs[1]);
}

void trans_def_list(struct ast *t) {
    if (t->size == 2) {
        // Def DefList
        trans_def(t->childs[0]);
        trans_def_list(t->childs[1]);
    }
}

void trans_comp_st(struct ast *t) {
    // LC DefList StmtList RC
    trans_def_list(t->childs[0]);
    trans_stmt_list(t->childs[1]);
}

void trans_fun_dec(struct ast *t) {
    if (t->size == 2) {
        // ID LP VarList RP
    } else if (t->size == 1) {
        // ID LP RP
    }
    printf("\nf_%s: \n", t->childs[0]->val.text);
    printf("push\tebp\n");
    printf("mov\tebp, esp\n");
    struct FunNode *f = lookupFunc(t->childs[0]->val.text);
    trans_curFunc = f;
}

void trans_ext_def(struct ast *t) {
    if (t->size == 2) {
        // Specifier ExtDecList SEMI
    } else if (t->size == 1) {
        // Specifier SEMI
    } else if (t->size == 3) {
        // Specifier FunDec CompSt
        trans_fun_dec(t->childs[1]);
        trans_comp_st(t->childs[2]);
        trans_curFunc = NULL;
    }
}

void trans_ext_def_list(struct ast *t) {
    if (t->size == 2) {
        // ExtDef ExtDefList
        trans_ext_def(t->childs[0]);
        trans_ext_def_list(t->childs[1]);
    }
}

void trans_program(struct ast *t) {
    // ExtDefList
    trans_ext_def_list(t->childs[0]);
}

void trans_var(struct SymNode *v, const char *prefix) {
    struct SymNode *t = v;
    char pp[512] = "";
    while (t) {
        int type = t->symtype;
        if (type == S_VAR) type = t->type->symtype;
        switch (type) {
        case S_INT:
            printf("%s_%s:\tresd\t1\n", prefix, t->name);
            break;
        case S_FLOAT:
            printf("%s_%s:\tresd\t1\n", prefix, t->name);
            break;
        case S_STRUCT:
            strcpy(pp, prefix);
            strcat(pp, "_");
            strcat(pp, t->name);
            trans_var(t->type->type, pp);
            break;
        }
        t = t->next;
    }
}

void print_var(struct SymNode *v, const char *prefix) {
    struct SymNode *t = v;
    char pp[512] = "";
    while (t) {
        int type = t->symtype;
        if (type == S_VAR) type = t->type->symtype;
        switch (type) {
        case S_INT:
            printf("PRINT_HEX\t4, %s_%s\n", prefix, t->name);
            printf("PRINT_CHAR\t13\n");
            printf("PRINT_CHAR\t10\n");
            break;
        case S_FLOAT:
            printf("PRINT_HEX\t4, %s_%s\n", prefix, t->name);
            printf("PRINT_CHAR\t13\n");
            printf("PRINT_CHAR\t10\n");
            break;
        case S_STRUCT:
            strcpy(pp, prefix);
            strcat(pp, "_");
            strcat(pp, t->name);
            print_var(t->type->type, pp);
            break;
        }
        t = t->next;
    }
}

void trans_fl() {
    struct FloatNode *p = flconst;
    int i = 0;
    while (p) {
        printf("__fl_%d:\tdd\t%f\n", i++, p->v);
        p = p->n;
    }
}

void translate(struct ast *t) {
    if (t == NULL) return;

    printf("%%include \"io.inc\"\n");
    printf("\nsection .data\n");
    printf("__fl_neg:\tdd\t2147483648\n");
    trans_fl();
    printf("\nsection .bss\n");
    trans_var(varTable, "");

    printf("\nsection .text\n");
    printf("GLOBAL CMAIN\n");
    printf("CMAIN:\n");
    printf("jmp f_main\n");
    trans_program(t);
}
