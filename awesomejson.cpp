//
// Created by cyh on 2018/2/18.
//

#include "awesomejson.h"

#include <assert.h>

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

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

/**
 * parse NULL value in json context c, and return parsing result(@see awesomejson.h)
 * @param c : json to be parsed
 * @param v :in and out param
 * @return
 */
PRIVATE int parse_null(awesome_context *c, awesome_value *v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] !='l') {
        return AS_PARSE_INVALID_VALUE;
    }

    c->json += 3;
    v->type = AS_NULL;
    return AS_PARSE_OK;
}

/**
 * parse TRUE value in json context c
 * @param c
 * @param v
 * @return
 */
PRIVATE int parse_true(awesome_context *c, awesome_value *v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e') {
        return AS_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = AS_TRUE;
    return AS_PARSE_OK;
}

PRIVATE int parse_false(awesome_context *c, awesome_value *v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' ||
            c->json[1] != 'l' ||
            c->json[2] != 's' ||
            c->json[3] != 'e') {
        return AS_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    v->type = AS_FALSE;
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
        case 'n': return parse_null(c, v);
        case 't': return parse_true(c, v);
        case 'f': return parse_false(c, v);
        case '\0': return AS_PARSE_EXPECT_VALUE;
        default: return AS_PARSE_INVALID_VALUE;
    }
}

int parse(awesome_value *v, const char *json) {
    assert(v != NULL);

    awesome_context c;
    c.json = json;
    v->type = AS_NULL;
    parse_whitespace(&c);

    int ret;
    if ((ret = parse_value(&c, v)) == AS_PARSE_OK) {
        parse_whitespace(&c);
        if (*c.json != '\0') {
            ret = AS_PARSE_ROOT_NOT_SINGULAR;
        }
    }

    return ret;
}

awesome_type get_type(const awesome_value *v) {
    assert(v != NULL);
    return v->type;
}






























