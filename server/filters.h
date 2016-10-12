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
* Checks solved position against known position.
* Known position loaded from the config file.
* @return Void
*/
void krl_filter(void);

/** @brief Checks if a sensor has been marked as moved
 *
 * Iterates through client_list and checks for clients marked
 * as moved. Raises alarm.
 *
 * @return Void
 */
void raise_alarm(void);

#endif /* !ANALYZER_H */