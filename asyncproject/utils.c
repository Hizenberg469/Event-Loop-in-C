#include "utils.h"
#include "memory.h"
#include <stdlib.h>
#include <stdio.h>

char*
hrs_min_sec_format(unsigned int seconds) {

    static char time_f[16];
    unsigned int hrs = 0,
        min = 0, sec = 0;

    if (seconds > 3600) {
        min = seconds / 60;
        sec = seconds % 60;
        hrs = min / 60;
        min = min % 60;
    }
    else {
        min = seconds / 60;
        sec = seconds % 60;
    }
    memset(time_f, 0, sizeof(time_f));
    sprintf(time_f, "%u::%u::%u", hrs, min, sec);
    return time_f;
}