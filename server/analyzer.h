/**
 * @file analyzer.h
 * @author Aril Schultzen
 * @date 13.04.2016
 * @brief File containing function prototypes and includes for analyzer.h
 */

#ifndef ANALYZER_H
#define ANALYZER_H

#include "sensor_server.h"

/** @brief Analyzes collected NMEA data
 *
 * Analyzes collected NMEA data by calling:
 * check_moved()
 *
 *	@return Void
 */
void analyze(void);

#endif /* !ANALYZER_H */