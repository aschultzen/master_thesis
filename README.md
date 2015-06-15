# Master thesis notes
These are some notes related to the master thesis. They are currently very general. 

## 15.05.2015: Installing and setting up the PI
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

The NTP config file at */etc/ntp.conf* was changed to the following:
 

	

