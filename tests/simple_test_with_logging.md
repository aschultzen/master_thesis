# Spoofing simulation 26.08.2016
We wanted to know what kind of data we would collect if the GPS receivers where moved and packed in tin-foil. In this test we have two GPS receivers, one at the north end of the building and one at the south end. These two GPS receiver's data is collected in gps1.txt (north) and gps2.txt (south). It's worth mentioning that the southern GPS receiver is being used to discipline a CSAC. 

## Approach (or method)
- Downloaded and used an app called "Atomic Clock" for Android that parsed NMEA data from the phone's GPS receiver and presented time. This was used for time-stamping.
- Wrote down the actions performed and when.

## Times recorded
It proved to be challenging to time-stamp data within the second. The start of each test was usually pretty accurate, but the end was not.

		ACTIVITY			START 		END 	MJD_START			MJD_END
	==========================================================================
	EXPERIMENT START	= 11:00:01				57626.4583449	
	GPSNORTH WAVED		= 11:02:40 - 11:03:27	57626.4601852	57626.4607292
	GPSSOUTH WAVED		= 11:06:00 - 11:07:00	57626.4625		57626.4631944

	GPSNORTH_TINFOIL	= 11:08:59 - 11:09:59	57626.4645718	57626.4652662
	GPSSOUTH_TINFOIL	= 11:13:00 - 11:17:30   57626.4673611	57626.4704861

	GPSSOUTHONGPSNORTH	= 11:24:05 - 11:27:00	57626.4750579	57626.4770833

The MJD values where calculated using the following script:

	import datetime
	import jdutil
	from dateutil import parser

	# Copy paste date as string
	dt = parser.parse("2016-08-26 11:27:00")
	print(jdutil.jd_to_mjd(jdutil.datetime_to_jd(dt))) 
	print(dt)

