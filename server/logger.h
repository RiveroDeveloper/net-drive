#ifndef LOGGER_H
#define LOGGER_H

extern char log_filename[256];

void init_logger(const char* filename);

void log_printf(const char* format, ...);

#endif
