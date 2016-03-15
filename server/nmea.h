#ifndef NMEA_H
#define NMEA_H

/* NMEA SENTENCES */
#define GGA "$GNGGA"
#define GSA "$GNGSA"
#define RMC "$GNRMC"
#define SENTENCE_LENGTH 100

/* NMEA SENTENCES DELIMITER POSITIONS */
#define ALTITUDE_START 9
#define LATITUDE_START 3
#define LONGITUDE_START 5
#define RMC_TIME_START 1

#define SAFE 0
#define HIGH 1
#define LOW -1

struct nmea_container{
	/* Raw data */
	char raw_gga[SENTENCE_LENGTH];
	char raw_rmc[SENTENCE_LENGTH];

	/* Latitude */
	double lat_low;
	double lat_high;
	double lat_current;
	int lat_disturbed;

	/* Longitude */
	double lon_low;
	double lon_high;
	double lon_current;
	int lon_disturbed;

	/* Altitude */
	double alt_low;
	double alt_high;
	double alt_current;
	int alt_disturbed;
};

#endif /* !NMEA_H */