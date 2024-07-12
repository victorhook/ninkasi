#ifndef AP_H
#define AP_H

#include "mavcom.h"
#include "telemetry.h"
#include "log_server.h"

class AP
{
    public:
        typedef enum
        {
            IDLE
        } ControlState;

        AP(const char* serial_port, const int baudrate);
        ~AP();
        bool init();

        /* Should be called from main loop at fixed frequency */
        void update();

        void set_control_state(const ControlState ctrl_state);

        void handle_mavlink_message(const mavlink_message_t& msg, const mavlink_status_t& status);

        Mavcom mavcom;
        Telemetry telemetry;

    private:
        uint32_t m_last_heartbeat;
        uint32_t m_uptime_t0;
        ControlState m_ctrl_state;

        void send_control_signal();

        // Mavlink handlers
        void mav_handle_sys_status(const mavlink_message_t& msg);
        void mav_handle_distance_sensor(const mavlink_message_t& msg);
        void mav_handle_rangefinder(const mavlink_message_t& msg);
        void mav_handle_radio_status(const mavlink_message_t& msg);
        void mav_handle_gps_raw_int(const mavlink_message_t& msg);
        void mav_handle_scaled_imu(const mavlink_message_t& msg);
        void mav_handle_scaled_pressure(const mavlink_message_t& msg);
        void mav_handle_attitude(const mavlink_message_t& msg);
        void mav_handle_local_position_ned(const mavlink_message_t& msg);
        void mav_handle_global_position_int(const mavlink_message_t& msg);
        void mav_handle_rc_channels(const mavlink_message_t& msg);
        void mav_handle_servo_output_raw(const mavlink_message_t& msg);
        void mav_handle_altitude(const mavlink_message_t& msg);
        void mav_handle_battery_status(const mavlink_message_t& msg);
        void mav_handle_vibration(const mavlink_message_t& msg);
        void mav_handle_esc_status(const mavlink_message_t& msg);
        void mav_handle_optical_flow(const mavlink_message_t& msg);

        void mav_handle_autopilot_version(const mavlink_message_t& msg);
        void mav_handle_heartbeat(const mavlink_message_t& msg);
        void mav_handle_statustext(const mavlink_message_t& msg);
        void mav_handle_timesync(const mavlink_message_t& msg);
        void mav_handle_command_ack(const mavlink_message_t& msg);
        void mav_handle_param_value(const mavlink_message_t& msg);
};

extern AP ap;
extern uint32_t frame_number;
extern LogServer logger;


#endif /* AP_H */
