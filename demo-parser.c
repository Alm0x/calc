#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

const char *s;
int calc_number(void) {
    int result = 0;
    while (isdigit(*s)) {
        result = result * 10 + *s++ - '0';
    }
    return result;
}

int calc_expr(int cur_prio);

int calc_atom(void) {
    if (isdigit(*s))
        return calc_number();
    assert(*s++ == '(');
    int result = calc_expr(1);
    assert(*s++ == ')');
    return result;
}

char prio[128] = {
    ['+'] = 1,
    ['-'] = 1,
    ['*'] = 2,
    ['/'] = 2,
};

int calc_expr(int cur_prio) {
    if (cur_prio == 3) {
        return calc_atom();
    }
    int result = calc_expr(cur_prio + 1);
    while (prio[(int)*s] == cur_prio) {
        char op = *s++;
        int next = calc_expr(cur_prio + 1);
        switch (op) {
            case '+':
                result += next;
                break;
            case '-':
                result -= next;
                break;
            case '*':
                result *= next;
                break;
            case '/':
                result /= next;
                break;
        }
    }
    return result;
}

int main(void) {
    s = "(5+50)/(2+3)";
    printf("res=%d\n", calc_expr(1));  // res=11
    assert(!*s);
}
