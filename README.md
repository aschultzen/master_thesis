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

I downloaded the latest image here: [Raspbian Download]: http://downloads.raspberrypi.org/raspbian_latest 






	

