//
// Created by cyh on 2018/2/18.
//

#include "awesomejson.h"

#include <assert.h> /* assert() */
#include <errno.h> /* errno, ERANGE */
#include <math.h> /* HUGE_VAL */
#include <stdlib.h> /* NULL, strtod() */

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')

#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char *json;
} awesome_context;

/**
 * skip whitespace at begin
 * @param c
 */
PRIVATE void parse_whitespace(awesome_context *c) {
    const char *j = c->json;
    while (*j == ' ' ||
            *j == '\t' ||
            *j == '\n' ||
            *j == '\r') {
        j++;
    }
    c->json = j;
}

PRIVATE int as_parse_literal(awesome_context *c, awesome_value *v,
                             const char *literal, awesome_type type) {
    EXPECT(c, literal[0]);

    size_t i;
    for (i= 0; literal[i + 1] ; ++i) {
        if (c->json[i] != literal[i + 1]) {
            return AS_PARSE_INVALID_VALUE;
        }
    }

    c->json += i;
    v->type = type;
    return AS_PARSE_OK;
}

/**
 * [负号] 整数 [小数(点)] [指数]
 *  number = [ "-" ] int [ frac ] [ exp ]
 *  int = "0" / digit1-9 *digit
 *  frac = "." 1*digit
 *  exp = ("e" / "E") ["-" / "+"] 1*digit
 *
 * @param c
 * @param v
 * @return
 */
PRIVATE int parse_number(awesome_context *c, awesome_value *v) {

    /**
     * 下面步骤 1-5 是数字的合法性校验
     */

    const char *p = c->json;
    /* 1. 负号直接跳过 */
    if (*p == '-') {
        ++p;
    }

    /* 2. 整数部分有两种合法情况，一是单个 0，否则是一个 1-9 再加上任意数量的 digit。 */
    if (*p == '0') {
        ++p;
    } else {
        if (!ISDIGIT1TO9(*p)) {
            return AS_PARSE_INVALID_VALUE;
        }

        /**
         * 下面的 for 循环等价于如下代码，下同
         *
         *  p++;
         *  while (ISDIGIT(*p)) {
         *      p++;
         *  }
         *
         */
        for (p++; ISDIGIT(*p); ++p);
    }

    /* 3. 如果出现小数点，跳过，然后检查它至少应有一个 digit，不是 digit 就返回错误码 */
    if (*p == '.') {
        ++p;
        if (!ISDIGIT(*p)) {
            return AS_PARSE_INVALID_VALUE;
        }
        for (p++; ISDIGIT(*p); ++p);
    }

    /* 4. 如果出现大小写 e，就表示有指数部分。跳过那个 e 之后，可以有一个正或负号，其他和小数的逻辑是一样的。 */
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') {
            ++p;
        }
        if (!ISDIGIT(*p)) {
            return AS_PARSE_INVALID_VALUE;
        }
        for (p++; ISDIGIT(*p); ++p);
    }

    /* 5. 数字过大的处理 */
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return AS_PARSE_NUMBER_TOO_BIG;
    }

    /* 至此，正确性校验通过，直接赋值即可 */
    v->type = AS_NUMBER;
    c->json = p;

    return AS_PARSE_OK;
}

/**
 *
 * @param c : json to be parsed
 * @param v : in and out param
 * @return
 */
PRIVATE int parse_value(awesome_context *c, awesome_value *v) {
    switch (*c->json) {
        case 'n': return as_parse_literal(c, v, "null", AS_NULL);
        case 't': return as_parse_literal(c, v, "true", AS_TRUE);
        case 'f': return as_parse_literal(c, v, "false", AS_FALSE);
        default: return parse_number(c, v);
        case '\0': return AS_PARSE_EXPECT_VALUE;
    }
}

int as_parse(awesome_value *v, const char *json) {
    assert(v != NULL);

    awesome_context c;
    c.json = json;
    v->type = AS_NULL;
    parse_whitespace(&c);

    int ret;
    if ((ret = parse_value(&c, v)) == AS_PARSE_OK) {
        parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = AS_NULL;
            ret = AS_PARSE_ROOT_NOT_SINGULAR;
        }
    }

    return ret;
}

awesome_type as_get_type(const awesome_value *v) {
    assert(v != NULL);
    return v->type;
}

double as_get_number(const awesome_value *v) {
    assert(v != NULL && v->type == AS_NUMBER);
    return v->n;
}





























