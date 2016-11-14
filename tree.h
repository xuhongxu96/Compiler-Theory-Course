#pragma once

union Value {
    char text[256];
    int n;
    float f;
};

struct ast {
    union Value val;
    int type;
    struct ast *childs[10];
    int size;
    int lineno;
};

struct ast *newastTK(union Value v, int tk);
struct ast *newast(int size, union Value v);
struct ast *newast1(union Value v, struct ast *a);
struct ast *newast2(union Value v, struct ast *a, struct ast *b);
struct ast *newast3(union Value v, struct ast *a, struct ast *b, struct ast *c);
union Value makeTextVal(const char *s);
union Value makeIntVal(int i);
union Value makeFloatVal(float f);
void freetree(struct ast *t);
void tracetree(struct ast *t, int l);
