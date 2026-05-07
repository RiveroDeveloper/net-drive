#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"

// Variable global para el nombre del archivo de logs
char log_filename[256] = "log.txt";

// Inicializar el sistema de logging
void init_logger(const char* filename) {
    if (filename != NULL && strlen(filename) > 0) {
        strncpy(log_filename, filename, sizeof(log_filename) - 1);
        log_filename[sizeof(log_filename) - 1] = '\0';
    }
}

void log_printf(const char *formato, ...) {
    va_list args;

    va_start(args, formato);
    vprintf(formato, args);  // Consola
    va_end(args);

    FILE *f = fopen(log_filename, "a");
    if (f) {
        va_start(args, formato);
        vfprintf(f, formato, args); // Archivo
        va_end(args);
        fclose(f);
    }
}
