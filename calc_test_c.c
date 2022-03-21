#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "calc.h"

void require_eq(double a,
                double b,
                const char *file,
                int line,
                const char *astr,
                const char *bstr) {
    if (fabs(a - b) > 1e-6) {
        printf("REQUIRE_EQ failed at %s:%d: %s != %s (%lf != %lf)\n", file,
               line, astr, bstr, a, b);
        abort();
    }
}

#define REQUIRE_EQ(a, b) require_eq(a, b, __FILE__, __LINE__, #a, #b)

double my_pi(void) {
    return 3.14;  // https://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c
}

double my_hypot3(double a, double b, double c) {
    return sqrt(a * a + b * b + c * c);
}

int main(void) {
    static_assert(
        CALC_MAX_ARITY >= 3,
        "CALC_MAX_ARITY is too small");  // Lies in <assert.h> since C11,
                                         // message is optional since C23 only.
    {
        printf("Complicated expression...\n");
        calc_result res;
        REQUIRE_EQ(calc_evaluate("2+3*(4+5*-8/   2* 3)/3-pow ( 2*5 , 1+1 ) ",
                                 &res, NULL),
                   0);
        REQUIRE_EQ(res.value, -154);
        printf("    ok\n");
    }
    {
        printf("No functions...\n");
        calc_function funcs[] = {CALC_FUNCTIONS_SENTINEL};
        calc_result res;
        REQUIRE_EQ(calc_evaluate("5+pow(1, 1)", &res, funcs),
                   CALC_ERROR_UNKNOWN_FUNCTION);
        REQUIRE_EQ(res.error_position, 5);
        printf("    ok\n");
    }
    {
        printf("Custom functions...\n");
        calc_function funcs[] = {
            {.name = "pi", .arity = 0, .func0 = my_pi},
            {.name = "hypot", .arity = 3, .func3 = my_hypot3},
            CALC_FUNCTIONS_SENTINEL,
        };
        calc_result res;
        REQUIRE_EQ(calc_evaluate("hypot(2, 6, 9) * 100 + pi()", &res, funcs),
                   0);
        REQUIRE_EQ(res.value, 1103.14);
        printf("    ok\n");
    }
    printf("All ok.\n");
}
