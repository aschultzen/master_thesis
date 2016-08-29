/**
 * @file analyzer.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes and includes for analyzer.h
 */

#ifndef ANALYZER_H
#define ANALYZER_H

#include "sensor_server.h"

 /** @brief Checks for any "moving" SENSORS
 *
 * Iterates through client_list
 * and checks if anyone's current position (LAT, LON, ALT, SPEED)
 * is within the ranges recorded during warm-up. If it is, the
 * dimension's disturbed value is set to SAFE (no change),
 * LOW (lower then the lowest recorded) or HIGH (higher than recorded).
 * Unless SAFE, moved is set to 1. The moved variable is used by
 * min_max_result() to raise an alarm.
 *
 * @return Void
 */
void min_max(void);

#endif /* !ANALYZER_H */