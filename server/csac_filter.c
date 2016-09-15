#include "csac_filter.h"

const int PHASE_LIMIT = 50 ;	/* Load from config file in final impl */
const int STEER_LIMIT = 50; 	/* Load from config file in final impl */
const double W = 10000;			/* Sample rate */
const int warmup_days = 0;


static float mjd_diff_day(double mjd_a, double mjd_b)
{
    float diff = mjd_a - mjd_b;
    return diff;
}

/* Used during tests to read line by line from text file */
int get_csac_line(char *buffer, FILE *log, int buffer_size)
{
    memset(buffer, '\0', buffer_size);
    if(fgets(buffer, buffer_size, log)) {
        return 1;
    }
    return 0;
}

static int load_telemetry(struct csac_filter_data *cfd, char *telemetry)
{
    const int BUFFER_LEN = 100;
    char buffer[BUFFER_LEN];

    /* Checking discipline mode of the CSAC */
    if(!substring_extractor(13,14,',',buffer,100,telemetry,strlen(telemetry))) {
        printf("Failed to extract DiscOK from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%d", &cfd->discok) == EOF) {
            return 0;
        }
        /* CSAC is in holdover or acquiring */
        if(cfd->discok == 2){
            return 0;
        }
    }

    if(!substring_extractor(12,13,',',buffer,100,telemetry,strlen(telemetry))) {
        printf("Failed to extract Phase from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &cfd->phase_current) == EOF) {
            return 0;
        }
    }

    if(!substring_extractor(10,11,',',buffer,100,telemetry,strlen(telemetry))) {
        printf("Failed to extract Steer from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &cfd->steer_current) == EOF) {
            return 0;
        }
    }

    double mjd_today = 0;
    memset(buffer, '\0', BUFFER_LEN);
    if(!get_today_mjd(buffer)) {
        printf("Failed to calculate current MJD\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &mjd_today) == EOF) {
            return 0;
        } else {
            if(mjd_diff_day(mjd_today, cfd->today_mjd) >= 1 && cfd->t_current != 0) {
                cfd->new_day = 1;
                cfd->today_mjd = mjd_today;
                cfd->days_passed++;
            }
            // Initializing today_mjd, only done once at startup
            if(cfd->today_mjd == 0) {
                cfd->today_mjd = mjd_today;
                cfd->days_passed = 0;
            }
            // Updating running MJD
            cfd->t_current = mjd_today;
        }
    }
    return 1;
}

static void calc_smooth(struct csac_filter_data *cfd)
{
    /* Setting previous values */
    cfd->t_smooth_previous = cfd->t_smooth_current;
    cfd->steer_smooth_previous = cfd->steer_smooth_current;

    /* Calculating t_smooth_current */
    cfd->t_smooth_current = (((W-1)/W) * cfd->t_smooth_previous) + ((1/W) * cfd->t_current);

    /* Calculating steer_smooth_current */
    cfd->steer_smooth_current = (((W-1)/W) * cfd->steer_smooth_previous) + ((1/W) * cfd->steer_current);
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

static void update_prediction(struct csac_filter_data *cfd)
{
    /* Updating t_smooth */
    cfd->t_smooth_yesterday = cfd->t_smooth_today;
    cfd->t_smooth_today = cfd->t_smooth_current;

    /* Updating steer_smooth */
    cfd->steer_smooth_yesterday = cfd->steer_smooth_today;
    cfd->steer_smooth_today = cfd->steer_smooth_current;

    /* Updating steer prediction, just for show */
    get_steer_predict(cfd);
}

double get_steer_predict(struct csac_filter_data *cfd)
{
    if(cfd->days_passed >= warmup_days) {
        cfd->steer_prediction = cfd->t_current - cfd->t_smooth_today;
        cfd->steer_prediction = cfd->steer_prediction * (cfd->steer_smooth_today - cfd->steer_smooth_yesterday);
        cfd->steer_prediction = cfd->steer_prediction / (cfd->t_smooth_today - cfd->t_smooth_yesterday);
        cfd->steer_prediction = cfd->steer_prediction+cfd->steer_smooth_today;
        return cfd->steer_prediction;
    } else {
        return -1;
    }
}

/* Making sure there are no 0 values about */
int init_csac_filter(struct csac_filter_data *cfd, char *telemetry)
{

    if(!load_telemetry(cfd, telemetry)) {
        return 0;
    }

    /* Setting preliminary values, don't want to divide by zero */
    cfd->t_smooth_current = cfd->t_current;
    cfd->t_smooth_today = cfd->t_smooth_current;
    cfd->t_smooth_yesterday = cfd->t_smooth_current -0.1;

    /* Setting values from config if preset */
    if(s_conf->init_cfd_from_file){
        cfd->steer_smooth_current = s_conf->init_cfd_ssc;
        cfd->steer_smooth_today = s_conf->init_cfd_sst;
        cfd->steer_smooth_previous = s_conf->init_cfd_ssp;
    
    /* Setting preliminary values, don't want to divide by zero */
    } else {
        cfd->steer_smooth_current = cfd->steer_current;
        cfd->steer_smooth_today = cfd->steer_smooth_current;
        cfd->steer_smooth_previous = cfd->steer_smooth_today;
    }

    return 1;
}

/* Update the filter with new data */
int update_csac_filter(struct csac_filter_data *cfd, char *telemetry)
{
    /* Load new telemetry into the filter */
    if(!load_telemetry(cfd, telemetry) ) {
        return 0;
    }

    /* Calculate smoothed values */
    calc_smooth(cfd);

    /* Updating prediction if 24 hours has passed since the last update */
    if(cfd->new_day == 1) {

        /* Update prediction */
        update_prediction(cfd);

        /* Clearing new day variable*/
        cfd->new_day = 0;

        /* If logging is enabled, log steer predicted */
        if(s_conf->pred_logging) {
            char log_output[200];
            memset(log_output, '\0', 200);
            snprintf(log_output, 100, "%lf\n", cfd->steer_prediction);
            log_to_file(s_conf->pred_log_path, log_output, 1);
        }
    }
    return 1;
}

int start_csac_filter(struct csac_filter_data *cfd)
{
    /* Allocating buffer for run_program() */
    char program_buf[200];
    memset(program_buf, '\0', 200);
    int status = 0;

    int filter_initialized = 0;
    /* Running prgram requesting telemetry from CSAC */
    /* Rework this part, the whole shablang fucks up if the python script fails
    to return data */

    while(status > 1 && (!done)){ 

        /* Acquiring lock*/
        sem_wait(&(s_synch->csac_mutex));
        
        /* Querying CSAC */
        status = run_command("python get_telemetry.py", program_buf);
        
        /* Releasing lock */
        sem_post(&(s_synch->csac_mutex));

        /* Initialize filter if not already initialized */
        if(!filter_initialized) {
            filter_initialized = init_csac_filter(cfd, program_buf);

        /* If initialized, update filter with new values */
        } else {
            update_csac_filter(cfd, program_buf);
        }

        /* If logging enabled, log all data from the CSAC */
        if(s_conf->csac_logging) {
            log_to_file(s_conf->csac_log_path, program_buf, 1);
        }

        /* Dump filter data for every iteration */
        dump_cfd(s_conf->cfd_log_path);

        sleep(0.5);
        memset(program_buf, '\0', 200);
    }
    return 0;
}
