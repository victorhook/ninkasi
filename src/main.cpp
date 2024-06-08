#include <stdio.h>
#include "utils.h"
#include "mavcom.h"
#include "unistd.h"
#include <termios.h>

const char* fc_serial_port = "/dev/serial0";
const int   fc_serial_baud = B921600;


Mavcom mavcom(fc_serial_port, fc_serial_baud);

int main()
{
    printf("Hello world!\n");
    mavcom.init();

    while (1)
    {
        sleep(1);
    }

    return 0;
}
