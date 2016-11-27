int a; 

struct A {
    int b;
    float c;
}x;

struct B {
    struct A in_a;
} ab;

float d;

int add(int t1, int t2) {
    return t1 + t2;
}

int main() {
    int haha = 3 + 2;
    struct A y;
    ab.in_a.b = 3;
    a = 0;
    d = 12.1;
    d = 3.4;
    x.b = a + 2; // x.b = 2
    x.c = -d; // x.c = 3.4
    y = x; // y.b = 2, y.c = 3.4
    y.b = add(a, 3); // y.b = 3
    a = a == 0; // a = 1
    return a; // return 0
}
