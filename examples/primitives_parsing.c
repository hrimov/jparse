#include "../include/jparse.h"
#include <stdio.h>

int main() {
    const char *json_str = "{\"name\": \"Alice\", \"age\": 30, \"active\": true}";
    JsonValue *root = json_parse(json_str);

    if (!root) {
        printf("Failed to parse JSON\n");
        return 1;
    }

    if (root->type == JSON_OBJECT) {
        for (size_t i = 0; i < root->object.count; i++) {
            printf("Key: %s, ", root->object.entries[i].key);
            const JsonValue *val = root->object.entries[i].value;

            switch (val->type) {
                case JSON_STRING:
                    printf("Value: \"%s\"\n", val->string);
                    break;
                case JSON_NUMBER:
                    printf("Value: %g\n", val->number);
                    break;
                case JSON_BOOLEAN:
                    printf("Value: %s\n", val->boolean ? "true" : "false");
                    break;
                case JSON_NULL:
                    printf("Value: null\n");
                    break;
                default:
                    printf("Value: <complex>\n");
                    break;
            }
        }
    }

    json_free(root);
    return 0;
}