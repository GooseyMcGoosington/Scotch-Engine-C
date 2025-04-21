#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ENTRIES 256

typedef struct {
    const char *section; // primary key
    unsigned int value;
    const char *key;
} iniEntry;

static iniEntry INIENTRIES[MAX_ENTRIES];
static int INI_ENTRY_COUNT = 0;

static inline unsigned TO_UINT(const char* str) {
    return (unsigned)strtoul(str, NULL, 10);
}

static inline unsigned int INI_FIND_VALUE(const char *key) {
    for (int i = 0; i < INI_ENTRY_COUNT; ++i) {
        if (strcmp(INIENTRIES[i].key, key) == 0) {
            return (int)INIENTRIES[i].value;
        }
    }
    fprintf(stderr, "Failed to find key \"%s\"\n", key);
    return 0;
}

void INI_PARSE(const char* filename) { // File must be within the main folder and not hidden deep within a subfolder.
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("%s\n", "File does not exist within root folder. Verify name & extension type.");
        return;
    }

    char line[256];
    char current_section[64] = "";

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '#' || *p == ';' || *p == '\n') continue;
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                *end = 0;
                strncpy(current_section, p + 1, sizeof(current_section));
            }
            continue;
        }
        char* equal = strchr(p, '=');
        if (equal) {
            *equal = 0;
            char* key = p;
            char* value = equal + 1;
            char* newline = strchr(value, '\n');
            if (newline) *newline = 0;
            if (INI_ENTRY_COUNT < MAX_ENTRIES) {
                INIENTRIES[INI_ENTRY_COUNT++] = (iniEntry){
                    .section = strdup(current_section),
                    .key = strdup(key),
                    .value = TO_UINT(value)
                };
            }
        }
    }
    fclose(f);
}
