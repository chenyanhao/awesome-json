
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "awesomejson.h"

PRIVATE int main_ret = 0;
PRIVATE int test_count = 0;
PRIVATE int test_pass = 0;


/**
 *
 * @param equality
 * @param expect
 * @param actual
 * @param format
 */
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", \
                    __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")


PRIVATE void test_parse_null() {
    awesome_value v;
    v.type = AS_FALSE; // 随便先初始化一个值，下同
    EXPECT_EQ_INT(AS_PARSE_OK, parse(&v, "null"));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));
}

PRIVATE void test_parse_expect_value() {
    awesome_value v;
    v.type = AS_FALSE;
    EXPECT_EQ_INT(AS_PARSE_EXPECT_VALUE, parse(&v, ""));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));

    v.type = AS_FALSE;
    EXPECT_EQ_INT(AS_PARSE_EXPECT_VALUE, parse(&v, " "));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));
}

PRIVATE void test_parse_invalid_value() {
    awesome_value v;
    v.type = AS_FALSE;
    EXPECT_EQ_INT(AS_PARSE_INVALID_VALUE, parse(&v, "nul"));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));


    v.type = AS_FALSE;
    EXPECT_EQ_INT(AS_PARSE_INVALID_VALUE, parse(&v, "?"));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));
}

PRIVATE void test_parse_root_not_singular() {
    awesome_value v;
    v.type = AS_FALSE;

    EXPECT_EQ_INT(AS_PARSE_ROOT_NOT_SINGULAR, parse(&v, "null x"));
    EXPECT_EQ_INT(AS_NULL, get_type(&v));
}

PRIVATE void test_parse_true() {
    awesome_value v;
    v.type = AS_FALSE;
    EXPECT_EQ_INT(AS_PARSE_OK, parse(&v, "true"));
    EXPECT_EQ_INT(AS_TRUE, get_type(&v));
}

PRIVATE void test_parse_false() {
    awesome_value v;
    v.type = AS_TRUE;
    EXPECT_EQ_INT(AS_PARSE_OK, parse(&v, "false"));
    EXPECT_EQ_INT(AS_FALSE, get_type(&v));
}

PRIVATE void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}




































