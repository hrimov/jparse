#include "../include/jparse.h"
#include <stdio.h>

int main() {
    const char *json_str = "[1, 2.5, \"hello\", true, null]";
    JsonValue *root = json_parse(json_str);

    if (!root || root->type != JSON_ARRAY) {
        printf("Failed to parse JSON array\n");
        return 1;
    }

    printf("Array contains %zu items:\n", root->array.count);

    for (size_t i = 0; i < root->array.count; i++) {
        const JsonValue *item = root->array.items[i];
        printf("[%zu] ", i);

        switch (item->type) {
            case JSON_NUMBER:
                printf("Number: %g\n", item->number);
                break;
            case JSON_STRING:
                printf("String: \"%s\"\n", item->string);
                break;
            case JSON_BOOLEAN:
                printf("Boolean: %s\n", item->boolean ? "true" : "false");
                break;
            case JSON_NULL:
                printf("Null\n");
                break;
            default:
                printf("Other type\n");
                break;
        }
    }

    json_free(root);
    return 0;
}