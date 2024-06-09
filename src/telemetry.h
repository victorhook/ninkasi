#ifndef TELEMETRY_H
#define TELEMETRY_H


typedef struct
{
    float battery;
    float current;
    float roll;
    float pitch;
    float yaw;
    float height;
}__attribute__((packed)) Telemetry;


#endif /* TELEMETRY_H */
