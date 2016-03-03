#ifndef ANALYZER_H
#define ANALYZER_H

#include "sensor_server.h"
    
void analyze();

#define ALARM_MOVED "[ ALARM ] Client %d was moved!\n"
#define ALARM_RETURNED "[ ALARM ] Client %d has returned!\n"

#endif /* !ANALYZER_H */