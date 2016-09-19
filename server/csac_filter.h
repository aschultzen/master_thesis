/**
 * @csac_filter.h
 * @author Aril Schultzen
 * @date 05.09.2016
 * @brief Filter module using CSAC for the sensor_server
 */

#ifndef CSAC_FILTER_H
#define CSAC_FILTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"
#include "serial.h"
 
#include "sensor_server.h"

struct csac_filter_config {
    int pred_logging;
    char pred_log_path[PATH_LENGTH_MAX];
    char cfd_log_path[PATH_LENGTH_MAX];
    int init_cfd_from_file;
    double init_cfd_ssc;
    double init_cfd_sst;
    double init_cfd_ssp;
    double phase_limit;
    double steer_limit;
    double time_constant;
    int warmup_days;
};

struct csac_filter_data {
    /* Phase */
    double phase_current;

    /* Current */
    double t_current;
    double steer_current;
    double steer_prediction;

    /* Current smooth */
    double t_smooth_current;
    double steer_smooth_current;

    /* Previous */
    double t_smooth_previous;
    double steer_smooth_previous;

 
    double t_smooth_today;
    double steer_smooth_today;


    double t_smooth_yesterday;
    double steer_smooth_yesterday;

    /* Changes once a day */
    double today_mjd;

    /* Days passed since startup */
    int days_passed;

    /* New day, 1 if yes, 0 if no */
    int new_day;

    /* Discipline mode */
    int discok;

    /* Config */
    struct csac_filter_config cf_conf;
};

/** @brief Updates the state of the filter from data
 *		   received from the CSAC
 *
 *  @param cfd State of filter
 *  @param telemetry String of telemetry from the CSAC
 *	@return 0 if error, 1 if success.
 */
int update_csac_filter(struct csac_filter_data *cfd, char *telemetry);

/** @brief Initializes the state of the filter by using 
 *		   telemetry from the CSAC.
 *
 *  @param cfd State of filter
 *  @param telemetry String of telemetry from the CSAC
 *	@return 0 if error, 1 if success.
 */
int init_csac_filter(struct csac_filter_data *cfd, char *telemetry);

/** @brief Updates the state of the filter from data
 *		   received from the CSAC
 *
 *  @param cfd State of filter
 *	@return The predicted steer value as double.
 */
double get_steer_predict(struct csac_filter_data *cfd);

/** @brief Starts the csac_filter
 *
 *  @param cfd State of filter
 *  @return 1 if filter started successfully, 0 if not.
 */
int start_csac_filter(struct csac_filter_data *cfd);

#endif /* !CSAC_FILTER_H */