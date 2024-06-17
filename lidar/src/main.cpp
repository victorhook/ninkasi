#include <Wire.h>
#include "lidar.h"
#include "mavlink/ardupilotmega/mavlink.h"


#define MAV_SERIAL_BAUDRATE
#define MavSerial Serial1
#define MAV_SERIAL_PIN_RX 1
#define MAV_SERIAL_PIN_TX 0
#define MAV_SERIAL_BAUD 921600

#define MAV_SYSTEM_ID    10
#define MAV_COMPONENT_ID MAV_COMP_ID_PERIPHERAL

#define MAV_DISTANCE_SENSOR_PERIOD_MS 50

std::vector<Lidar> lidars{
    Lidar(6, 0x20, Orientation::Orientation_NONE,   &Wire),
    Lidar(7, 0x21, Orientation::Orientation_YAW_45, &Wire),
};


typedef struct
{
    uint8_t lidar_orient;
    float distance_cm;
} distance_obj_t;


distance_obj_t distances[2];

void setup()
{
    Serial.begin(115200);
    Wire.setSDA(4);
    Wire.setSCL(5);
    Wire.begin();

    while (!Serial);

    uint32_t t0 = millis();
    Serial.printf("Configuring %d lidars\n", lidars.size());

    // Begin all I/O
    Serial.println("Init:");
    for (Lidar& lidar : lidars)
    {
        Serial.printf("  %d: %d\n", lidar.m_orientation, lidar.init());
    }
    Serial.println("Init done");

    // Configure
    Serial.println("Configuring:");
    for (Lidar& lidar : lidars)
    {
        Serial.printf("%d: %d\n", lidar.m_orientation, lidar.configure());
    }
    Serial.println("Configuring done");

    Serial.println("Starting:");
    for (Lidar& lidar : lidars)
    {
        Serial.printf("%d: %d\n", lidar.m_orientation, lidar.start());
    }
    Serial.println("Starting done");

    Serial.printf("Initialization took %d ms\n", millis() - t0);
    Serial.println("Let's run!");
}

void setup1()
{
    // Mav serial
    MavSerial.setRX(MAV_SERIAL_PIN_RX);
    MavSerial.setTX(MAV_SERIAL_PIN_TX);
    MavSerial.begin(MAV_SERIAL_BAUD);
}

void loop1()
{
    static uint32_t latest_mav_heartbeat = 0;
    static uint32_t latest_mav_distance = 0;

    if ((millis() - latest_mav_heartbeat) > 1000)
    {
        static uint8_t buf[MAVLINK_MAX_PACKET_LEN];
        mavlink_message_t msg;
        mavlink_msg_heartbeat_pack(MAV_SYSTEM_ID, MAV_COMPONENT_ID, &msg, MAV_TYPE_ONBOARD_CONTROLLER, MAV_AUTOPILOT_INVALID, 0, 0, 0);
        uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
        MavSerial.write(buf, len);
        latest_mav_heartbeat = millis();
    }

    if ((millis() - latest_mav_distance) > MAV_DISTANCE_SENSOR_PERIOD_MS)
    {
        static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

        for (int i = 0; i < lidars.size(); i++)
        {
            distance_obj_t dist = distances[i];
            mavlink_message_t msg;
            mavlink_msg_distance_sensor_pack(
                MAV_SYSTEM_ID, MAV_COMPONENT_ID, &msg, millis(),
                5, 120, (uint16_t) dist.distance_cm, MAV_DISTANCE_SENSOR_LASER,
                dist.lidar_orient,
                dist.lidar_orient,
                0xFF, 0, 0, 0, 0
            );
            uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
            MavSerial.write(buf, len);
        }
        latest_mav_distance = millis();
    }

}


void loop()
{
    // We're polling at 20hz
    static uint32_t latest_read = 0;

    if ((millis() - latest_read) < 50)
    {
        return;
    }

    latest_read = millis();

    for (Lidar& lidar : lidars)
    {
       lidar.read_distance();
    }

    Serial.printf("%ld\n", millis());
    rp2040.idleOtherCore();
    for (int i = 0; i < lidars.size(); i++)
    {
        Lidar& lidar = lidars[i];
        distance_obj_t obj =
        {
            .lidar_orient = lidar.m_orientation,
            .distance_cm = lidar.latest_distance() / 10.0
        };
        Serial.printf("  %d: %.2f cm\n", obj.lidar_orient, obj.distance_cm);
        distances[i] = obj;
    }
    Serial.print("\n");
    rp2040.resumeOtherCore();
}