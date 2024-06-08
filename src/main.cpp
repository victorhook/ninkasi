#include "utils.h"
#include "mavcom.h"

#include <chrono>
#include <thread>


const char* fc_serial_port = "/dev/serial0";
const int   fc_serial_baud = B921600;
const char* version = "1.0";
uint32_t frame = 0;


Mavcom mavcom(fc_serial_port, fc_serial_baud);

int main()
{
    using namespace std::chrono;
    const auto period = milliseconds(20);

    printf("*** ninkasi v%s ***\n", version);

    mavcom.init();

    auto next_loop = steady_clock::now();

    while (1)
    {
        frame++;
        // Main function calls that are synched to main loop goes here

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
