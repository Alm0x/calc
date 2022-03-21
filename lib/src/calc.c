#include "calc.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "math.h"
#include "stdlib.h"

void skip_spaces(const char **c) {
    if (*c == NULL) {
        return;
    }
    while (isspace(**c)) {
        (*c)++;
    }
}

CALC_ERROR_TYPE calc_number(const char **c,
                            calc_result **res,
                            const char *begin_expr,
                            calc_result *res_) {
    calc_result cr;
    char *end = NULL;
    skip_spaces(c);
    cr.value = strtod(*c, &end);
    if (end == NULL || end == *c) {
        (*res)->error_position = (int)(*c - begin_expr);
        return CALC_ERROR_BAD_NUMBER;
    }
    *c = end;
    res_->value = cr.value;
    return CALC_ERROR_OK;
}

CALC_ERROR_TYPE calc_expr(int cur_prio,
                          const char **c,
                          calc_result **res,
                          const char *begin_expr,
                          const calc_function *functions,
                          calc_result *res_);

CALC_ERROR_TYPE calc_func(const char **c,
                          calc_result **res,
                          const char *begin_expr,
                          const calc_function *functions,
                          calc_result *res_);

CALC_ERROR_TYPE calc_atom(const char **c,
                          calc_result **res,
                          const char *begin_expr,
                          const calc_function *functions,
                          calc_result *res_) {
    calc_result cr;
    skip_spaces(c);
    if (isdigit(**c) || **c == '+' || **c == '-' || **c == '.') {
        CALC_ERROR_TYPE TYPE = calc_number(c, res, begin_expr, &cr);
        if (TYPE == CALC_ERROR_OK) {
            res_->value = cr.value;
        }
        return TYPE;
    }
    if (isalpha(**c)) {
        CALC_ERROR_TYPE TYPE = calc_func(c, res, begin_expr, functions, &cr);
        if (TYPE == CALC_ERROR_OK) {
            res_->value = cr.value;
        }
        return TYPE;
    }
    if (**c == '(') {
        (*c)++;
        skip_spaces(c);
        CALC_ERROR_TYPE TYPE = calc_expr(1, c, res, begin_expr, functions, &cr);
        if (TYPE == CALC_ERROR_OK) {
            res_->value = cr.value;
        }
        skip_spaces(c);
        if (**c == ')') {
            (*c)++;
            return TYPE;
        } else {
            (*res)->error_position = (int)(*c - begin_expr);
            return CALC_ERROR_EXPECTED_CLOSE_PAREN;
        }
    }
    (*res)->error_position = (int)(*c - begin_expr);
    return CALC_ERROR_UNEXPECTED_CHAR;
}

const char prio[128] = {
    ['+'] = 1,
    ['-'] = 1,
    ['*'] = 2,
    ['/'] = 2,
};

CALC_ERROR_TYPE calc_expr(int cur_prio,
                          const char **c,
                          calc_result **res,
                          const char *begin_expr,
                          const calc_function *functions,
                          calc_result *res_) {
    calc_result cr;
    skip_spaces(c);
    if (cur_prio == 3) {
        CALC_ERROR_TYPE TYPE = calc_atom(c, res, begin_expr, functions, &cr);
        if (TYPE == CALC_ERROR_OK) {
            res_->value = cr.value;
        }
        return TYPE;
    }
    CALC_ERROR_TYPE TYPE1 =
        calc_expr(cur_prio + 1, c, res, begin_expr, functions, &cr);
    if (TYPE1 == CALC_ERROR_OK) {
        res_->value = cr.value;
    } else {
        return TYPE1;
    }
    skip_spaces(c);
    while (prio[(int)**c] == cur_prio) {
        char op = **c;
        (*c)++;
        skip_spaces(c);
        CALC_ERROR_TYPE TYPE2 =
            calc_expr(cur_prio + 1, c, res, begin_expr, functions, &cr);
        if (TYPE2 == CALC_ERROR_OK) {
            switch (op) {
                case '+':
                    res_->value += cr.value;
                    skip_spaces(c);
                    break;
                case '-':
                    res_->value -= cr.value;
                    skip_spaces(c);
                    break;
                case '*':
                    res_->value *= cr.value;
                    skip_spaces(c);
                    break;
                case '/':
                    res_->value /= cr.value;
                    skip_spaces(c);
                    break;
            }
        } else {
            (*res)->error_position = (int)(*c - begin_expr);
            return TYPE2;
        }
    }
    return CALC_ERROR_OK;
}

CALC_ERROR_TYPE calc_func(const char **c,
                          calc_result **res,
                          const char *begin_expr,
                          const calc_function *functions,
                          calc_result *res_) {
    skip_spaces(c);
    const char *begin = *c;
    const char *end = *c;
    while (isalpha(*end)) {
        end++;
        (*c)++;
    }
    const calc_function *function = functions;
    while (function->name != NULL) {
        if (strlen(function->name) == (size_t)(end - begin) &&
            memcmp(function->name, begin, end - begin) == 0) {
            skip_spaces(c);
            if (**c != '(') {
                (*res)->error_position = (int)(*c - begin_expr);
                return CALC_ERROR_EXPECTED_OPEN_PAREN;
            }
            (*c)++;
            skip_spaces(c);
            calc_result func_args[CALC_MAX_ARITY];
            for (int i = 0; i < function->arity; i++) {
                calc_expr(1, c, res, begin_expr, functions, &func_args[i]);
                skip_spaces(c);
                if (i < function->arity - 1 && **c != ',') {
                    (*res)->error_position = (int)(*c - begin_expr);
                    return CALC_ERROR_EXPECTED_COMMA;
                }
                if (i < function->arity - 1) {
                    (*c)++;
                    skip_spaces(c);
                }
            }
            if (**c != ')') {
                (*res)->error_position = (int)(*c - begin_expr);
                return CALC_ERROR_EXPECTED_CLOSE_PAREN;
            }
            (*c)++;
            if (function->arity == 0) {
                res_->value = function->func0();
            } else if (function->arity == 1) {
                res_->value = function->func1(func_args[0].value);
            } else if (function->arity == 2) {
                res_->value =
                    function->func2(func_args[0].value, func_args[1].value);
            } else if (function->arity == 3) {
                res_->value = function->func3(
                    func_args[0].value, func_args[1].value, func_args[2].value);
            }
            break;
        } else {
            function++;
        }
    }
    if (function->name == NULL) {
        (*res)->error_position = (int)(*c - begin_expr);
        return CALC_ERROR_UNKNOWN_FUNCTION;
    }
    return CALC_ERROR_OK;
}

static const calc_function default_functions[5] = {
    [0] = {.name = "sqrt", .arity = 1, .func1 = sqrt},
    [1] = {.name = "sin", .arity = 1, .func1 = sin},
    [2] = {.name = "cos", .arity = 1, .func1 = cos},
    [3] = {.name = "pow", .arity = 2, .func2 = pow},
    [4] = CALC_FUNCTIONS_SENTINEL};

CALC_ERROR_TYPE calc_evaluate(const char *expr,
                              calc_result *res,
                              const calc_function *functions) {
    if (functions == NULL) {
        functions = default_functions;
    }
    const char *start = expr;
    calc_result cr;
    CALC_ERROR_TYPE TYPE = calc_expr(1, &expr, &res, start, functions, &cr);
    skip_spaces(&expr);
    if (TYPE == CALC_ERROR_OK && *expr != '\0') {
        res->error_position = (int)(expr - start);
        return CALC_ERROR_EXTRA_INPUT;
    }
    res->value = cr.value;
    return TYPE;
}