/**
 * @file filters.h
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
void min_max_filter(void);

/** @brief Checks for any "moving" SENSORS
*
* Similar to min_max_filter(), but uses values from
* the config file.
* @return Void
*/
void ref_dev_filter(void);

/** @brief Checks if a sensor has been marked as moved
 *
 * Iterates through client_list and checks for clients marked
 * as moved. Raises alarm.
 *
 * @return Void
 */
void raise_alarm(void);

#endif /* !ANALYZER_H */