#ifndef JPARSE_H
#define JPARSE_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    // Primitive types
    JSON_STRING = 0,
    JSON_NUMBER = 1,
    JSON_NULL = 2,
    JSON_BOOLEAN = 3,
    // Structured types
    JSON_ARRAY = 4,
    JSON_OBJECT = 5,
} JsonType;

// Type guards to distinguish primitives/structured
#define IS_PRIMITIVE_JSON_TYPE(type) ((type) <= JSON_BOOLEAN)
#define IS_STRUCTURED_JSON_TYPE(type) ((type) >= JSON_ARRAY)

#define TRUE_STRING "true"
#define FALSE_STRING "false"
#define NULL_STRING "null"

#define TRUE_LENGTH 4
#define FALSE_LENGTH 5
#define NULL_LENGTH 4

typedef struct JsonValue JsonValue;

typedef struct {
    char* key;
    JsonValue* value;
} JsonObjectEntry;

struct JsonValue {
    JsonType type;
    union {
        double number;
        char* string;
        int boolean;
        struct {
            JsonValue** items;
            size_t count;
        } array;
        struct {
            JsonObjectEntry* entries;
            size_t count;
        } object;
    };
};

typedef struct {
    double value;
    bool is_integer;
} JsonNumber;

typedef struct {
    JsonValue** items;
    size_t length;
} JsonArray;

typedef struct {
    char** keys;
    JsonValue** values;
    size_t length;
} JsonObject;

/*
 * Public API
 * - Parse/load
 * - File I/O
 * - Free memory
 */

JsonValue* json_parse(const char* src);
void json_print(const JsonValue* value, FILE* file);

JsonValue* json_load_file(const char* filename);
bool json_save_file(const JsonValue* value, const char* filename);

void json_free(JsonValue* value);

/*
 * Internal API
 */

typedef struct {
    const char* src;
    size_t pos;
} JsonParser;

JsonValue* parse_value(JsonParser* parser);

void skip_whitespace(JsonParser* parser);
bool match(JsonParser* parser, char expected);
char peek(JsonParser* parser);
bool expect(JsonParser* parser, char expected);
char* parse_string(JsonParser* parser);
double parse_number(JsonParser* parser);
JsonValue* parse_array(JsonParser* parser);
JsonValue* parse_object(JsonParser* parser);

#endif  // JPARSE_H
