# Spoofing simulation 26.08.2016
We wanted to know what kind of data we would collect if the GPS receivers where moved and packed in tin-foil. In this test we have two GPS receivers, one at the north end of the building and one at the south end. These two GPS receiver's data is collected in gps1.txt (north) and gps2.txt (south). It's worth mentioning that the southern GPS receiver is being used to discipline a CSAC. 

## Approach (or method)
- Downloaded and used an app called "Atomic Clock" for Android that parsed NMEA data from the phone's GPS receiver and presented time. This was used for time-stamping.
- Wrote down the actions performed and when.

## Times recorded
It proved to be challenging to time-stamp data within the second. The start of each test was usually pretty accurate, but the end was not. Timezone was UTC.

		ACTIVITY		   START 		END 	  MJD_START		  MJD_END
	==========================================================================
	EXPERIMENT START	= 09:00:01				57626.3750116	
	GPSNORTH WAVED		= 09:02:40 - 09:03:27	57626.3768519	57626.3773958
	GPSSOUTH WAVED		= 09:06:00 - 09:07:00	57626.3791667	57626.3798611

	GPSNORTH_TINFOIL	= 09:08:59 - 09:09:59	57626.3798611	57626.3819329
	GPSSOUTH_TINFOIL	= 09:13:00 - 09:17:30   57626.3840278	57626.3871528

	GPSSOUTHONGPSNORTH	= 09:24:05 - 09:27:00	57626.3917245	57626.39375

The MJD values where calculated using the following script:

	import datetime
	import jdutil
	from dateutil import parser

	# Copy paste date as string
	dt = parser.parse("2016-08-26 11:27:00")
	print(jdutil.jd_to_mjd(jdutil.datetime_to_jd(dt))) 
	print(dt)

