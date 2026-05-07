# Makefile para NetDrive Server
# Uso: make, make clean, make run

# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

# Nombre del ejecutable
TARGET = server

# Archivos fuente y objetos
SRCS = main.c car.c auth.c protocol.c logger.c
OBJS = $(SRCS:.c=.o)

# Regla principal: compilar el servidor
all: $(TARGET)

# Compilar el ejecutable
$(TARGET): $(OBJS)
	@echo "Enlazando $(TARGET)..."
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Compilación exitosa. Ejecutable: ./$(TARGET)"

# Compilar archivos .c a .o
%.o: %.c
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	@echo "Limpiando archivos compilados..."
	rm -f $(OBJS) $(TARGET) *.log
	@echo "Limpieza completa."

# Ejecutar el servidor (con parámetros por defecto)
run: $(TARGET)
	./$(TARGET) 2000 server.log

# Ejecutar con puerto personalizado
run-custom: $(TARGET)
	@echo "Uso: make run-custom PORT=<puerto> LOG=<archivo>"
	./$(TARGET) $(PORT) $(LOG)

# Mostrar ayuda
help:
	@echo "Makefile para NetDrive Server"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Compila el servidor"
	@echo "  make clean    - Elimina archivos compilados"
	@echo "  make run      - Compila y ejecuta con parámetros por defecto"
	@echo "  make help     - Muestra esta ayuda"
	@echo ""
	@echo "Uso manual:"
	@echo "  ./server <puerto> <archivo_logs>"
	@echo "  Ejemplo: ./server 2000 server.log"

# Declarar objetivos que no son archivos
.PHONY: all clean run run-custom help

