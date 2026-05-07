#ifndef LOGGER_H
#define LOGGER_H

// Variable global para el nombre del archivo de logs
extern char log_filename[256];

// Inicializar el sistema de logging con un archivo
void init_logger(const char* filename);

// Función de logging
void log_printf(const char* format, ...);

#endif