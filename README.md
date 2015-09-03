[rpirb]: https://github.com/aschultzen/master_thesis/blob/master/pics/raspberry_pi_gps_rubber_bands.png "Raspi GPS Rubber Bands"
[schm_rpirb]: https://github.com/aschultzen/master_thesis/blob/master/pics/raspi_gps.png "Raspi GPS Schematic"

# Notes

## 3.09.2015 rtklib, Raspi and Ublox Part 3
Turned out i just had mixed up RX and TX. Swapped them around and the following:

	cat /dev/ttyAMA0

Produced:
	
	$GPRMC,143809.00,A,5957.80441,N,01043.83609,E,0.152,,030915,,,D*7F
	$GPVTG,,T,,M,0.152,N,0.281,K,D*2B
	$GPGGA,143809.00,5957.80441,N,01043.83609,E,2,11,0.86,185.6,M,38.5,M,,0000*57
	$GPGSA,A,3,14,18,27,19,28,32,22,08,04,01,11,,1.50,0.86,1.23*0F
	$GPGSV,4,1,16,01,21,263,17,04,48,258,34,08,69,239,35,11,34,272,34*76
	$GPGSV,4,2,16,14,18,122,19,15,06,017,,18,32,060,29,19,66,265,35*73
	$GPGSV,4,3,16,21,01,089,,22,61,101,27,24,00,050,,27,48,160,25*72
	$GPGSV,4,4,16,28,20,328,30,30,04,297,,32,15,204,31,33,18,209,36*71
	$GPGLL,5957.80441,N,01043.83609,E,143809.00,A,D*6E

The U-blox hasn't got a lock on the satellites yet, but the communication works. At this point the setup looks like this:
![alt text][rpirb]
Yeah, i know. It's not pretty but at least i can transport it somehow. I use some really cheap leads i ordered of Ebay a long time a go, it's a long story. Anyway, the schematics:
![alt text][schm_rpirb]

## 2.09.2015 rtklib, Raspi and Ublox Part 2
Picking things up where i left them last time.
<s>Installed *minibian* (https://minibianpi.wordpress.com/) on my Raspi 1 that i had laying around.

	dd bs=4M if=2015-02-18-minibian.img of=/dev/sdd

Update and upgraded and restarted the box.</s>
NOTE: Something happened when i updated and upgraded using miniban. Falling back to Raspbian. The procedure is the same though.

After i had a clean box with Raspbian, i downloaded the rtklib source:

	wget http://www.rtklib.com/prog/rtklib_2.4.2.zip
	unzip rtklib_2.4.2.zip

Now standing in:
	
	~/rtklib_2.4.2/app/rtkrcv/gcc
	make

Now, the compiling took a good while, so you might as well get some coffee. I got a couple of warnings but no errors.

*PRO-TIP: Thomas Yan @ GNSS Corner (http://gnss.co/?p=52) suggest modifying the makefile with:* 

	CTARGET = -march=armv6 -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -ffast-math

*I haven't tried it, usually GCC is way better than me anyway.*

Rtkrcv can be started with:

	./rtkrvc

*PRO-TIP: Use 'shutdown' to exit. You'll thank me later*

Using **raspi-config** i disabled login over serial. Raspbian will by default use the serial for console, so it needs to be disabled. 

	8 Advanced Options
	A8 Serial Options

**The GPIO serial port is /dev/ttyAMA0.** At this point i have not yet been able to receive any data from the U-blox chip. The PPS led is blinking like it's supposed to, so it might be an issue with the wiring or setup. The U-blox chip should output empty NMEA data even without satellite lock.

## 31.08.2015 rtklib, Raspi and Ublox
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

The same source states the rtklib can be used with all u-blox receivers, so that's worth looking into. From what I've gathered, the U-blox 6M can be configured to transmit raw data over serial that can be used by rtklib. The price difference between the 6M and the 6T isn't really big, i found them priced at about ~100NOK and ~300NOK respectively (at ebay from china). The 6T actually came with an "antenna assembly" which looked pretty good. 

## 28.08.2015
Had a meeting with Harald at Justervesenet. I need to check out the following:

- rtklib for the Ublox
- Com with CSAC, build a layer that implements the "CSAC" protocol.