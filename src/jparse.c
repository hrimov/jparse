#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Explicit re-include
#include <stdbool.h>
#include <stdio.h>

#include "../include/jparse.h"

void skip_whitespace(JsonParser* parser) {
    while (isspace(parser->src[parser->pos])) {
        parser->pos++;
    }
}

bool match(JsonParser* parser, const char expected) {
    skip_whitespace(parser);

    if (parser->src[parser->pos] == expected) {
        parser->pos++;
        return true;
    }

    return false;
}

char peek(JsonParser* parser) {
    skip_whitespace(parser);
    return parser->src[parser->pos];
}

bool expect(JsonParser* parser, const char expected) {
    if (!match(parser, expected)) {
        fprintf(stderr, "Expected '%c'\n", expected);
        return false;
    }

    return true;
}

char* parse_string(JsonParser* parser) {
    if (!match(parser, '"')) {
        return NULL;
    }

    const char* start = &parser->src[parser->pos];
    size_t len = 0;

    while (parser->src[parser->pos] && parser->src[parser->pos] != '"') {
        if (parser->src[parser->pos] == '\\') {
            parser->pos++;  // Skip escape
        }

        parser->pos++;
        len++;
    }
    char* str = malloc(len + 1);
    strncpy(str, start, len);
    str[len] = '\0';
    expect(parser, '"');
    return str;
}

double parse_number(JsonParser* parser) {
    const size_t buffer_size = 64;
    char buffer[buffer_size];
    size_t idx = 0;
    skip_whitespace(parser);

    while (isdigit(parser->src[parser->pos]) || parser->src[parser->pos] == '.' ||
           parser->src[parser->pos] == '-') {
        buffer[idx++] = parser->src[parser->pos++];
    }
    buffer[idx] = '\0';
    char* endptr;
    const double result = strtod(buffer, &endptr);
    return result;
}

JsonValue* parse_array(JsonParser* parser) {
    if (!match(parser, '[')) {
        return NULL;
    }

    JsonValue** items = NULL;
    size_t count = 0;

    if (peek(parser) != ']') {
        do {
            JsonValue* item = parse_value(parser);
            JsonValue** new_items = realloc(items, sizeof(JsonValue*) * (count + 1));

            if (!new_items) {
                // Cleanup on realloc failure
                for (size_t i = 0; i < count; i++) {
                    // TODO: resolve `Pointer may be null`
                    json_free(items[i]);  // NOLINT
                }
                free(items);

                return NULL;
            }

            items = new_items;
            items[count++] = item;

        } while (match(parser, ','));
    }
    expect(parser, ']');

    JsonValue* arr = malloc(sizeof(JsonValue));
    arr->type = JSON_ARRAY;
    arr->array.items = items;
    arr->array.count = count;
    return arr;
}

JsonValue* parse_object(JsonParser* parser) {
    if (!match(parser, '{')) {
        return NULL;
    }

    JsonObjectEntry* entries = NULL;
    size_t count = 0;

    if (peek(parser) != '}') {
        do {
            char* key = parse_string(parser);
            expect(parser, ':');
            JsonValue* value = parse_value(parser);
            JsonObjectEntry* new_entries = realloc(entries, sizeof(JsonObjectEntry) * (count + 1));

            if (!new_entries) {
                // Cleanup on realloc failure
                for (size_t i = 0; i < count; i++) {
                    // TODO: resolve `Pointer may be null`
                    free(entries[i].key);  // NOLINT
                    json_free(entries[i].value);
                }
                free(entries);
                free(key);
                json_free(value);

                return NULL;
            }

            entries = new_entries;
            entries[count++] = (JsonObjectEntry){key, value};

        } while (match(parser, ','));
    }

    expect(parser, '}');

    JsonValue* obj = malloc(sizeof(JsonValue));
    obj->type = JSON_OBJECT;
    obj->object.entries = entries;
    obj->object.count = count;
    return obj;
}

JsonValue* parse_value(JsonParser* parser) {
    skip_whitespace(parser);
    const char first_char = peek(parser);
    JsonValue* val = malloc(sizeof(JsonValue));

    if (first_char == '"') {
        val->type = JSON_STRING;
        val->string = parse_string(parser);
    } else if (first_char == '-' || isdigit(first_char)) {
        val->type = JSON_NUMBER;
        val->number = parse_number(parser);
    } else if (first_char == '{') {
        free(val);
        return parse_object(parser);
    } else if (first_char == '[') {
        free(val);
        return parse_array(parser);
    } else if (!strncmp(&parser->src[parser->pos], TRUE_STRING, TRUE_LENGTH)) {
        parser->pos += TRUE_LENGTH;
        val->type = JSON_BOOLEAN;
        val->boolean = 1;
    } else if (!strncmp(&parser->src[parser->pos], FALSE_STRING, FALSE_LENGTH)) {
        parser->pos += FALSE_LENGTH;
        val->type = JSON_BOOLEAN;
        val->boolean = 0;
    } else if (!strncmp(&parser->src[parser->pos], NULL_STRING, NULL_LENGTH)) {
        parser->pos += NULL_LENGTH;
        val->type = JSON_NULL;
    } else {
        fprintf(stderr, "Unexpected character '%c'\n", first_char);
        free(val);
        return NULL;
    }

    return val;
}

JsonValue* json_parse(const char* src) {
    JsonParser parser = {src, 0};
    return parse_value(&parser);
}

void json_print_indent_recursively(FILE* file, const int level) {
    for (int i = 0; i < level; i++) {
        fprintf(file, "  ");
    }
}

void json_print_internal(const JsonValue* value, FILE* file, const int indent) {
    switch (value->type) {
        case JSON_NULL:
            fprintf(file, NULL_STRING);
            break;
        case JSON_BOOLEAN:
            fprintf(file, value->boolean ? TRUE_STRING : FALSE_STRING);
            break;
        case JSON_NUMBER:
            fprintf(file, "%g", value->number);
            break;
        case JSON_STRING:
            fprintf(file, "\"%s\"", value->string);
            break;
        case JSON_ARRAY:
            fprintf(file, "[\n");
            for (size_t i = 0; i < value->array.count; i++) {
                json_print_indent_recursively(file, indent + 1);
                json_print_internal(value->array.items[i], file, indent + 1);

                if (i + 1 < value->array.count) {
                    fprintf(file, ",");
                }

                fprintf(file, "\n");
            }
            json_print_indent_recursively(file, indent);
            fprintf(file, "]");
            break;
        case JSON_OBJECT:
            fprintf(file, "{\n");
            for (size_t i = 0; i < value->object.count; i++) {
                json_print_indent_recursively(file, indent + 1);
                fprintf(file, "\"%s\": ", value->object.entries[i].key);
                json_print_internal(value->object.entries[i].value, file, indent + 1);

                if (i + 1 < value->object.count) {
                    fprintf(file, ",");
                }

                fprintf(file, "\n");
            }
            json_print_indent_recursively(file, indent);
            fprintf(file, "}");
            break;
    }
}

void json_print(const JsonValue* value, FILE* file) {
    json_print_internal(value, file, 0);
    fprintf(file, "\n");
}

JsonValue* json_load_file(const char* filename) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    const long len = ftell(file);
    rewind(file);
    char* data = malloc(len + 1);
    fread(data, 1, len, file);
    data[len] = '\0';
    fclose(file);
    JsonValue* value = json_parse(data);
    free(data);
    return value;
}

bool json_save_file(const JsonValue* value, const char* filename) {
    FILE* file = fopen(filename, "w");

    if (!file) {
        return false;
    }

    json_print(value, file);
    fclose(file);
    return true;
}

void json_free(JsonValue* value) {
    if (!value) {
        return;
    }

    switch (value->type) {
        case JSON_STRING:
            free(value->string);
            break;
        case JSON_ARRAY:
            for (size_t i = 0; i < value->array.count; i++) {
                json_free(value->array.items[i]);
            }
            free(value->array.items);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < value->object.count; i++) {
                free(value->object.entries[i].key);
                json_free(value->object.entries[i].value);
            }
            free(value->object.entries);
            break;
        default:
            break;
    }
    free(value);
}
