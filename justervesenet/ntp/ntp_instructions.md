# Instructions for Raspberry PI Stratum 1 server

**If you are starting from scratch with a completely blank SD card, start here. If you have bought a SD card with Raspbian installed, go to *Login and Update*. If you are using a pre-configured image (**INSERT FILENAME**), go to *IP Setup*.**

## Tools
**(If you plan to use linux for the setup, you can probably ignore this part of the guide.)** I've used *putty* (windows application) for SSH connections. Putty supports copy/paste by copying normally in windows and pasting into putty by right-clicking in putty's windows. I also like to use *nano* to edit files. You can open files with nano like this:

	sudo nano /example_dir/example_file


## From scratch

NOTE: It's worth mentioning that these instructions probably will work on other flavors of Linux as well, Raspbian is quite big and has functionality that a NTP server will never benefit from (like a GUI). There are for example a couple of stripped down versions of Raspbian containing only the server essentials with a footprint > 100 MB. 

**The following instructions are meant for a empty SD card.**

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

Once this process has started, it might take some time. 

NOTE: If the block size of 4M for some reason fails, change it to 1M. This will however make the copy process a lot slower.

If everything went according to plan, you should see similar output:

	781+1 records in
	781+1 records out
	3276800000 bytes (3,3 GB) copied, 566,728 s, 5,8 MB/s

Pop the SD card into the RASPI and connect the power. 

## Login and Update

Once the SD card is popped into the RASPI, the Ethernet and power cables are connected, you have two options:

- Connect a monitor to the HDMI port 
- Use SSH and manage it remotely over LAN

Depending on what you have at hand (i preferred the NMAP way, but i didn't have access to a HDMI cable) choose a way to determine the IP:
### The NMAP way

 The RASPI has DHCP enabled by default, this means it might be a little hassle establishing the address it has been leased by the DHCP controller. The subnet i connected mine to, contained 510 hosts, so i used NMAP to scan the subnet with the following command (**Substitute the IPs in the example with ones from your own subnet!**):

	nmap -sn -v 10.1.1.1/23

By scanning the subnet right before you connect the Raspberry PI and once after it has been connected (and booted), you can compare the two results and find the difference. The Raspberry PI's IP will be that difference.

NOTE: On a busy subnet where devices are connected and disconnected often, the Raspberry Pi might not be the only change you detect, but it still narrows down the search dramaticly.

### The HDMI way

Once the IP address has been found, a SSH connection can be made to the device. The default login credentials are:

	Username: pi
	Password: raspberry

Raspbian includes a tool to do some rudimentary configuration in a very easy way, it can be launched with:

	sudo raspi-config

This opens a menu. I chose to expand the file-system to utilize all the storage space available on the SD card and i also disabled SSH over serial since we are planning to use the serial to control the CSAC later on. The latter is *8 Advanced Options* -> *A8 Serial*.

NOTE: The expansion feature might fail when using SD cards with pre-installed versions of Raspbian (NOOBS).

Once the RASPI has rebooted, i updated the system entirely (the upgrade process is going to take some time):

	sudo apt-get update
	sudo apt-get dist-upgrade
	sudo rpi-update

Set the correct locale and reboot:

	sudp dpkg-reconfigure tzdata
	sudo reboot

NOTE: **Every time you reboot the Raspberry PI, the SSH connections is lost and needs to be re-established after boot.** 

## Setup PPS
We first need to install some packages for our PPS setup to work:

	sudo apt-get install pps-tools
	sudo apt-get install libcap-dev

We also need to add the following line to */boot/config.txt*:
	
	dtoverlay=pps-gpio,gpiopin=18

NOTE: Here we specify at what line we are planning to use to receive the 1 PPS from the reference clock.

A line also needs to be added to */etc/modules*:

	pps-gpio

Reboot:

	sudo reboot

Depending on your source for the 1 PPS, you might have to fabricate a cable in order to connect the Raspberry Pi to GPIO pin 18 and ground. The CSAC that i used, had a 1 PPS OUT with a SMA connector. This is pretty straight forward, but beware that there is limited space in the Raspberry Pi's enclosure.

### Verifying PPS setup

Before you go any further, it is recommended to check that the PPS setup acutally works. Use the following command:

	dmesg | grep pps

This should result in the following output (or similar):

	[    3.252888] pps_core: LinuxPPS API ver. 1 registered
	[    3.257520] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
	[    3.328702] pps pps0: new PPS source pps.-1
	[    3.331458] pps pps0: Registered IRQ 498 as PPS source

At this point the PPS is registered in the system, it might not receive a pulse, but it is ready. To check for a pulse use the following command:

	sudo ppstest /dev/pps0

Thkis should result in output similar to this:

	trying PPS source "/dev/pps0"
	found PPS source "/dev/pps0"
	ok, found 1 source(s), now start fetching data...
	source 0 - assert 1434703831.155754060, sequence: 6971 - clear  0.000000000, sequence: 0
	source 0 - assert 1434703832.155752877, sequence: 6972 - clear  0.000000000, sequence: 0

We are now ready for the NTP install.
## Installing NTP

Since we plan to use the reference clock for 1 PPS, we need to update NTP since neither the current one installed or the one in the Debian repository supports PPS. The source files for the latest release of NTP can be downloaded directly:

	wget http://archive.ntp.org/ntp4/ntp-4.2.8p2.tar.gz
	wget http://archive.ntp.org/ntp4/ntp-4.2.8p2.tar.gz.md5

I also like to check the MD5 hashes to verify the integrity of the files i have downloaded:

	md5sum -c ntp-4.2.8p2.tar.gz.md5

The check should result in the following:

	ntp-4.2.8p2.tar.gz: OK

Extract the source files, compile and install them:

	tar zxvf ntp-4.2.8p2.tar.gz
	cd ntp-4.2.8p2/
	./configure -enable-linuxcaps --enable-ATOM
	make
	sudo make install
	sudo cp /usr/local/bin/ntp* /usr/bin/  && sudo cp /usr/local/sbin/ntp* /usr/sbin/
	sudo service ntp stop

Issue the following commands to make sure that the NTP server uses only your specified servers and not the one used by the DHCP server.

	rm /etc/dhcp/dhclient-exit-hooks.d/ntp
	rm /var/lib/ntp/ntp.conf.dhcp

As well as changing *dhclient.conf*:

	sudo nano /etc/dhcp/dhclient.conf

...to make sure it stops requesting ntp servers from the DHCP controller. The "request" part should look like this:

	request subnet-mask, broadcast-address, time-offset, routers,
        domain-name, domain-name-servers, domain-search, host-name,
        dhcp6.name-servers, dhcp6.domain-search,
        netbios-name-servers, netbios-scope, interface-mtu,
        rfc3442-classless-static-routes;
	
In order for NTP to deal with leap seconds gracefully, NIST has released a leap second file containing a table of both past and upcomming leap seconds. This file can be used by *ntpd* to apply the leap second locally at an appropriate time rather than having the clients noticing the error and correcting it when they detect it. Mirrors for this file can be found at NIST [NIST:Configuring NTP]: http://support.ntp.org/bin/view/Support/ConfiguringNTP#Section_6.14 though some of them where outdated when i tried to download it. 

NOTE: The name of the leap second file name is changed whenever it gets updated. Depending on how old this guide is when you read it, the filename used in the description below might be wrong.

	sudo mkdir /var/ntp && cd /var/ntp
	sudo wget ftp://tycho.usno.navy.mil/pub/ntp/leap-seconds.3629577600

Replace the contents of */etc/ntp.conf* with the following:

	/etc/ntp.conf, configuration for ntpd; see ntp.conf(5) for help
	
	driftfile /var/lib/ntp/ntp.drift
	
	# Enable this if you want statistics to be logged.
	#statsdir /var/log/ntpstats/
	
	leapfile /etc/leap-seconds.3629577600
	
	statistics loopstats peerstats clockstats
	filegen loopstats file loopstats type day enable
	filegen peerstats file peerstats type day enable
	filegen clockstats file clockstats type day enable
	
	#server 127.127.1.0
	#fudge 127.127.1.0 stratum 10
	
	server 127.127.22.0 minpoll 4 maxpoll 4
	fudge 127.127.22.0 refid PPS
	fudge 127.127.22.0 flag 3 1
	
	server 10.1.1.58 minpoll 4 maxpoll 4 iburst prefer
	server 10.1.1.59 minpoll 4 maxpoll 4 iburst
	server 10.1.1.60 minpoll 4 maxpoll 4 iburst
	server 10.1.1.61 minpoll 4 maxpoll 4 iburst
	
	# By default, exchange time with everybody, but don't allow configuration.
	restrict -4 default kod notrap nomodify nopeer
	restrict -6 default kod notrap nomodify nopeer noquery
	
	# Local users may interrogate the ntp server more closely.
	restrict 127.0.0.1
	restrict ::1

Restart the service:

	sudo service ntp restart

Run the following command to query the NTP server:

	ntpq -pcrv

You should get something like this:

	 PPS(0)          .PPS.            0 l   11   16    1    0.000  -155.76   0.004
	*10.1.1.58       .PPS.            1 u    8   16    3    0.320   -0.023   0.014
	+10.1.1.59       .PPS.            1 u   12   16    3    0.327   -0.067   0.015
	+10.1.1.60       .PPS.            1 u   16   16    3    0.337    0.022   0.011
	 10.1.1.61       .PPS.            1 u    2   16    7    0.334    0.007   0.009
	associd=0 status=0619 leap_none, sync_ntp, 1 event, leap_armed,
	version="ntpd 4.2.8p2@1.3265 Fri Jun 19 15:15:04 UTC 2015 (5)",
	processor="armv7l", system="Linux/3.18.14-v7+", leap=00, stratum=2,
	precision=-18, rootdelay=0.320, rootdisp=3939.320, refid=10.1.1.58,
	reftime=d92eb5b3.8613fddd  Fri, Jun 19 2015 17:37:23.523,
	clock=d92eb5df.e02784ae  Fri, Jun 19 2015 17:38:07.875, peer=1657, tc=4,
	mintc=3, offset=-0.023357, frequency=-6.015, sys_jitter=0.014451,
	clk_jitter=0.009, clk_wander=0.000, tai=35, leapsec=201507010000,
	expire=201512010000

In the output we can observe the following:
- The version is 4.2.8p2@1.3265 compiled at Fri Jun 19 15:15:04 UTC 2015 (5)
- It is leap armed
- PPS is working (first line). 

That's it!

## IP Setup
Though DHCP is quite allright, you probably want to set up your Raspberry Pi with a static IP. Just copy and paste the following config file into */etc/networking/interfaces* and change the IP addresses to something available in your subnet (when in doubt, ask the IT guy):

	auto lo
	iface lo inet loopback
	
	iface eth0 inet static
	        address 10.1.1.62
	        netmask 255.255.254.0
	        gateway 10.1.1.19
	        network 10.1.0.0/23     #Optional
	        broadcast 10.1.1.255    #Also optional
	        
This is also a good time to change the host name:

	sudo nano /etc/hosts

Change the following line: 

	127.0.1.1	raspberrypi

to whatever name you have chosen to use. 

NOTE: The line mentioned over might contain some other name. It does not matter. Also, at this point, the system might start nagging about not being able to resolve the host name. Just ignore it and soldier on. 

Do the same in *hostname*:

	sudo nano /etc/hostname

No IP goes here. Only the host name.

Do a reboot:
	
	sudo reboot

**PRO-TIP:** Remember to connect using the new address instead of the old!

**DID YOU KNOW?:** We all know that 127.0.0.1 is localhost, however, in Debian there is a bug [Debian Bug #719621] https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=719621 for which there was made a workaround that makes the hostname resolve to 127.0.1.1 and localhost 127.0.0.1
	

	
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
	
	
### Lates config file with CSAC

	/etc/ntp.conf, configuration for ntpd; see ntp.conf(5) for help
	
	driftfile /var/lib/ntp/ntp.drift
	
	# Enable this if you want statistics to be logged.
	#statsdir /var/log/ntpstats/
	
	leapfile /etc/leap-seconds.3629577600
	
	statistics loopstats peerstats clockstats
	filegen loopstats file loopstats type day enable
	filegen peerstats file peerstats type day enable
	filegen clockstats file clockstats type day enable
	
	#server 127.127.1.0
	#fudge 127.127.1.0 stratum 10
	
	server 127.127.22.0 minpoll 4 maxpoll 4
	fudge 127.127.22.0 refid PPS
	fudge 127.127.22.0 flag 3 1
	
	server 10.1.1.58 minpoll 4 maxpoll 4 iburst prefer
	server 10.1.1.59 minpoll 4 maxpoll 4 iburst
	server 10.1.1.60 minpoll 4 maxpoll 4 iburst
	server 10.1.1.61 minpoll 4 maxpoll 4 iburst
	
	# By default, exchange time with everybody, but don't allow configuration.
	restrict -4 default kod notrap nomodify nopeer
	restrict -6 default kod notrap nomodify nopeer noquery
	
	# Local users may interrogate the ntp server more closely.
	restrict 127.0.0.1
	restrict ::1



	
	








 






	

