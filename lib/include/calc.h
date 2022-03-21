#ifndef CALC_H
#define CALC_H

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_MAX_ARITY 3

typedef struct {
    const char *name;
    int arity;
#ifdef _MSC_VER  // Код внутри #ifdef компилируется только на Visual Studio
#pragma warning(push)  // Сохранить текущие настройки предупреждений
#pragma warning(disable : 4201)  // Отключить C4201
#endif
    union {
        double (*func0)(void);
        double (*func1)(double);
        double (*func2)(double, double);
        double (*func3)(double, double, double);
    };
#ifdef _MSC_VER
#pragma warning(pop)  // Восстановить настройки предупреждений
#endif
} calc_function;

#define CALC_FUNCTIONS_SENTINEL \
    {                           \
        NULL, 0, {              \
            NULL                \
        }                       \
    }

#define CALC_ERROR_TYPE int
#define CALC_ERROR_OK 0
#define CALC_ERROR_BAD_NUMBER 1
#define CALC_ERROR_UNKNOWN_FUNCTION 2
#define CALC_ERROR_EXPECTED_OPEN_PAREN 3
#define CALC_ERROR_EXPECTED_COMMA 4
#define CALC_ERROR_EXPECTED_CLOSE_PAREN 5
#define CALC_ERROR_UNEXPECTED_CHAR 6
#define CALC_ERROR_EXTRA_INPUT 7

typedef struct {
    double value;
    int error_position;
} calc_result;

CALC_ERROR_TYPE calc_evaluate(const char *expr,
                              calc_result *res,
                              const calc_function *functions
#ifdef __cplusplus
                              = nullptr
#endif
);

#ifdef __cplusplus
}
#endif
#endif