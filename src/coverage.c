#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "coverage.h"

#define COVERAGE_FILE "coverage.log"

void coverage_hit(const char* feature) {
    FILE* f = fopen(COVERAGE_FILE, "a");
    if (f) {
        fprintf(f, "%s\n", feature);
        fclose(f);
    }
}

void coverage_reset() {
    remove(COVERAGE_FILE);
}

void coverage_report() {
    FILE* f = fopen(COVERAGE_FILE, "r");
    if (!f) {
        printf("Nenhum dado de cobertura encontrado.\n");
        return;
    }
    char linha[128];
    char* features[256];
    int count = 0;
    while (fgets(linha, sizeof(linha), f)) {
        linha[strcspn(linha, "\r\n")] = 0;
        int found = 0;
        for (int i = 0; i < count; i++) {
            if (strcmp(features[i], linha) == 0) {
                found = 1;
                break;
            }
        }
        if (!found && count < 256) {
            features[count] = strdup(linha);
            count++;
        }
    }
    fclose(f);
    printf("\nCobertura dos testes:\n");
    for (int i = 0; i < count; i++) {
        printf("%s: OK\n", features[i]);
        free(features[i]);
    }
    printf("\n");
} 