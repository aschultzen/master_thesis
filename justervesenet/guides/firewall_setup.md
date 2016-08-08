# Firewall Setup

## Introduction
This is as guide on how to build a Linux firewall from "scratch" using free and open-source software. Usability will make precedence over functionality as the firewall will be administrated by someone with no knowledge of iptables.

## The box
- Dell Optiplex G740 ~2008
- AMD Dual Core (Athlon 64) 
- 4 GB of RAM
- 160 GB HDD 
- NIC 1: On motherboard: Broadcom BCM5754 
- NIC 2: PCI: Intel 82541PI
- Ubuntu 16.04 LTS (Kernel 4.4.0-21-generic)


## The plan
The plan is as following:

- Firewall in Layer 2 (transparent)
- 3 NICs (outside, inside and management)
- Simple Iptables rules:
	- Security first approach.

## Log 17.06.2016

### Hang at boot
The server hangs at boot trying to raise one of the NICs. I've configured both the NICs to use DHCP, so I'm guessing it is waiting for me to plug in a cable into the unused network card. I can configure the "Raise network interfaces" like this:

	sudo nano /etc/systemd/system/network-online.targets.wants/networking.service

and change the line

	TimeoutStartSec=5min

to

	TimeoutStartSec=30sec

Which should still be more than enough time. Make sure to reload the daemon as well:

	sudo systemctl daemon-reload

### Installing packets
It's better to install the packets you need right away before you start configuring. While tweaking the config or firewall rules, you are going to loose connection to the Internet. Anyway, i installed the following packets:

	sudo apt-get install bridge_utils traceroute tshark nmap


### Bridge interface configuration
The interfaces where configured manually using nano:

	sudo nano /etc/network/interfaces

The configuration for my setup:

	auto enp2s0
	iface enp2s0 inet manual
		up ifconfig enp2s0 promisc up
		down ifconfig enp2s0 promisc down

	auto enp4s8
	iface enp4s8 inet manual
		up ifconfig enp4s8 promisc up
		down ifconfig enp4s8 promisc down

	auto br0
	iface br0 inet manual
		bridge_ports enp2s0 enp4s8

The config can be applied by either rebooting:

	sudo reboot now

or restarting the network service:

	sudo /etc/init.d/networking restart

There is nothing fancy here. On my machine, the enp2s0 is the internal NIC on the motherboard and the enp4s8 is the card installed in one of PCI ports. A couple of things though: 

- The *auto* keyword means that the interface is brought up automatically at start up. Both physical cards are brought up before anything is done with the bridge. 
- *manual* instead of *dhcp* or *static* is used when you want to create a interface without an IP. Typical scenario is the one i am using it for now, creating a bridge.
- The two physical cards are the united with the *bridge_ports* keyword. 
- *promisc* is used to put the interfaces (enp2s0 and enp4s8) in promiscuous mode. In non-promiscuous mode, the NIC will ignore all traffic it receives that is NOT addressed to it. This makes perfect sense, you do not want to deal with traffic NOT destined for you. I plan however, to use the machine to analyze and sniff some packets thus making the promiscuous mode ideal.

NOTE! When the promiscuous mode is enabled, it will be reflected when issuing the command:

	netstat -i

Which outputs something like the following (example output edited by me):

	Iface       MTU Met    RX-OK RX-ERR RX-DRP RX-OVR    TX-OK TX-ERR TX-DRP TX-OVR Flg
	br0	         1500   0     2075      0      0      0     1369      0      0      0 BMRU
	enp2s0       1500   0     2075      0      0      0     1369      0      0      0 BMPRU
	enp4s8       1500   0     2075      0      0      0     1369      0      0      0 BMPRU
	lo        	16436   0     1985      0      0      0     1985      0      0      0 LRU

The *Flg* column consists of flags meaning the following:

	B = broadcast
	M = multicast
	P = promisc mode
	R = running
	U = Is up




