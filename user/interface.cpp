#include <iostream>
#include <cstdint>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include "interface.h"

using namespace std;

int main() 
{
    /*Variable with the path of devicee */
    const char* device_path = "/dev/simtemp";
    /*Polling variable with all properties*/
    struct pollfd simtemp_poll;
    /*Local variable to get data from kernel */
    SimTempSample data_sample;
    /*Variables to convert timestamp to local date*/
    time_t total_sec;
    tm *day_time;
    unsigned int millisec;


    /* Open file descriptor for driver*/
    int simtemp_fd = open(device_path, O_RDONLY);
    if (simtemp_fd < 0) {
        cout << "Error trying to open file descriptor for /dev/simtemp" << endl;
        return 1;
    }

    /* File descript is associated with pollin structure */
    simtemp_poll.fd = simtemp_fd;
    simtemp_poll.events = POLLIN;

    while(true) 
    {

        /*Wait pollin answer or 5 sec */
        int ret = poll(&simtemp_poll, 1, 5000); 
        if (ret == -1) {
            cout << "Error during poll call open" << endl;
            break;
        }
        else if(ret == 0) 
        {
            cout << "Time out, POLLIN was not reached" << endl;
            continue;
        }

        /* Validate that POLLIN was the answer*/
        if (simtemp_poll.revents & POLLIN) {
            
            /* Read function for fil descript from driver*/
            ssize_t bytes_read = read(simtemp_fd, &data_sample, sizeof(data_sample));
            if (bytes_read != sizeof(data_sample)) {
                cout << "Size of data is not correct" << endl;
                continue;
            }

            /* Convert time from nano seconds to seconds and milli seconds*/
            total_sec = data_sample.timestamp_ns / NANOSECOND_TO_SECONDS;
            millisec = data_sample.timestamp_ns % NANOSECOND_TO_SECONDS;
            millisec /= NANOSECOND_TO_MILISECOND;
            day_time = localtime(&total_sec);

            /*Print data in format YEAR-MONT-DAY :: HOUR:MINUTE:SECOND:MILLISECOND :: TEMP IN C° :: ALERT (0 or 1)*/
            cout << day_time->tm_year + 1900 << "-" << day_time->tm_mon + 1<< "-" << day_time->tm_mday << "T";
            cout << day_time->tm_hour << ":" << day_time->tm_min << ":" << day_time->tm_sec << ":" << millisec <<"  ";
            cout << "Temp =  " << (data_sample.temp_mC / MICROC_TO_C) << " °C  ";
            cout << "Alert = " << (data_sample.flags & TEMP_ALERT_MASK) << endl;
        }
    }

    close(simtemp_fd);
    return 0;
}
