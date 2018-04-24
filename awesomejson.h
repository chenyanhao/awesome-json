//
// Created by cyh on 2018/2/18.
//

#ifndef AWESOME_JSON_AWESOMEJSON_H
#define AWESOME_JSON_AWESOMEJSON_H

#include <stddef.h> /* size_t */

#define PRIVATE static

typedef enum {
    AS_NULL,
    AS_FALSE,
    AS_TRUE,
    AS_NUMBER,
    AS_STRING,
    AS_ARRAY,
    AS_OBJECT
} awesome_type;

typedef struct {
    union {
        struct { /* string */
            char *s;
            size_t len;
        } s;
        double n; /* number */
    } u;
    awesome_type type;
} awesome_value;

enum {
    AS_PARSE_OK = 0,
    AS_PARSE_EXPECT_VALUE,
    AS_PARSE_INVALID_VALUE,
    AS_PARSE_ROOT_NOT_SINGULAR,
    AS_PARSE_NUMBER_TOO_BIG,
    AS_PARSE_MISS_QUOTATION_MARK,
    AS_PARSE_INVALID_STRING_ESCAPE,
    AS_PARSE_INVALID_STRING_CHAR
};

#define as_init(v) do { (v)->type = AS_NULL; } while(0)

int as_parse(awesome_value *v, const char *json);

void as_free(awesome_value *v);

awesome_type as_get_type(const awesome_value *v);

#define as_set_null(v) as_free(v)

int as_get_boolean(const awesome_value *v);
void as_set_boolean(awesome_value *v, int b);

double as_get_number(const awesome_value *v);
void as_set_number(awesome_value *v, double n);

const char * as_get_string(const awesome_value *v);
size_t as_get_string_length(const awesome_value *v);
void as_set_string(awesome_value *v, const char *s, size_t len);

#endif //AWESOME_JSON_AWESOMEJSON_H
