/**
 * @csac_filter.c
 * @author Aril Schultzen
 * @date 05.09.2016
 * @brief Filter module using CSAC for the sensor_server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"

#define PHASE_LIMIT 50 	/* Load from config file in final impl */
//#define W 10000			/* Load from config file in final impl */
const double W = 10000;
#define STEER_LIMIT 50 	/* Load from config file in final impl */

FILE *csac_log;			/* Only here to pretend we have the CSAC */

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
};

int new_day(double new_mjd, double old_mjd)
{
    int n_mjd = (int)new_mjd;
    int o_mjd = (int)old_mjd;

    if(n_mjd > o_mjd) {
        return 1;
    }
    return 0;
}

float mjd_diff_day(double mjd_a, double mjd_b){
	float diff = mjd_a - mjd_b;
	return diff;
}

int get_csac_line(char *buffer, FILE *log, int buffer_size)
{
    memset(buffer, '\0', buffer_size);
    if(fgets(buffer, buffer_size, log)) {
        return 1;
    }
    return 0;
}

int load_telemetry(struct csac_filter_data *cfd)
{
    /* NOTE! query_csa0c in complete implementation */
    const int BUFFER_LEN = 100;
    char line[200];
    char buffer[BUFFER_LEN];
    get_csac_line(line, csac_log, 200);

    if(!substring_extractor(13,14,',',buffer,100,line,strlen(line))) {
        printf("Failed to extract substring from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &cfd->phase_current) == EOF) {
            return 0;
        }
    }

    if(!substring_extractor(11,12,',',buffer,100,line,strlen(line))) {
        printf("Failed to extract substring from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &cfd->steer_current) == EOF) {
            return 0;
        }
    }

    /* Use in finished implementation */
    /*
    double mjd_today = 0;
    memset(buffer, '\0', BUFFER_LEN);
    if(!get_today_mjd(buffer)){
    	printf("Failed to calculate current MJD\n");
    	return 0;
    } else {
    	if(sscanf(buffer, "%lf", &mjd_today) == EOF){
    		return 0;
    	} else {
    		if(new_day(mjd_today, cfd->t_current)){
    			cfd->new_day = 1;
    		}
    		cfd->t_current = mjd_today;
    	}
    }
    */

    /* Only used during testing */
    double mjd_today = 0;
    if(!substring_extractor(0,1,',',buffer,100,line,strlen(line))) {
        printf("Failed to extract substring from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &mjd_today) == EOF) {
            return 0;
        } else {
            if(mjd_diff_day(mjd_today, cfd->today_mjd) >= 1 && cfd->t_current != 0) {
                cfd->new_day = 1;
                cfd->today_mjd = mjd_today;
                cfd->days_passed++;
                printf("MJD diff %lf %lf\n", cfd->t_current, mjd_today);
            }
            /* Initializing today_mjd, only done once at startup */
            if(cfd->today_mjd == 0){
            	cfd->today_mjd = mjd_today;
            	cfd->days_passed = 0;
            }
            /* Updating running MJD */
            cfd->t_current = mjd_today;
        }
    }

    //printf("Load Telemetry:\nPhase Current %lf\nSteer Current %lf\nMJD %lf\n", cfd->phase_current, cfd->steer_current, cfd->t_current);
    return 1;
}

int calc_smooth(struct csac_filter_data *cfd)
{
    /* Setting previous values */
    cfd->t_smooth_previous = cfd->t_smooth_current;
    cfd->steer_smooth_previous = cfd->steer_smooth_current;
    //printf("Before smooth VALUES: %lf %lf\n", cfd->t_smooth_current, cfd->steer_smooth_current);

    /* Calculating t_smooth_current */
    cfd->t_smooth_current = (((W-1)/W) * cfd->t_smooth_previous) + ((1/W) * cfd->t_current);

    /* Calculating steer_smooth_current */
    cfd->steer_smooth_current = (((W-1)/W) * cfd->steer_smooth_previous) + ((1/W) * cfd->steer_current);
    //printf("After smooth VALUES: %lf %lf\n", cfd->t_smooth_current, cfd->steer_smooth_current);
    return 1;
}

/*
* Returns 0 if abs(phase_current) is bigger
*/
int fast_timing_filter(int phase_current)
{
    return abs(phase_current) > PHASE_LIMIT;
}

int freq_cor_filter(struct csac_filter_data *cfd)
{
	return abs(cfd->steer_current - cfd->steer_prediction) > STEER_LIMIT;
}

void update_prediction(struct csac_filter_data *cfd)
{
	/* Updating t_smooth */
	cfd->t_smooth_yesterday = cfd->t_smooth_today;
	cfd->t_smooth_today = cfd->t_smooth_current;

	/* Updating steer_smooth */
	cfd->steer_smooth_yesterday = cfd->steer_smooth_today;
	cfd->steer_smooth_today = cfd->steer_smooth_current;

	//printf("tsy: %lf, tst: %lf, ssy: %lf, sst: %lf ssc %lf\n", cfd->t_smooth_yesterday, cfd->t_smooth_today, cfd->steer_smooth_yesterday, cfd->steer_smooth_today, cfd->steer_smooth_current);
}
double steer_predict(struct csac_filter_data *cfd)
{
	cfd->steer_prediction = cfd->t_current - cfd->t_smooth_today;
	cfd->steer_prediction = cfd->steer_prediction * (cfd->steer_smooth_today - cfd->steer_smooth_yesterday);
	cfd->steer_prediction = cfd->steer_prediction / (cfd->t_smooth_today - cfd->t_smooth_yesterday);
	cfd->steer_prediction = cfd->steer_prediction+cfd->steer_smooth_today; 
	return cfd->steer_prediction;
}

/* Making sure there are no 0 values about */
int init_values(struct csac_filter_data *cfd)
{
    int status = load_telemetry(cfd);
    cfd->t_smooth_current = cfd->t_current;
    cfd->steer_smooth_current = cfd->steer_current;
    return status;
}

void init_prediction(struct csac_filter_data *cfd)
{
    cfd->steer_smooth_today = cfd->steer_smooth_current;
    cfd->steer_smooth_previous = cfd->steer_smooth_today;
    cfd->t_smooth_today = cfd->t_smooth_current;
    cfd->t_smooth_yesterday = cfd->t_smooth_current -0.1;
}

/* Good ol' main */
int main (int argc, char *argv[])
{
    /* getopt silent mode set */
    opterr = 0;

    if(argc < 2) {
        printf("Too few parameters.\n");
        return 0;
    } else if(argc > 2) {
        printf("Too many parameters, ignoring the rest.\n");
    } else {
        printf("Starting up!\n");
    }

    /* Opening CSAC log */
    csac_log = fopen(argv[1], "r");
    if (csac_log == NULL) {
        printf("Failed to load %s\n", argv[1]);
        return 0;
    }
    rewind(csac_log);

    struct csac_filter_data *cfd = calloc(1, sizeof(struct csac_filter_data));

    /* ======================================== */

    init_values(cfd);
    init_prediction(cfd);

    /* Load telemetry */
    while( load_telemetry(cfd) ) {
    	calc_smooth(cfd);
    	//steer_predict(cfd);
        /* New day!! */
        if(cfd->new_day == 1) {
            cfd->new_day = 0;
            update_prediction(cfd);
        }
    }

    /* ======================================== */
    free(cfd);
    fclose(csac_log);
    exit(0);
}