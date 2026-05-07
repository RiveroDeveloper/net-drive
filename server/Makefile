# NetDrive server Makefile
# Targets: make | make clean | make run

CC = gcc
CFLAGS = -Wall -Wextra -pthread -g
LDFLAGS = -pthread

TARGET = server

SRCS = main.c car.c auth.c protocol.c logger.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Build OK. Run: ./$(TARGET)"

%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -f $(OBJS) $(TARGET) *.log
	@echo "Done."

run: $(TARGET)
	./$(TARGET) 2000 server.log

run-custom: $(TARGET)
	@echo "Usage: make run-custom PORT=<port> LOG=<file>"
	./$(TARGET) $(PORT) $(LOG)

help:
	@echo "NetDrive server Makefile"
	@echo "  make          Build"
	@echo "  make clean    Remove objects and binary"
	@echo "  make run      Run with 2000 / server.log"
	@echo "  make help     This help"
	@echo "Manual: ./server <port> <logfile>  (e.g. ./server 2000 server.log)"

.PHONY: all clean run run-custom help
