#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Explicit re-include
#include <stdbool.h>
#include <stdio.h>

#include "../include/jparse.h"

void skip_whitespace(JsonParser* p) {
    while (isspace(p->src[p->pos])) {
        p->pos++;
    }
}

bool match(JsonParser* p, const char expected) {
    skip_whitespace(p);

    if (p->src[p->pos] == expected) {
        p->pos++;
        return true;
    }

    return false;
}

char peek(JsonParser* p) {
    skip_whitespace(p);
    return p->src[p->pos];
}

bool expect(JsonParser* p, const char expected) {
    if (!match(p, expected)) {
        fprintf(stderr, "Expected '%c'\n", expected);
        return false;
    }

    return true;
}

char* parse_string(JsonParser* p) {
    if (!match(p, '"')) {
        return NULL;
    }

    const char* start = &p->src[p->pos];
    size_t len = 0;

    while (p->src[p->pos] && p->src[p->pos] != '"') {
        if (p->src[p->pos] == '\\') {
            p->pos++;  // Skip escape
        }

        p->pos++;
        len++;
    }
    char* str = malloc(len + 1);
    strncpy(str, start, len);
    str[len] = '\0';
    expect(p, '"');
    return str;
}

double parse_number(JsonParser* p) {
    char buf[64];
    size_t i = 0;
    skip_whitespace(p);

    while (isdigit(p->src[p->pos]) || p->src[p->pos] == '.' || p->src[p->pos] == '-') {
        buf[i++] = p->src[p->pos++];
    }
    buf[i] = '\0';
    char* endptr;
    const double result = strtod(buf, &endptr);
    return result;
}

JsonValue* parse_array(JsonParser* p) {
    if (!match(p, '[')) {
        return NULL;
    }

    JsonValue** items = NULL;
    size_t count = 0;

    if (peek(p) != ']') {
        do {
            JsonValue* item = parse_value(p);
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

        } while (match(p, ','));
    }
    expect(p, ']');

    JsonValue* arr = malloc(sizeof(JsonValue));
    arr->type = JSON_ARRAY;
    arr->array.items = items;
    arr->array.count = count;
    return arr;
}

JsonValue* parse_object(JsonParser* p) {
    if (!match(p, '{')) {
        return NULL;
    }

    JsonObjectEntry* entries = NULL;
    size_t count = 0;

    if (peek(p) != '}') {
        do {
            char* key = parse_string(p);
            expect(p, ':');
            JsonValue* val = parse_value(p);
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
                json_free(val);

                return NULL;
            }

            entries = new_entries;
            entries[count++] = (JsonObjectEntry){key, val};

        } while (match(p, ','));
    }

    expect(p, '}');

    JsonValue* obj = malloc(sizeof(JsonValue));
    obj->type = JSON_OBJECT;
    obj->object.entries = entries;
    obj->object.count = count;
    return obj;
}

JsonValue* parse_value(JsonParser* p) {
    skip_whitespace(p);
    const char c = peek(p);
    JsonValue* val = malloc(sizeof(JsonValue));

    if (c == '"') {
        val->type = JSON_STRING;
        val->string = parse_string(p);
    } else if (c == '-' || isdigit(c)) {
        val->type = JSON_NUMBER;
        val->number = parse_number(p);
    } else if (c == '{') {
        free(val);
        return parse_object(p);
    } else if (c == '[') {
        free(val);
        return parse_array(p);
    } else if (!strncmp(&p->src[p->pos], "true", 4)) {
        p->pos += 4;
        val->type = JSON_BOOLEAN;
        val->boolean = 1;
    } else if (!strncmp(&p->src[p->pos], "false", 5)) {
        p->pos += 5;
        val->type = JSON_BOOLEAN;
        val->boolean = 0;
    } else if (!strncmp(&p->src[p->pos], "null", 4)) {
        p->pos += 4;
        val->type = JSON_NULL;
    } else {
        fprintf(stderr, "Unexpected character '%c'\n", c);
        free(val);
        return NULL;
    }

    return val;
}

JsonValue* json_parse(const char* src) {
    JsonParser p = {src, 0};
    return parse_value(&p);
}

void json_print_indent_recursively(FILE* f, const int level) {
    for (int i = 0; i < level; i++) {
        fprintf(f, "  ");
    }
}

void json_print_internal(const JsonValue* val, FILE* f, const int indent) {
    switch (val->type) {
        case JSON_NULL:
            fprintf(f, "null");
            break;
        case JSON_BOOLEAN:
            fprintf(f, val->boolean ? "true" : "false");
            break;
        case JSON_NUMBER:
            fprintf(f, "%g", val->number);
            break;
        case JSON_STRING:
            fprintf(f, "\"%s\"", val->string);
            break;
        case JSON_ARRAY:
            fprintf(f, "[\n");
            for (size_t i = 0; i < val->array.count; i++) {
                json_print_indent_recursively(f, indent + 1);
                json_print_internal(val->array.items[i], f, indent + 1);

                if (i + 1 < val->array.count) {
                    fprintf(f, ",");
                }

                fprintf(f, "\n");
            }
            json_print_indent_recursively(f, indent);
            fprintf(f, "]");
            break;
        case JSON_OBJECT:
            fprintf(f, "{\n");
            for (size_t i = 0; i < val->object.count; i++) {
                json_print_indent_recursively(f, indent + 1);
                fprintf(f, "\"%s\": ", val->object.entries[i].key);
                json_print_internal(val->object.entries[i].value, f, indent + 1);

                if (i + 1 < val->object.count) {
                    fprintf(f, ",");
                }

                fprintf(f, "\n");
            }
            json_print_indent_recursively(f, indent);
            fprintf(f, "}");
            break;
    }
}

void json_print(const JsonValue* val, FILE* f) {
    json_print_internal(val, f, 0);
    fprintf(f, "\n");
}

JsonValue* json_load_file(const char* filename) {
    FILE* f = fopen(filename, "r");

    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    const long len = ftell(f);
    rewind(f);
    char* data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);
    JsonValue* val = json_parse(data);
    free(data);
    return val;
}

bool json_save_file(const JsonValue* val, const char* filename) {
    FILE* f = fopen(filename, "w");

    if (!f) {
        return false;
    }

    json_print(val, f);
    fclose(f);
    return true;
}

void json_free(JsonValue* val) {
    if (!val) {
        return;
    }

    switch (val->type) {
        case JSON_STRING:
            free(val->string);
            break;
        case JSON_ARRAY:
            for (size_t i = 0; i < val->array.count; i++) {
                json_free(val->array.items[i]);
            }
            free(val->array.items);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < val->object.count; i++) {
                free(val->object.entries[i].key);
                json_free(val->object.entries[i].value);
            }
            free(val->object.entries);
            break;
        default:
            break;
    }
    free(val);
}
