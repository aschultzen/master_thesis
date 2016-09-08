#include "csac_filter.h"

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

static float mjd_diff_day(double mjd_a, double mjd_b){
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

    if(!substring_extractor(13,14,',',buffer,100,telemetry,strlen(telemetry))) {
        printf("Failed to extract substring from CSAC data\n");
        return 0;
    } else {
        if(sscanf(buffer, "%lf", &cfd->phase_current) == EOF) {
            return 0;
        }
    }

    if(!substring_extractor(11,12,',',buffer,100,telemetry,strlen(telemetry))) {
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
            if(mjd_diff_day(mjd_today, cfd->today_mjd) >= 1 && cfd->t_current != 0) {
                cfd->new_day = 1;
                cfd->today_mjd = mjd_today;
                cfd->days_passed++;
                printf("MJD diff %lf %lf\n", cfd->t_current, mjd_today);
            }
            // Initializing today_mjd, only done once at startup
            if(cfd->today_mjd == 0){
            	cfd->today_mjd = mjd_today;
            	cfd->days_passed = 0;
            }
            // Updating running MJD
            cfd->t_current = mjd_today;
        }
    }*/

    /* Only used during testing */
    double mjd_today = 0;
    if(!substring_extractor(0,1,',',buffer,100,telemetry,strlen(telemetry))) {
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

	//printf("tsy: %lf, tst: %lf, ssy: %lf, sst: %lf ssc %lf\n", cfd->t_smooth_yesterday, cfd->t_smooth_today, cfd->steer_smooth_yesterday, cfd->steer_smooth_today, cfd->steer_smooth_current);
}
double get_steer_predict(struct csac_filter_data *cfd)
{
	if(cfd->days_passed >= warmup_days){
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
    int status = load_telemetry(cfd, telemetry);
    cfd->t_smooth_current = cfd->t_current;
    cfd->steer_smooth_current = cfd->steer_current;

    cfd->steer_smooth_today = cfd->steer_smooth_current;
    cfd->steer_smooth_previous = cfd->steer_smooth_today;
    cfd->t_smooth_today = cfd->t_smooth_current;
    cfd->t_smooth_yesterday = cfd->t_smooth_current -0.1;
    return status;
}

/* Update the filter with new data */
int update_csac_filter(struct csac_filter_data *cfd, char *telemetry)
{
	/* Load new telemetry into the filter */
	if(!load_telemetry(cfd, telemetry) ){
		printf("Failed to load telemetry into filter!\n");
		return 0;
	}

	/* Calculate smoothed values */
	calc_smooth(cfd);

	/* Updating prediciton if 24 hours has passed since the last update */
	if(cfd->new_day == 1) {
        cfd->new_day = 0;
        update_prediction(cfd);
    }
    return 1;
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

    FILE *csac_log;
    /* Opening CSAC log */
    csac_log = fopen(argv[1], "r");
    if (csac_log == NULL) {
        printf("Failed to load %s\n", argv[1]);
        return 0;
    }
    rewind(csac_log);

    char buffer[200];
    struct csac_filter_data *cfd = calloc(1, sizeof(struct csac_filter_data));

    /* init filter */
    get_csac_line(buffer, csac_log, 200);
    init_csac_filter(cfd, buffer);

    /* Update */
    while(get_csac_line(buffer,csac_log, 200)){
    	update_csac_filter(cfd, buffer);
    	printf("Steer prediction %lf\n",get_steer_predict(cfd));
    }

    free(cfd);
    fclose(csac_log);
    exit(0);
}