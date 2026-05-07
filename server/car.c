#include "car.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "logger.h"

#define printf(...) log_printf(__VA_ARGS__)

void initCar(struct CarState *car) {
    car->speed = 0;
    car->battery = 100;
    car->temp = 25.0;
    car->direction = 0; // 0 = north
}

void updateCarTelemetry(struct CarState *car, const char *command) {
    // Strip trailing spaces / null padding from command
    char clean_command[150];
    strncpy(clean_command, command, sizeof(clean_command) - 1);
    clean_command[sizeof(clean_command) - 1] = '\0';
    
    int len = strlen(clean_command);
    while (len > 0 && (clean_command[len - 1] == ' ' || clean_command[len - 1] == '\0')) {
        clean_command[len - 1] = '\0';
        len--;
    }
    
    printf("[CAR] Processing command: '%s' (len=%d)\n", clean_command, len);
    printf("[CAR] Previous state: speed=%d dir=%d battery=%d temp=%.1f\n", 
           car->speed, car->direction, car->battery, car->temp);
    
    if (strcmp(clean_command, "SPEED UP") == 0) {
        if (car->speed < 120) {
            car->speed += 10;
            printf("[CAR] Speed increased to %d km/h\n", car->speed);
        } else {
            printf("[CAR] Maximum speed reached\n");
        }
    } else if (strcmp(clean_command, "SLOW DOWN") == 0) {
        if (car->speed > 0) {
            car->speed -= 10;
            if (car->speed < 0) car->speed = 0;
            printf("[CAR] Speed reduced to %d km/h\n", car->speed);
        } else {
            printf("[CAR] Speed already 0\n");
        }
    }
    else if (strcmp(clean_command, "TURN LEFT") == 0) {
        car->direction = (car->direction - 45 + 360) % 360;
        printf("[CAR] Turned left, new heading: %d deg\n", car->direction);
    } else if (strcmp(clean_command, "TURN RIGHT") == 0) {
        car->direction = (car->direction + 45) % 360;
        printf("[CAR] Turned right, new heading: %d deg\n", car->direction);
    } else {
        printf("[CAR] Unknown command: '%s'\n", clean_command);
    }

    if (car->battery > 0) {
        car->battery -= 1;
    }

    if (car->speed > 0 && car->temp < 50.0) {
        car->temp += 0.5;
    } else if (car->speed == 0 && car->temp > 20.0) {
        car->temp -= 0.3;
    }
    
    printf("[CAR] New state: speed=%d dir=%d battery=%d temp=%.1f\n", 
           car->speed, car->direction, car->battery, car->temp);
}

void generateCarTelemetry(struct CarState car, char *buffer, size_t size) {
    snprintf(buffer, size, "speed=%d;dir=%d;battery=%d;temp=%.1f",
             car.speed, car.direction, car.battery, car.temp);
}
