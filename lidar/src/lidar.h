#ifndef LIDAR_H
#define LIDAR_H

#include <Wire.h>
#include "VL53L0X.h"
#include "DFRobot_VL53L0X.h"

// This enum is copy from ardupilots `Rotation` enum
enum Orientation : uint8_t
{
    Orientation_NONE                = 0,
    Orientation_YAW_45              = 1,
    Orientation_YAW_90              = 2,
    Orientation_YAW_135             = 3,
    Orientation_YAW_180             = 4,
    Orientation_YAW_225             = 5,
    Orientation_YAW_270             = 6,
    Orientation_YAW_315             = 7,
};

class Lidar
{
    public:
        Lidar(const uint8_t enable_pin, const uint8_t i2c_addr, const Orientation orientation, TwoWire* wire);
        ~Lidar();
        bool init();
        bool configure();
        bool start();
        void read_distance();
        float latest_distance() const;
        bool dist(float& distance);
        const Orientation m_orientation;

    private:
        const uint8_t m_enable_pin;
        const uint8_t m_i2c_addr;
        float m_latest_distance;
        DFRobot_VL53L0X m_lidar;
        TwoWire* m_wire;
};


#endif /* LIDAR_H */
