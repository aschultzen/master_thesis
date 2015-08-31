# Notes

# 28.08.2015
Had a meeting with Harald at Justervesenet. I need to check out the following:

- rtklib for the Ublox
- Com with CSAC, build a layer that implements the "CSAC" protocol.

# 31.08.2015

## rtklib, Raspi and Ublox
Just discovered that rtklib is easy to compile and run for ARM architectures. rtklib contains a GUI environment that only works with Windows, but this does not matter for me. I also discovered that rtklib only (officially) support the following GNSS chips. 

	Brand 		Device 		Supported frequencies 		Approximate price
	SkyTraq 	S1315F 		Single frequency GPS 		€100[33]
	SkyTraq 	Venus 8 	Single frequency GPS 		$80[34]
	NVS 		NV08C BINR 	Single freq and GLONASS 	€146,40[35][36]
	U-blox 		LEA-6T 		Single frequency GPS 		$349[37] €295[38]
	U-blox 		LEA-5T 		Single frequency GPS 		No longer available
	U-blox 		LEA-4T 		Single frequency GPS 		No longer available
	U-blox 		NEO-6T 		Single frequency GPS 		€140[39]
	U-blox 		NEO-6P 		Single frequency GPS  		€140[40]
	U-blox 		EVK-6P 		Single frequency GPS  	
	U-blox 		EVK-7P 		Single frequency GPS  		250,€+
	Furuno 		GW-10 II/III
	Hemisphere 	Eclipse 	DualGPS, GLONASS,Galileo 	From $1125[41]
	Hemisphere 	Crescent 	Single frequency GPS 	
	JAVAD 		Multiple 	Multiple frequencies 		From $1500
	NovAtel 	OEM4/V/6, OEM3, OEMStar, Superstar II
source(http://wiki.openstreetmap.org/wiki/RTKLIB)
The same source states the rtklib can be used with all ublox receivers, so that's worth looking into.




