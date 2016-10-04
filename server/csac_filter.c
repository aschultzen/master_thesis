#include "csac_filter.h"

/* PATH TO CONFIG FILE */
#define CSAC_FILTER_CONFIG_PATH "cfilter_config.ini"

/* CONFIG CONSTANTS */
#define CONFIG_PRED_LOGGING "pred_logging: "
#define CONFIG_PRED_LOG_PATH "pred_log_path: "
#define CONFIG_CFD_PATH "cfd_state_path: "
#define CONFIG_INIT_FROM_FILE "init_cfd_from_file: "
#define CONFIG_INIT_SSC "init_cfd_steer_smooth_current: "
#define CONFIG_INIT_SST "init_cfd_steer_smooth_today: "
#define CONFIG_INIT_SSP "init_cfd_steer_smooth_previous: "
#define CONFIG_INIT_SSY "init_cfd_steer_smooth_yesterday: "
#define CONFIG_PHASE_LIMIT "phase_limit: "
#define CONFIG_STEER_LIMIT "steer_limit: "
#define CONFIG_PRED_LIMIT "pred_limit: "
#define CONFIG_TIME_CONSTANT "time_constant: "
#define CONFIG_WARMUP_DAYS "warmup_days: "
#define CSAC_FILTER_CONFIG_ENTRIES 13


static float mjd_diff_day(double mjd_a, double mjd_b)
{
    float diff = mjd_a - mjd_b;
    return diff;
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
            // Initializi      ng today_mjd, only done once at startup
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
    double W = cfd->cf_conf.time_constant;

    /* Setting previous values */
    cfd->t_smooth_previous = cfd->t_smooth_current;
    cfd->steer_smooth_previous = cfd->steer_smooth_current;

    /* Calculating t_smooth_current */
    cfd->t_smooth_current = (((W-1)/W) * cfd->t_smooth_previous) + ((1/W) * cfd->t_current);

    /* Calculating steer_smooth_current */
    cfd->steer_smooth_current = (((W-1)/W) * cfd->steer_smooth_previous) + ((1/W) * cfd->steer_current);
}

/*
* Returns 1 if abs(phase_current) is bigger
*/
int fast_timing_filter(int phase_current, int phase_limit)
{
    if(abs(phase_current) > phase_limit){
        return 1;
    }
    return 0;
}

/*
* Returns 1 if abs(cfd->steer_current - cfd->steer_prediction) is bigger
*/
int freq_cor_filter(struct csac_filter_data *cfd)
{
    if ( abs(cfd->steer_current - cfd->steer_prediction) > cfd->cf_conf.steer_limit){
        return 1;
    }
    return 0;
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
    if(cfd->days_passed >= cfd->cf_conf.warmup_days) {
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
    if(cfd->cf_conf.init_cfd_from_file){
        cfd->steer_smooth_current = cfd->cf_conf.init_cfd_ssc;
        cfd->steer_smooth_today = cfd->cf_conf.init_cfd_sst;
        cfd->steer_smooth_previous = cfd->cf_conf.init_cfd_ssp;
        cfd->steer_smooth_yesterday = cfd->cf_conf.init_cfd_ssy;
    
    /* Setting preliminary values, don't want to divide by zero */
    } else {
        cfd->steer_smooth_current = cfd->steer_current;
        cfd->steer_smooth_today = cfd->steer_smooth_current;
        cfd->steer_smooth_previous = cfd->steer_smooth_today;
    }

    if(cfd->cf_conf.warmup_days == 0){
        cfd->new_day = 1;
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

    /* If current steer is bigger than the predicted limit */
    if( abs(cfd->steer_current) > cfd->cf_conf.pred_limit){
        /* Print warning message */
        fprintf(stderr,"CLOCK CONCISTENCY ALARM!\n");

	if(1 + 1 == 3){
        /* Allocating buffer for run_program() */
        char program_buf[200];
        memset(program_buf, '\0', 200);

        /* Buffer for the prediction */ 
        char pred_string[200];
        memset(pred_string, '\0', 200);
        sprintf(pred_string, "%lf", cfd->steer_prediction);

        /* Buffer for the steer adjust command string */ 
        char steer_com_string[200];
        memset(steer_com_string, '\0', 200);
        /* Building the string */
        strcat(steer_com_string, "python query_csac.py FA");
        strcat(steer_com_string, pred_string);

        /* Print warning message */
        fprintf(stderr,"CLOCK CONCISTENCY ALARM!\n");

        /* Acquiring lock on CSAC serial*/
        sem_wait(&(s_synch->csac_mutex));

        /* Disabling disciplining */
        run_command("python query_csac.py Md", program_buf);
        fprintf(stderr, "Disabling CSAC disciplining: [%s]\n", program_buf);
        memset(program_buf, '\0', 200);

        /* Adjusting frequency according to the models prediction */
        run_command(steer_com_string, program_buf);
        fprintf(stderr, "Setting steer value %lf: [%s]\n", cfd->steer_prediction,program_buf);

        /* Releasing lock on CSAC serial*/
        sem_post(&(s_synch->csac_mutex));
	}
    }

    /* Updating prediction if 24 hours has passed since the last update */
    if(cfd->new_day == 1) {

        /* Update prediction */
        update_prediction(cfd);

        /* Updating fast timing filter status */
        cfd->ftf_status = fast_timing_filter(cfd->phase_current, cfd->cf_conf.phase_limit);

        /* Updating frequency correction filter status */
        cfd->fqf_status = freq_cor_filter(cfd);

        /* Clearing new day variable*/
        cfd->new_day = 0;

        /* If logging is enabled, log steer predicted */
        if(cfd->cf_conf.pred_logging) {
            char log_output[200];
            memset(log_output, '\0', 200);
            snprintf(log_output, 100, "%lf\n", cfd->steer_prediction);
            log_to_file(cfd->cf_conf.pred_log_path, log_output, 1);
        }
    }
    return 1;
}

/* Setting up the config structure specific for the server */
static void initialize_config(struct config_map_entry *conf_map, struct csac_filter_config *cf_conf)
{
    conf_map[0].entry_name = CONFIG_PRED_LOG_PATH;
    conf_map[0].modifier = FORMAT_STRING;
    conf_map[0].destination = &cf_conf->pred_log_path;

    conf_map[1].entry_name = CONFIG_PRED_LOGGING;
    conf_map[1].modifier = FORMAT_INT;
    conf_map[1].destination = &cf_conf->pred_logging;

    conf_map[2].entry_name = CONFIG_CFD_PATH;
    conf_map[2].modifier = FORMAT_STRING;
    conf_map[2].destination = &cf_conf->cfd_log_path;

    conf_map[3].entry_name = CONFIG_INIT_FROM_FILE;
    conf_map[3].modifier = FORMAT_INT;
    conf_map[3].destination = &cf_conf->init_cfd_from_file;

    conf_map[4].entry_name = CONFIG_INIT_SSC;
    conf_map[4].modifier = FORMAT_DOUBLE;
    conf_map[4].destination = &cf_conf->init_cfd_ssc;

    conf_map[5].entry_name = CONFIG_INIT_SST;
    conf_map[5].modifier = FORMAT_DOUBLE;
    conf_map[5].destination = &cf_conf->init_cfd_sst;

    conf_map[6].entry_name = CONFIG_INIT_SSP;
    conf_map[6].modifier = FORMAT_DOUBLE;
    conf_map[6].destination = &cf_conf->init_cfd_ssp;

    conf_map[7].entry_name = CONFIG_PHASE_LIMIT;
    conf_map[7].modifier = FORMAT_DOUBLE;
    conf_map[7].destination = &cf_conf->phase_limit;

    conf_map[8].entry_name = CONFIG_STEER_LIMIT;
    conf_map[8].modifier = FORMAT_DOUBLE;
    conf_map[8].destination = &cf_conf->steer_limit;

    conf_map[9].entry_name = CONFIG_TIME_CONSTANT;
    conf_map[9].modifier = FORMAT_DOUBLE;
    conf_map[9].destination = &cf_conf->time_constant;

    conf_map[10].entry_name = CONFIG_WARMUP_DAYS;
    conf_map[10].modifier = FORMAT_INT;
    conf_map[10].destination = &cf_conf->warmup_days;

    conf_map[11].entry_name = CONFIG_INIT_SSY;
    conf_map[11].modifier = FORMAT_DOUBLE;
    conf_map[11].destination = &cf_conf->init_cfd_ssy;

    conf_map[12].entry_name = CONFIG_PRED_LIMIT;
    conf_map[12].modifier = FORMAT_DOUBLE;
    conf_map[12].destination = &cf_conf->pred_limit;
}

int start_csac_filter(struct csac_filter_data *cfd)
{
    /* Allocating buffer for run_program() */
    char program_buf[200];
    memset(program_buf, '\0', 200);

    int rc_status = 0;
    int filter_initialized = 0;

    /* csac_filter config */
    struct config_map_entry conf_map[CSAC_FILTER_CONFIG_ENTRIES];
    initialize_config(conf_map, &cfd->cf_conf);
    load_config(conf_map, CSAC_FILTER_CONFIG_PATH, CSAC_FILTER_CONFIG_ENTRIES);

    /* Keep going as long as the server is running */
    while(!done){ 
        /* Acquiring lock*/
        sem_wait(&(s_synch->csac_mutex));

        /* Querying CSAC */
        rc_status = run_command("python get_telemetry.py", program_buf);

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
        dump_cfd(cfd->cf_conf.cfd_log_path);

        sleep(0.5);
        memset(program_buf, '\0', 200);
    }
    return 0;
}
