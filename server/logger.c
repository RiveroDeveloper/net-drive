#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "logger.h"

char log_filename[256] = "log.txt";

void init_logger(const char* filename) {
    if (filename != NULL && strlen(filename) > 0) {
        strncpy(log_filename, filename, sizeof(log_filename) - 1);
        log_filename[sizeof(log_filename) - 1] = '\0';
    }
}

void log_printf(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    FILE *f = fopen(log_filename, "a");
    if (f) {
        va_start(args, format);
        vfprintf(f, format, args);
        va_end(args);
        fclose(f);
    }
}
