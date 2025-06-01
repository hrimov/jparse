#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../include/jparse.h"

void test_parse_null() {
    JsonValue *val = json_parse("null");
    assert(val && val->type == JSON_NULL);
    json_free(val);
}

void test_parse_boolean_true() {
    JsonValue *val = json_parse("true");
    assert(val && val->type == JSON_BOOLEAN && val->boolean);
    json_free(val);
}

void test_parse_boolean_false() {
    JsonValue *val = json_parse("false");
    assert(val && val->type == JSON_BOOLEAN && !val->boolean);
    json_free(val);
}

void test_parse_number_integer() {
    JsonValue *val = json_parse("42");
    assert(val && val->type == JSON_NUMBER && val->number == 42);
    json_free(val);
}

void test_parse_number_negative() {
    JsonValue *val = json_parse("-123");
    assert(val && val->type == JSON_NUMBER && val->number == -123);
    json_free(val);
}

void test_parse_number_float() {
    JsonValue *val = json_parse("3.14");
    assert(val && val->type == JSON_NUMBER && val->number == 3.14);
    json_free(val);
}

void test_parse_string_simple() {
    JsonValue *val = json_parse("\"hello\"");
    assert(val && val->type == JSON_STRING && strcmp(val->string, "hello") == 0);
    json_free(val);
}

void test_parse_string_empty() {
    JsonValue *val = json_parse("\"\"");
    assert(val && val->type == JSON_STRING && strcmp(val->string, "") == 0);
    json_free(val);
}

void test_parse_array_empty() {
    JsonValue *val = json_parse("[]");
    assert(val && val->type == JSON_ARRAY && val->array.count == 0);
    json_free(val);
}

void test_parse_array_numbers() {
    JsonValue *val = json_parse("[1, 2, 3]");
    assert(val && val->type == JSON_ARRAY && val->array.count == 3);

    for (int i = 0; i < 3; i++) {
        assert(val->array.items[i]->type == JSON_NUMBER);
        assert(val->array.items[i]->number == i + 1);
    }

    json_free(val);
}

void test_parse_array_mixed() {
    JsonValue *val = json_parse("[\"hello\", 42, true, null]");
    assert(val && val->type == JSON_ARRAY && val->array.count == 4);
    assert(val->array.items[0]->type == JSON_STRING);
    assert(strcmp(val->array.items[0]->string, "hello") == 0);
    assert(val->array.items[1]->type == JSON_NUMBER);
    assert(val->array.items[1]->number == 42.0);
    assert(val->array.items[2]->type == JSON_BOOLEAN);
    assert(val->array.items[2]->boolean);
    assert(val->array.items[3]->type == JSON_NULL);
    json_free(val);
}

void test_parse_object_empty() {
    JsonValue *val = json_parse("{}");
    assert(val && val->type == JSON_OBJECT && val->object.count == 0);
    json_free(val);
}

void test_parse_object_simple() {
    JsonValue *val = json_parse("{\"key\": \"value\"}");
    assert(val && val->type == JSON_OBJECT && val->object.count == 1);
    assert(strcmp(val->object.entries[0].key, "key") == 0);
    assert(val->object.entries[0].value->type == JSON_STRING);
    assert(strcmp(val->object.entries[0].value->string, "value") == 0);
    json_free(val);
}

void test_parse_object_multiple() {
    JsonValue *val = json_parse("{\"name\": \"John\", \"age\": 30, \"active\": true}");
    assert(val && val->type == JSON_OBJECT && val->object.count == 3);

    const JsonValue *name_val = NULL;
    const JsonValue *age_val = NULL;
    const JsonValue *active_val = NULL;

    // TODO: resolve `Pointer may be null`
    for (size_t i = 0; i < val->object.count; i++)  { // NOLINT
        if (strcmp(val->object.entries[i].key, "name") == 0) {
            name_val = val->object.entries[i].value;
        }
        else if (strcmp(val->object.entries[i].key, "age") == 0) {
            age_val = val->object.entries[i].value;
        }
        else if (strcmp(val->object.entries[i].key, "active") == 0) {
            active_val = val->object.entries[i].value;
        }
    }

    assert(name_val && name_val->type == JSON_STRING && strcmp(name_val->string, "John") == 0);
    assert(age_val && age_val->type == JSON_NUMBER && age_val->number == 30.0);
    assert(active_val && active_val->type == JSON_BOOLEAN && active_val->boolean);
    json_free(val);
}

// Test nested structures
void test_parse_nested() {
    JsonValue *val = json_parse("{\"users\": [{\"name\": \"Alice\", \"scores\": [95, 87]}, {\"name\": \"Bob\", \"scores\": [88, 92]}]}");
    assert(val && val->type == JSON_OBJECT && val->object.count == 1);

    // TODO: resolve `Pointer may be null`
    const JsonValue *users = val->object.entries[0].value; // NOLINT
    assert(users->type == JSON_ARRAY && users->array.count == 2);

    const JsonValue *user1 = users->array.items[0];
    assert(user1->type == JSON_OBJECT && user1->object.count == 2);
    json_free(val);
}

void test_file_save_and_load() {
    JsonValue *original = json_parse("{\"test\": \"data\", \"number\": 123}");
    assert(original);

    assert(json_save_file(original, "test_output.json"));

    JsonValue *loaded = json_load_file("test_output.json");
    assert(loaded && loaded->type == JSON_OBJECT && loaded->object.count == 2);

    json_free(original);
    json_free(loaded);
    remove("test_output.json");
}

void test_print() {
    JsonValue *val = json_parse("{\"array\": [1, 2, 3], \"string\": \"test\", \"null\": null}");
    assert(val);

    FILE *temp = fopen("test_print.json", "w");
    assert(temp);
    json_print(val, temp);
    fclose(temp);

    FILE *check = fopen("test_print.json", "r");
    assert(check);
    fseek(check, 0, SEEK_END);
    const long size = ftell(check);
    fclose(check);

    assert(size > 0);
    json_free(val);
    remove("test_print.json");
}

void test_invalid_json() {
    FILE *original_stderr = stderr;
    stderr = fopen("/dev/null", "w");

    assert(json_parse("invalid") == NULL);
    assert(json_parse("") == NULL);
    assert(json_parse("   ") == NULL);
    assert(json_parse("tru") == NULL);

    fclose(stderr);
    stderr = original_stderr;
}

void test_whitespace_handling() {
    JsonValue *val = json_parse("  {  \"key\"  :  \"value\"  }  ");
    assert(val && val->type == JSON_OBJECT && val->object.count == 1);
    assert(strcmp(val->object.entries[0].key, "key") == 0);
    assert(strcmp(val->object.entries[0].value->string, "value") == 0);
    json_free(val);
}

int main() {
    test_parse_null();
    test_parse_boolean_true();
    test_parse_boolean_false();
    test_parse_number_integer();
    test_parse_number_negative();
    test_parse_number_float();
    test_parse_string_simple();
    test_parse_string_empty();
    test_parse_array_empty();
    test_parse_array_numbers();
    test_parse_array_mixed();
    test_parse_object_empty();
    test_parse_object_simple();
    test_parse_object_multiple();
    test_parse_nested();
    test_file_save_and_load();
    test_print();
    test_invalid_json();
    test_whitespace_handling();

    return 0;
}
