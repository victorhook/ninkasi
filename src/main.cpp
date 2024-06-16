#include "utils.h"
#include "mavcom.h"
#include "ap.h"
#include "command_server.h"
#include "telemetry_server.h"
#include "video_server.h"
#include "mission/mission.h"

#include "ws_server.h"


const char* fc_serial_port      = "/dev/serial0";
const int   fc_serial_baud      = B921600;
const char* version             = "1.0";
const int COMMAND_SERVER_PORT   = 2345;
const int TELEMETRY_SERVER_PORT = 2346;
const int VIDEO_SERVER_PORT     = 2347;

AP ap(fc_serial_port, fc_serial_baud);
CommandServer command_server(COMMAND_SERVER_PORT);
TelemetryServer telemetry_server(TELEMETRY_SERVER_PORT);
VideoServer video_server(VIDEO_SERVER_PORT);
Mission* active_mission;

uint32_t frame_number = 0;


int main()
{
    //WsServer(1234).start();
    //video_server.go();
    //return 0;

    using namespace std::chrono;
    const auto period = milliseconds(20);

    printf("*** ninkasi v%s ***\n", version);


    ap.init();
    command_server.start();
    telemetry_server.go();
    telemetry_server.start();
    active_mission = Mission::get_startup_mission();

    auto next_loop = steady_clock::now();

    std::cout << "Main loop starting now" << std::endl;

    while (1)
    {
        frame_number++;

        // Main function calls that are synched to main loop goes here
        ap.update();
        telemetry_server.update();

        // Schedule next loop
        next_loop += period;
        const auto time_to_sleep = next_loop - steady_clock::now();
        if (time_to_sleep > milliseconds(0))
        {
            std::this_thread::sleep_for(time_to_sleep);
        }
    }

    return 0;
}
