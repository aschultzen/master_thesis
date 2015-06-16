# Master thesis notes
The following should be considered a log and notes from my summer internship at Justervesenet. They are however highly related to my master thesis.

## 15.06.2015: Installing and setting up the PI : Part 1
I had some issues finding the PI after it was connected to the network. By doing a simple scan

	nmap -sn -v 10.1.1.1/23

before the pie was connected and one after, the PI's IP was discovered by *diffing* the two. After the PI was up, i updated it and installed *ntpdate*:

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

## 16.06.2015 Installing and setting up the PI : Part 2

I discovered that i had done a couple of things wrong yesterday, i will therefore attempt to correct my mistakes today and test my instructions from yesterday on a fresh install of Raspbian. 

NOTE: It's worth mentioning that these instructions probably will work on other flavors of Linux as well, Raspbian is quite big and has functionality that an NTP server will never benefit from (like a GUI). There are for example a couple of stripped down versions of Raspbian containing only the server essentials with a footprint > 100 MB. 

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

I chose to expand the filesystem to utilize all the storage space available on the SD card and i also disabled SSH over serial since we are planning to use the serial to control the CSAC later on.




 






	

