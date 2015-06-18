# Master thesis notes
The following should be considered a log and notes from my summer internship at Justervesenet. They are however highly related to my master thesis. 

Complete ntp config files can be found at the bottom of this document.

## Prerequisite information
Workstation:

	CPU: Core2Duo E7500
	Windows 7 64 Bit
	Generic HW

## 15.06.2015: Installing and setting up the PI : Part 1
I had some issues finding the PI after it was connected to the network. By doing a simple scan

	nmap -sn -v 10.1.1.1/23

before the pie was connected and one after, the PI's IP was discovered by *diffing* the two. <s>After the PI was up, i updated it and installed *ntpdate*:

	sudo apt-get update
	sudo apt-get upgrade
	sudo apt-get dist-upgrade
	sudo apt-get install ntpdate

After the updates where finished, the local time zone was selected:

	sudp dpkg-reconfigure tzdata

With *ntpdate*, the time could by synchronized by force with:

	ntpdate 10.1.1.60

The address *10.1.1.60* is to one of the NTP servers at Justervesenet. 

The NTP config file at */etc/ntp.conf* was changed:

	server 10.1.1.60 minpoll 4 iburst
	fudge 127.0.0.1 stratum 10

After the config was changed, the *ntpd* service was restarted to apply the changes done in the config file:

	sudo /etc/init.d/ntp restart

</s>

A lot of errors where made this day. Hence the striking.

## 16.06.2015 Installing and setting up the PI : Part 2

I discovered that i had done a couple of things wrong yesterday, i will therefore attempt to correct my mistakes today and test my instructions on a fresh install of Raspbian. 

NOTE: It's worth mentioning that these instructions probably will work on other flavors of Linux as well, Raspbian is quite big and has functionality that a NTP server will never benefit from (like a GUI). There are for example a couple of stripped down versions of Raspbian containing only the server essentials with a footprint > 100 MB. 

I downloaded the latest image here: [Raspbian Download]: http://downloads.raspberrypi.org/raspbian_latest and prepared to copy it to the SD card. After the SD was inserted, the device can found by using:

	dmesg

which should output a lot of (for this application) useless information, but also info about the SD card. On my computer, i found the following output:

	[ 8957.378542] scsi 6:0:0:0: Direct-Access     Generic- Multi-Card       1.00 PQ: 0 ANSI: 0 CCS
	[ 8957.379931] sd 6:0:0:0: Attached scsi generic sg1 type 0
	[ 8958.093824] sd 6:0:0:0: [sdb] 15415296 512-byte logical blocks: (7.89 GB/7.35 GiB)

This shows that the systems has mounted the device as *sdb*. This important for the next steps. The image file is probably compressed as a zip, in most linux distros, the file can be unzipped with *unzip*:

	unzip 2015-05-05-raspbian-wheezy.zip

Once the file is unzipped and ready, the copying can begin:

	sudo dd bs=4M if=2015-05-05-raspbian-wheezy.img of=/dev/sdb

Once this process has started, it might use some time. Be patient. 

NOTE: If the block size of 4M for some reason fails, change it to 1M. This will however make the copy process a lot slower.

If everything went according to plan, you should see a similar message:

	781+1 records in
	781+1 records out
	3276800000 bytes (3,3 GB) copied, 566,728 s, 5,8 MB/s

Once the SD card is popped into the RASPI, the Ethernet and power cables are connected, you have two options:

- Connect a monitor to the HDMI port
- Use SSH and manage it remotely over LAN

The latter, is usually the easiest. The RASPI has DHCP enabled by default, this means it might be a little hassle establishing the address it has been leased by the DHCP controller. The subnet i connected mine to, contained 510 hosts, so i used to approach as described under *15.06.2015*. 

Once the IP address has been found, an SSH connection can be made to the device. The default login credentials are:

	Username: pi
	Password: raspberry

Raspbian includes a tool to do some rudimentary configuration in a very easy way, it can be launched with:

	sudo raspi-config

I chose to expand the filesystem to utilize all the storage space available on the SD card and i also disabled SSH over serial since we are planning to use the serial to control the CSAC later on. Once the RASPI has rebooted, i updated the system entirely:

	sudo apt-get update
	sudo apt-get dist-upgrade
	sudo rpi-update
	sudo reboot

Set the correct locale:

	sudp dpkg-reconfigure tzdata

We now have a RASPI that is ready to go.

### Installing NTP

Since we plan to use the CSAC for 1 PPS, we need to update NTP since neither the current one installed or the one in the Debian repository supports PPS. The source files for the latest release of NTP can be downloaded directly:

	wget http://archive.ntp.org/ntp4/ntp-4.2.8p2.tar.gz
	wget http://archive.ntp.org/ntp4/ntp-4.2.8p2.tar.gz.md5

I also like to check the MD5 hashes to verify the integrity of the files i have downloaded:

	md5sum -c ntp-4.2.8p2.tar.gz.md5

The check should result in the following:

	ntp-4.2.8p2.tar.gz: OK


Since we plan on using the CSAC we need some additional packages installed:

	sudo apt-get install pps-tools
	sudo apt-get install libcap-dev

Extract the source files, compile and install them:


	tar zxvf ntp-4.2.8p2.tar.gz
	cd ntp-4.2.8p2/
	./configure -enable-linuxcaps
	make
	sudo make install
	sudo service ntp stop

At this point, the NTP server should be a config file away from running. We still have not set up the 1PPS from the CSAC, but we will get there later. In */etc/ntp.conf* comment out (or delete) any line specifying a time broadcast server on the local subnet, in my case to this:

	# Generated by dhcpcd from eth0
	#server 10.1.1.251
	# End of dhcpcd from eth0

This is generated by some config files that should be removed while you're at it. If you don't, it will revert to the default setting every time you reboot the RASPI:

	rm /etc/dhcp/dhclient-exit-hooks.d/ntp
	rm /var/lib/ntp/ntp.conf.dhcp

NOTE: Don't fret if one of the files don't exist on the system. If they are gone, they are gone, and that's good.

In the end, we really need some servers to synchronize with. I added the following lines specifying some of the NTP servers at Justervesenet. If you do not have access to these, you should use other servers close to you. [NTP Pool project]: www.pool.ntp.org should help you get started.

	server 10.1.1.58 minpoll 4 iburst prefer
	server 10.1.1.59 minpoll 4 iburst
	server 10.1.1.60 minpoll 4 iburst
	server 10.1.1.61 minpoll 4 iburst
	server 10.1.1.60 minpoll 4 iburst
	server 127.127.1.0
	fudge 127.127.1.0 stratum 10

I also changed the restrictions somewhat to allow clients to query our server. This can be done by changing:

	#restrict -4 default kod notrap nomodify nopeer noquery

to:

	restrict -4 default kod notrap nomodify nopeer

Restart the service:

	sudo service ntp restart

Run the following command to query the NTP server locally:

	ntpq -p

It should produce output similar to the following:

	     remote           refid      st t when poll reach   delay   offset  jitter
	==============================================================================
	*10.1.1.58       .PPS.            1 u   54  128  377    0.406    0.019   0.016
	+10.1.1.59       .PPS.            1 u   63  128  377    0.403   -0.058   0.025
	-10.1.1.60       .PPS.            1 u   57  128  377    0.336    0.026   0.019
	+10.1.1.61       .GPS.            1 u    4  128  377    0.384   -0.054   0.005
	 LOCAL(0)        .LOCL.          10 l    -   64    0    0.000    0.000   0.000

## 17.06.2015: Setting up the CSAC

The CSAC (as previously mentioned) will be used as a 1 PPS input to the NTP server. The CSAC can be controlled over serial with software supplied by Symmetricom (CSACdemo) or with simple commands as specified in the documentation over telnet. After connecting the CSAC to a workstation running Winodws 7, the COM driver was configured after Symmetricom's specifications as followed:

- 57600 Baud
- 8 Data bits
- No parity
- 1 stop bit (8-N-1)
- No flow control
- CMOS Voltage levels (0-3.3V) -> (Not configurable, but worth noting.)

With the exception of the baud rate, all of these settings were set correctly by default on my workstation.

NOTE: We didn't need to worry about the RS232's +/- 12V logic level since the evaluation board employs a level shifter. **Without the level shifter you run the risk of frying the serial interface on the CSAC.**  

PROBLEM 1: At this point i was not able to establish communication with the CSAC. It does not respond when connected to the workstation. I've tried both the CSACDemo and Realterm. Current theory is that the cable is broken.

SOLUTION 1: The cable we tried to use for to communicate turned out to be an extension cable. I suppose it is self explanatory, but the cable that worked was *null modem* cable. Symmetricom actually recommend that people use the provided cable in order to avoid confusion like this.

##18.06.2015: Dealing with leap seconds, testing cables.
In order for NTP to deal with leap seconds gracefully, NIST has released a leap second file containing a table of both past and upcomming leap seconds. This file can be used by *ntpd* to apply the leap second locally at an appropriate time rather than having the clients noticing the error and correcting it when they detect it. Mirrors for this file can be found at NIST [NIST:Configuring NTP]: http://support.ntp.org/bin/view/Support/ConfiguringNTP#Section_6.14 though some of them where outdated when i tried to download it. 

NOTE: The name of the leap second file name is changed whenever it gets updated. Depending on how old this guide is when you read it, the filename used in the description below might be wrong.

	sudo mkdir /var/ntp && cd /var/ntp
	wget ftp://tycho.usno.navy.mil/pub/ntp/leap-seconds.3629577600

Once the leap second file is downloaded and copied, changes has to be made to */etc/ntp.conf*:

	leapfile /etc/leap-seconds.3629577600

Restart the service:

	sudo service ntp restart
	
###Complete /etc/ntp.conf for Stratum 2 (Without the CSAC/PPS)

	# /etc/ntp.conf, configuration for ntpd; see ntp.conf(5) for help
	
	driftfile /var/lib/ntp/ntp.drift
	
	# Enable this if you want statistics to be logged.
	#statsdir /var/log/ntpstats/
	
	# Comment this out if you dont have a leap second file.
	leapfile /etc/var/leap-seconds.3629577600
	
	statistics loopstats peerstats clockstats
	filegen loopstats file loopstats type day enable
	filegen peerstats file peerstats type day enable
	filegen clockstats file clockstats type day enable
	
	server 10.1.1.58 minpoll 4 iburst prefer
	server 10.1.1.59 minpoll 4 iburst
	server 10.1.1.60 minpoll 4 iburst
	server 10.1.1.61 minpoll 4 iburst
	server 10.1.1.60 minpoll 4 iburst
	server 127.127.1.0
	fudge 127.127.1.0 stratum 10
	
	# Access control configuration; see /usr/share/doc/ntp-doc/html/accopt.html for
	# details.  The web page <http://support.ntp.org/bin/view/Support/AccessRestrictions>
	# might also be helpful.
	#
	# Note that "restrict" applies to both servers and clients, so a configuration
	# that might be intended to block requests from certain clients could also end
	# up blocking replies from your own upstream servers.
	
	# By default, exchange time with everybody, but don't allow configuration.
	#restrict -4 default kod notrap nomodify nopeer noquery
	restrict -6 default kod notrap nomodify nopeer noquery
	
	# Local users may interrogate the ntp server more closely.
	restrict 127.0.0.1
	restrict ::1
	
	# Clients from this (example!) subnet have unlimited access, but only if
	# cryptographically authenticated.
	#restrict 192.168.123.0 mask 255.255.255.0 notrust
	
	# If you want to provide time to your local subnet, change the next line.
	# (Again, the address is an example only.)
	#broadcast 192.168.123.255
	
	








 






	

