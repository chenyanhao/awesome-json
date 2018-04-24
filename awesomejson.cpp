//
// Created by cyh on 2018/2/18.
//

#include "awesomejson.h"

#include <assert.h> /* assert() */
#include <errno.h> /* errno, ERANGE */
#include <math.h> /* HUGE_VAL */
#include <stdlib.h> /* NULL, strtod(), malloc(), realloc(), free() */
#include <memory.h> /* memcpy() */


#ifndef AS_PARSE_STACK_INIT_SIZE
#define AS_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)         do { *(char *)as_context_push(c, sizeof(char)) = (ch); } while(0)

/**
 *
 * 解析字符串（以及之后的数组、对象）时，需要把解析的结果先储存在一个临时的缓冲区，
 * 最后再用 lept_set_string() 把缓冲区的结果设进值之中。
 * 在完成解析一个字符串之前，这个缓冲区的大小是不能预知的。
 * 因此，可以采用动态数组（dynamic array）这种数据结构，即数组空间不足时，能自动扩展。
 *
 * 但是如果每次解析字符串时，都重新建一个动态数组，那么是比较耗时的。
 * 因此可以重用这个动态数组，每次解析 JSON 时就只需要创建一个。
 *
 * 同时经过分析可以发现：
 * 无论是解析字符串、数组或对象，只需要以先进后出的方式访问这个动态数组。
 * 换句话说，需要一个动态的堆栈数据结构。
 *
 */
typedef struct {
    const char *json;
    char *stack;
    size_t size; // 当前堆栈容量
    size_t top; // 栈顶
} awesome_context;

PRIVATE void * as_context_push(awesome_context *c, size_t size) {
    void *ret;
    assert(size > 0);

    if (c->top + size >= c->size) {
        if (c->size == 0) {
            c->size = AS_PARSE_STACK_INIT_SIZE;
        }
        while (c->top + size >= c->size) {
            c->size += c->size >> 1; /* c->size * 1.5 */
        }

        // c->stack 在初始化时为 NULL，realloc(NULL, size) 的行为是等价于 malloc(size)
        c->stack = (char *) realloc(c->stack, c->size);
    }

    ret = c->stack + c->top;
    c->top += size;

    return ret;
}

PRIVATE void * as_context_pop(awesome_context *c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

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
PRIVATE int as_parse_number(awesome_context *c, awesome_value *v) {

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
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return AS_PARSE_NUMBER_TOO_BIG;
    }

    /* 至此，正确性校验通过，直接赋值即可 */
    v->type = AS_NUMBER;
    c->json = p;

    return AS_PARSE_OK;
}


PRIVATE int as_parse_string(awesome_context *c, awesome_value *v) {
    size_t head = c->top;
    size_t len;
    const char *p;

    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = c->top - head;
                as_set_string(v, (const char *)as_context_pop(c, len), len);
                c->json = p;
                return AS_PARSE_OK;
            case '\0':
                c->top = head;
                return AS_PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c, ch);
        }
    }
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
        default: return as_parse_number(c, v);
        case '"': return as_parse_string(c, v);
        case '\0': return AS_PARSE_EXPECT_VALUE;
    }
}

int as_parse(awesome_value *v, const char *json) {
    assert(v != NULL);

    awesome_context c;
    c.json = json;
    c.stack = NULL;
    c.size = 0;
    c.top = 0;
    as_init(v);

    parse_whitespace(&c);

    int ret;
    if ((ret = parse_value(&c, v)) == AS_PARSE_OK) {
        parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = AS_NULL;
            ret = AS_PARSE_ROOT_NOT_SINGULAR;
        }
    }

    assert(c.top == 0);
    free(c.stack);

    return ret;
}


void as_free(awesome_value *v) {
    assert(v != NULL);
    if (v->type == AS_STRING) {
        free(v->u.s.s);
    }
    v->type = AS_NULL;
}

awesome_type as_get_type(const awesome_value *v) {
    assert(v != NULL);
    return v->type;
}


int as_get_boolean(const awesome_value *v) {
    /* \TODO */
    return 0;
}

void as_set_boolean(awesome_value *v, int b) {
    /* \TODO */
}

double as_get_number(const awesome_value *v) {
    assert(v != NULL && v->type == AS_NUMBER);
    return v->u.n;
}

void as_set_number(awesome_value *v, double n) {
    /* \TODO */
}

const char * as_get_string(const awesome_value *v) {
    assert(v != NULL && v->type == AS_STRING);
    return v->u.s.s;
}

size_t as_get_string_length(const awesome_value* v) {
    assert(v != NULL && v->type == AS_STRING);
    return v->u.s.len;
}

void as_set_string(awesome_value *v, const char *s, size_t len) {
    assert(v != NULL && (s != NULL || len ==0));
    as_free(v);
    v->u.s.s = (char *) malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = AS_STRING;
}






























