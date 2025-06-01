#include "../include/jparse.h"
#include <stdio.h>

int main() {
    JsonValue *data = json_parse("{\"users\": [\"Alice\", \"Bob\"], \"count\": 2}");

    if (!data) {
        printf("Failed to create JSON\n");
        return 1;
    }

    if (json_save_file(data, "output.json")) {
        printf("JSON saved to output.json\n");
    }
    else {
        printf("Failed to save file\n");
        json_free(data);
        return 1;
    }

    JsonValue *loaded = json_load_file("output.json");

    if (!loaded) {
        printf("Failed to load file\n");
        json_free(data);
        return 1;
    }

    printf("Loaded JSON from file:\n");
    json_print(loaded, stdout);

    json_free(data);
    json_free(loaded);

    return 0;
}