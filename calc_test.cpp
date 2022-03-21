#include "calc.h"
#include <limits>
#include <string>
#include "doctest.h"

static_assert(CALC_MAX_ARITY >= 3);

TEST_CASE("Error codes") {
    CHECK(CALC_ERROR_OK == 0);
    CHECK(CALC_ERROR_BAD_NUMBER != 0);
    CHECK(CALC_ERROR_UNKNOWN_FUNCTION != 0);
    CHECK(CALC_ERROR_EXPECTED_OPEN_PAREN != 0);
    CHECK(CALC_ERROR_EXPECTED_COMMA != 0);
    CHECK(CALC_ERROR_EXPECTED_CLOSE_PAREN != 0);
    CHECK(CALC_ERROR_UNEXPECTED_CHAR != 0);
}

TEST_CASE("Basic operations") {
    calc_result res;
    SUBCASE("Number") {
        REQUIRE(calc_evaluate("123", &res) == 0);
        CHECK(res.value == doctest::Approx(123));
    }
    SUBCASE("Negative number") {
        REQUIRE(calc_evaluate("-123", &res) == 0);
        CHECK(res.value == doctest::Approx(-123));
    }
    SUBCASE("Addition") {
        REQUIRE(calc_evaluate("100+20", &res) == 0);
        CHECK(res.value == doctest::Approx(120));
    }
    SUBCASE("Subtraction") {
        REQUIRE(calc_evaluate("100-20", &res) == 0);
        CHECK(res.value == doctest::Approx(80));
    }
    SUBCASE("Multiplication") {
        REQUIRE(calc_evaluate("100*20", &res) == 0);
        CHECK(res.value == doctest::Approx(2000));
    }
    SUBCASE("Division") {
        REQUIRE(calc_evaluate("100/20", &res) == 0);
        CHECK(res.value == doctest::Approx(5));
    }
    SUBCASE("Parenthesis") {
        REQUIRE(calc_evaluate("(100)", &res) == 0);
        CHECK(res.value == doctest::Approx(100));
    }
    SUBCASE("Unary function") {
        REQUIRE(calc_evaluate("sqrt(144)", &res) == 0);
        CHECK(res.value == doctest::Approx(12));
    }
    SUBCASE("Binary function") {
        REQUIRE(calc_evaluate("pow(2,8)", &res) == 0);
        CHECK(res.value == doctest::Approx(256));
    }
}

TEST_CASE("Extra input") {
    calc_result res;
    REQUIRE(calc_evaluate("12 34", &res) == CALC_ERROR_EXTRA_INPUT);
    CHECK(res.error_position == 3);
}

TEST_CASE("Bad number") {
    calc_result res;
    REQUIRE(calc_evaluate("1 * -wtf", &res) == CALC_ERROR_BAD_NUMBER);
    CHECK(res.error_position == 4);
}

TEST_CASE("Simple expression") {
    calc_result res;
    REQUIRE(calc_evaluate("2+3*4+5", &res) == 0);
    CHECK(res.value == doctest::Approx(19));
}

TEST_CASE("Associativity") {
    SUBCASE("Additions") {
        calc_result res;
        REQUIRE(calc_evaluate("1+10+100-1000-10000+100000", &res) == 0);
        CHECK(res.value == doctest::Approx(89111));
    }
    SUBCASE("Multiplications") {
        calc_result res;
        REQUIRE(calc_evaluate("12*5*7/3*11/5", &res) == 0);
        CHECK(res.value == doctest::Approx(308));
    }
}

TEST_CASE("Complicated expression") {
    calc_result res;
    REQUIRE(calc_evaluate("2+3*(4+5*-8/   2* 3)/3-pow ( 2*5 , 1+1 ) ", &res) ==
            0);
    CHECK(res.value == doctest::Approx(-154));
}

TEST_CASE("Long expression") {
    const int N = 10'000'000;
    std::string s;
    s.reserve(2 * N - 1);
    for (int i = 0; i < N; i++) {
        if (i > 0) {
            s += '+';
        }
        s += '1';
    }

    calc_result res;
    REQUIRE(calc_evaluate(s.c_str(), &res) == 0);
    CHECK(res.value == doctest::Approx(N));
}

TEST_CASE("Deep expression") {
    const int DEPTH = 1000;
    std::string s;
    s.append(DEPTH, '(');
    s.append("1234");
    s.append(DEPTH, ')');

    calc_result res;
    REQUIRE(calc_evaluate(s.c_str(), &res) == 0);
    CHECK(res.value == doctest::Approx(1234));
}

TEST_CASE("Floating-point numbers") {
    calc_result res;
    REQUIRE(calc_evaluate("1.2+3.4*0.1", &res) == 0);
    CHECK(res.value == doctest::Approx(1.54));
}

TEST_CASE("Weird and big floating-point numbers") {
    calc_result res;
    REQUIRE(calc_evaluate("+InFiNiTy/-1e300", &res) == 0);
    CHECK(res.value == -std::numeric_limits<double>::infinity());
}

TEST_CASE("Call with nullptr functions") {
    calc_result res;
    REQUIRE(calc_evaluate("2+pow(3, 4)", &res, nullptr) == 0);
    CHECK(res.value == doctest::Approx(83));
}

TEST_CASE("Call with custom functions") {
    calc_function funcs[] = {
        []() {
            calc_function f;
            f.name = "foo";
            f.arity = 3;
            f.func3 = [](double a, double b, double c) {
                return a * a + b * b + c * c;
            };
            return f;
        }(),
        CALC_FUNCTIONS_SENTINEL,
    };
    SUBCASE("Call custom function") {
        calc_result res;
        REQUIRE(calc_evaluate("foo(2, 3, 4)", &res, funcs) == 0);
        CHECK(res.value == doctest::Approx(29));
    }
    SUBCASE("Call predefined function which should be missing") {
        calc_result res;
        REQUIRE(calc_evaluate("2 + pow   (3, 4)", &res, funcs) ==
                CALC_ERROR_UNKNOWN_FUNCTION);
        CHECK(res.error_position == 7);
    }
}
