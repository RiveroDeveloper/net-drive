#ifndef CAR_H
#define CAR_H
#include <stddef.h>

struct CarState {
    int speed;       // km/h
    int battery;     // %
    float temp;      // °C internal
    int direction;   // degrees 0–359
};

void initCar(struct CarState *car);
void updateCarTelemetry(struct CarState *car, const char *command);
void generateCarTelemetry(struct CarState car, char *buffer, size_t size);

#endif
