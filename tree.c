#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
#include "tree.h"

extern int yylineno;
void yyerror2(const char *, const char *);

struct ast *newast(int size, union Value v) {
    struct ast *AST = (struct ast *) malloc(sizeof(struct ast));
    if (!AST) {
        yyerror2("C", "No Memory Space");
    }

    AST->val = v;
    AST->size = size;
    AST->type = -1;
    AST->lineno = yylineno;

    return AST;
}

struct ast *newast1(union Value v, struct ast *a) {
    struct ast *AST = newast(1, v);
    AST->childs[0] = a;

    return AST;
}

struct ast *newast2(union Value v, struct ast *a, struct ast *b) {
    struct ast *AST = newast(2, v);
    AST->childs[0] = a;
    AST->childs[1] = b;

    return AST;
}

struct ast *newast3(union Value v, struct ast *a, struct ast *b, struct ast *c) {
    struct ast *AST = newast(3, v);
    AST->childs[0] = a;
    AST->childs[1] = b;
    AST->childs[2] = c;

    return AST;
}

union Value makeTextVal(const char *s) {
    union Value v;
    strcpy(v.text, s);
    return v;
}

union Value makeIntVal(int i) {
    union Value v;
    v.n = i;
    return v;
}

union Value makeFloatVal(float f) {
    union Value v;
    v.f = f;
    return v;
}

void freetree(struct ast *t) {
    int n = t->size;
    for (int i = 0; i < n; ++i) {
        freetree(t->childs[i]);
    }
    free(t);
}

void tracetree(struct ast *t, int l) {
    for (int i = 0; i < l; ++i) {
        printf("  ");
    }
    if (t == NULL) {
        printf("null\n");
        return;
    }
    int n = t->size;
    if (t->type == INT) {
        printf("%d\n", t->val.n);
    } else if (t->type == FLOAT) {
        printf("%f\n", t->val.f);
    } else {
        printf("%s (%d)\n", t->val.text, t->lineno);
    }
    for (int i = 0; i < n; ++i) {
        tracetree(t->childs[i], l + 1);
    }
    if (n == 0 && t->type == -1) {
        tracetree(NULL, l + 1);
    }
}

struct ast *newastTK(union Value v, int tk) {
    struct ast *AST = newast(0, v);
    AST->type = tk;
    return AST;
}

