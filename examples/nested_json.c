#include "../include/jparse.h"
#include <string.h>

int main() {
    const char *json_str =
        "{"
        "  \"company\": \"TechCorp\","
        "  \"employees\": ["
        "    {\"name\": \"Alice\", \"department\": \"Engineering\"},"
        "    {\"name\": \"Bob\", \"department\": \"Sales\"}"
        "  ]"
        "}";

    JsonValue *root = json_parse(json_str);

    if (!root || root->type != JSON_OBJECT) {
        printf("Failed to parse JSON\n");
        return 1;
    }

    // Find and access nested data
    for (size_t i = 0; i < root->object.count; i++) {
        if (strcmp(root->object.entries[i].key, "company") == 0) {
            const JsonValue *company = root->object.entries[i].value;

            if (company->type == JSON_STRING) {
                printf("Company: %s\n", company->string);
            }
        }

        if (strcmp(root->object.entries[i].key, "employees") == 0) {
            const JsonValue *employees = root->object.entries[i].value;

            if (employees->type == JSON_ARRAY) {
                printf("Employees:\n");

                for (size_t j = 0; j < employees->array.count; j++) {
                    const JsonValue *employee = employees->array.items[j];

                    if (employee->type == JSON_OBJECT) {
                        printf("\tEmployee %zu:\n", j + 1);

                        for (size_t k = 0; k < employee->object.count; k++) {
                            printf(
                                "\t\t%s: %s\n",
                                employee->object.entries[k].key,
                                employee->object.entries[k].value->string
                            );
                        }
                    }
                }
            }
        }
    }

    json_free(root);
    return 0;
}