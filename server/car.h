#ifndef CAR_H
#define CAR_H
#include <stddef.h>

// Estructura que modela el estado del carro
struct CarState {
    int speed;       // Velocidad (km/h)
    int battery;     // Batería (%)
    float temp;      // Temperatura interna (°C)
    int direction;   // Dirección en grados (0–359)
};

// Inicializa el estado del carro con valores por defecto
void initCar(struct CarState *car);

// Actualiza el estado según el comando recibido
// Ejemplos de comandos: SPEED UP, SLOW DOWN, TURN LEFT, TURN RIGHT
void updateCarTelemetry(struct CarState *car, const char *command);

// Genera una cadena de texto con los datos de telemetría actuales
void generateCarTelemetry(struct CarState car, char *buffer, size_t size);

#endif