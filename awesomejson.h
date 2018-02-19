//
// Created by cyh on 2018/2/18.
//

#ifndef AWESOME_JSON_AWESOMEJSON_H
#define AWESOME_JSON_AWESOMEJSON_H

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
    double n;
    awesome_type type;
} awesome_value;

enum {
    AS_PARSE_OK = 0,
    AS_PARSE_EXPECT_VALUE,
    AS_PARSE_INVALID_VALUE,
    AS_PARSE_ROOT_NOT_SINGULAR,
    AS_PARSE_NUMBER_TOO_BIG
};

int parse(awesome_value *v, const char *json);

awesome_type get_type(const awesome_value *v);

double get_number(const awesome_value *v);

#endif //AWESOME_JSON_AWESOMEJSON_H
