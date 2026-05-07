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
    car->direction = 0; // 0 = norte
}

void updateCarTelemetry(struct CarState *car, const char *command) {
    // Limpiar el comando (eliminar espacios al final y null bytes)
    char clean_command[150];
    strncpy(clean_command, command, sizeof(clean_command) - 1);
    clean_command[sizeof(clean_command) - 1] = '\0';
    
    // Remover espacios al final
    int len = strlen(clean_command);
    while (len > 0 && (clean_command[len - 1] == ' ' || clean_command[len - 1] == '\0')) {
        clean_command[len - 1] = '\0';
        len--;
    }
    
    printf("[CAR] Procesando comando: '%s' (len=%d)\n", clean_command, len);
    printf("[CAR] Estado anterior: speed=%d dir=%d battery=%d temp=%.1f\n", 
           car->speed, car->direction, car->battery, car->temp);
    
    // Cambiar velocidad
    if (strcmp(clean_command, "SPEED UP") == 0) {
        if (car->speed < 120) {
            car->speed += 10;
            printf("[CAR] Velocidad incrementada a %d km/h\n", car->speed);
        } else {
            printf("[CAR] Velocidad maxima alcanzada\n");
        }
    } else if (strcmp(clean_command, "SLOW DOWN") == 0) {
        if (car->speed > 0) {
            car->speed -= 10;
            if (car->speed < 0) car->speed = 0;
            printf("[CAR] Velocidad reducida a %d km/h\n", car->speed);
        } else {
            printf("[CAR] Velocidad ya es 0\n");
        }
    }
    // Cambiar dirección
    else if (strcmp(clean_command, "TURN LEFT") == 0) {
        car->direction = (car->direction - 45 + 360) % 360;
        printf("[CAR] Girado a la izquierda, nueva direccion: %d grados\n", car->direction);
    } else if (strcmp(clean_command, "TURN RIGHT") == 0) {
        car->direction = (car->direction + 45) % 360;
        printf("[CAR] Girado a la derecha, nueva direccion: %d grados\n", car->direction);
    } else {
        printf("[CAR] Comando no reconocido: '%s'\n", clean_command);
    }

    // Batería se reduce con cada comando
    if (car->battery > 0) {
        car->battery -= 1;
    }

    // Temperatura depende de la velocidad
    if (car->speed > 0 && car->temp < 50.0) {
        car->temp += 0.5;
    } else if (car->speed == 0 && car->temp > 20.0) {
        car->temp -= 0.3;
    }
    
    printf("[CAR] Estado nuevo: speed=%d dir=%d battery=%d temp=%.1f\n", 
           car->speed, car->direction, car->battery, car->temp);
}

void generateCarTelemetry(struct CarState car, char *buffer, size_t size) {
    snprintf(buffer, size, "speed=%d;dir=%d;battery=%d;temp=%.1f",
             car.speed, car.direction, car.battery, car.temp);
}