# Clock Data to Database (CDTDB) manual
Manual for the Clock data to database python script (3) for windows.

## Intro
CDTDB was developed to insert all data produced by *TidFreq-Klokker-Meas v04_06* into a database. Currently, all data produced by *TidFreq-Klokker-Meas v04_06* is dumped into a file locally on a hard drive. In the spirit of *never fix what ain't broke*, CDTDB was designed to remedy the limited capabilities of *TidFreq-Klokker-Meas v04_06* instead of changing it. You might think it's just a "hack", and you would be right.

## Requirements
CDTDB was tested and developed on a workstation running Windows 7 64-bit. It is written in Python (version 3.4.3) and uses *mysql-connector-python-2.0.4-py3.4* for all database interaction. The *mysql-connector* can be downloaded from:

	https://dev.mysql.com/downloads/connector/python/

and Python 3.4.3:

	https://www.python.org/downloads/release/python-343/

The script also uses the jdutil package from:

	https://gist.github.com/jiffyclub/1294443

The script will NOT work without this software. jdutil.py has to be downloaded and located in some directory in the Python installation path. E.g. C:\python34\Lib\

## How to use

### Configuring
A big part of CDTDB is it's config file. The script has no user-interface so all settings has to be set in it's config file. The following is a short description of the config file's fields:

- **[data]**
  - **ref_clock** The name of the clock uses as reference by *TidFreq-Klokker-Meas v04_06* when producing measurements.
  - **measurerID** The name or id of the device running an instance of CDTDB. 
- **[files]**
  - **folder** Path to the folder containing the data produced by *TidFreq-Klokker-Meas v04_06*. 
  - **file_prefix** The *static* part of the file name that *TidFreq-Klokker-Meas v04_06* produces. 
  - **insert_mode_path** Specifies the full path to a file to be used during *file insertion mode* (see "Modes of operation" for more)
- **[modes]**
  - **file_insert** If set to yes, enables file insert mode and uses the **insert_mode_path** to find the file.
- **[logs]**
  - **enable_logging** If set to yes, enables logging. The log file contains the same as what get written to the command line.
  - **log_path** Specifies full path to the log file.
- **[general]**
 - **interval** Specifies in seconds how often the CDTDB should look for changes in the file produced by *TidFreq-Klokker-Meas v04_06*.
- **[db]**

### Launching
The scripts is used by the command line (cmd). To launch the script, given that you have navigated to it's location, simply issue the command:

	python .\cdtdb.py  	

or more explicitely if Windows doesn't recognize the path tp the python executable:

	C:\python34\python .\cdtdb.py
	

## Modes of operation
### Normal
During normal operation, CDTDB will wake up at specified intervals and check if something new has been written to file. If there is nothing new, it will go back to sleep.

### File insert
When **insert_mode_path** is set to some file and **file_insert** is set to "yes", that file will be inserted into the base. CDTDB DOES NOT contain any sort of duplicate protection, so USE WITH CAUTION!
CDTDB uses the time stamp (mjd) to check where the database is relative to the file it is monitoring. Never use the insert mode for fresh files. The monitoring relies on that the newest time stamps are from the file currently being monitored. Failure to follow these instructions can result in a loss of database integrity. 

## Known issues
- Editing the config file while it's being read crashes the program. Make sure it's only edited while it sleeps.
- The script doesn't add new data to a non-empty database unless there is already some data with the same measurerID as in the current config. The workaround is to first add data using 'file_insert: yes', then run the script again with file insert switched off.

## To do
See "Known issues"