# Clock Data to Database (CDTDB) manual
Manual for the Clock data to database python script (3) for windows.

## Intro
CDTDB was developed to insert all data produced by *TidFreq-Klokker-Meas v04_06* into a database. Currently, all data produced by *TidFreq-Klokker-Meas v04_06* is dumped into a file locally on a hard drive. In the spirit of *never fix what ain't broke*, CDTDB was designed to remedy the limited capabilities of *TidFreq-Klokker-Meas v04_06* instead of changing it. You might think it's just a "hack", and you would be right.

## Requirements
CDTDB was tested and developed on a workstation running Windows 7 64-bit. It is written in Python (version 3.4.3) and uses *mysql-connector-python-2.0.4-py3.4* for all database interaction. The *mysql-connector* can be downloaded from:

	https://dev.mysql.com/downloads/connector/python/

and Python 3.4.3:

	https://www.python.org/downloads/release/python-343/

The script will NOT work without this software.

## How to use

### Configuring
A big part of CDTDB is it's config file. The script has no user-interface so all settings has to set in it's config file. The following is a short description of the config file's fields:

	[data]
	ref_clock : Someclock
	measurerID : Aril

*ref_clock_* is the name of the clock uses as reference by *TidFreq-Klokker-Meas v04_06* when producing measurements.
*measurerID* is the name or id of the device running an instance of CDTDB. 

	[files]
	folder: C:\
	file_prefix : Kontinuerlig
	insert_mode_path: C:\\test2.dat

*folder* is the path to the folder containing the data produced by *TidFreq-Klokker-Meas v04_06*. 
*file_prefix* is *static* part of the file name that *TidFreq-Klokker-Meas v04_06* produces. 




### Launching
The scripts is used by the command-line (cmd). To launch the script, given that you have navigated to it's location, simply issue the command:

	python .\cdtdb.py
