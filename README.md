# Notes

# 2.09.2015

## rtklib, Raspi and Ublox Part 2
Picking things where i left last time.
~~Installed *minibian* (https://minibianpi.wordpress.com/) on my Raspi 1 that i had laying around.

	dd bs=4M if=2015-02-18-minibian.img of=/dev/sdd

Update and upgraded and restarted the box.~~
NOTE: Something happened when i updated and upgraded using miniban. Falling back to Raspbian. The procedure is the same though.

After i had a clean box with Raspbian, i downloaded the rtklib source:

	wget http://www.rtklib.com/prog/rtklib_2.4.2.zip
	unzip rtklib_2.4.2.zip

And compiled it with:

	make

standing in:
	
	~/rtklib_2.4.2/app/rtkrcv/gcc

Now, this took a good while, so might as well get some coffee. I got a couple of warnings but no errors.

PRO-TIP: Thomas Yan @ GNSS Corner (http://gnss.co/?p=52) suggest modifying the makefile with: 

	CTARGET = -march=armv6 -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -ffast-math

I haven't tried it, usually GCC is way better than me anyway. 

	



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

The same source states the rtklib can be used with all u-blox receivers, so that's worth looking into. From what I've gathered, the U-blox 6M can be configured to transmit raw data over serial that can be used by rtklib. The price difference between the 6M and the 6T isn't really big, i found them priced at about ~100NOK and ~300NOK respectively (at ebay from china). The 6T actually came with an "antenna assembly" which looked pretty good. 

# 28.08.2015
Had a meeting with Harald at Justervesenet. I need to check out the following:

- rtklib for the Ublox
- Com with CSAC, build a layer that implements the "CSAC" protocol.






