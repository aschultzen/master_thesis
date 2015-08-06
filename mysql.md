# Prosjekt notes

## Mysql setup

Installed the package with:

	sudo apt-get install mysql-server python-mysqldb

Credentials:

	User: root
	pass: sqlroot

## Dealing with users

During the install of mySql, we set up a root account. This is all nice and dandy, but for everyday use in scripts and such, we need to create a user with a restricted set of GRANTS for security reasons. A user can be create with the following command:

	CREATE USER 'monitor'@'yl6l005396.justernett.no' IDENTIFIED BY 'monitor';

Notice how the hostname is specified. Since the hostname is *workstation specific*, the same user could not login from a different workstation. It is also important to grant the newly created user some permissions. In mySQL, permissions are called *grants*. These can be granted to users in a manner that you seem fit. For our script, we only need to be able to select, insert, update. But since we still don't know exactly what the script might end up doing, the user is granted permission to create and delete:

	GRANT SELECT, INSERT, DELETE, UPDATE, CREATE ON clock_data. * TO 'monitor'@'yl6l005396.justernett.no';

At some point you might need to change a user. Maybe the script will be executed from a different workstation. 

## Creating the "base"

Log in with root.

	mysql -u root -p

Now to make a DB:

	CREATE DATABASE clock_data;

The reason why we just used root, is that the monitor does not have the proper GRANTS to create a DB. The database that we just created, is empty. There are no tables here. You can check by selecting the database...:
	
	USE clock_data;

..and querying for the tables:

	SHOW TABLES;

Let's make a table:

	CREATE TABLE ntpq (
		ntpqID INT NOT NULL AUTO_INCREMENT,
		seenFromIP VARCHAR(32),
		timeStamp FLOAT(10,4),
		timeStampString VARCHAR(100),
		serverIP VARCHAR(32),
		ntpqResponse INT,
		selectStatus VARCHAR(25),
		syncSource VARCHAR(25), 
		stratum INT, 
		ntpClockType VARCHAR(5),
		ntpqWhen FLOAT(10,4),
		ntpqPoll FLOAT(10,4),
		ntpqReach FLOAT(10,4),
		ntpqDelay FLOAT(10,4),
		ntpqOffset FLOAT(10,4),
		ntpqJitter FLOAT(10,4),
		ntpMonitorID VARCHAR(100), 
		timeStampBackDated VARCHAR(100),
		PRIMARY KEY (ntpqID)
	);

Note: When creating the table over, the ntpqID will be made the primary key. It will also increment automatically for every insertion in the table (AUTO_INCREMENT). It has also got a constraint (NOT NULL) to keep it from ever being NULL. This is important since it will be used as a primary key. Let's create another table: 

	CREATE TABLE alarm (
		alarmID INT NOT NULL AUTO_INCREMENT,
		ntpqID INT, 
		alarmDescri VARCHAR(100),
		alarmState INT,
		alarmEmail VARCHAR(100),
		alarmCondition VARCHAR(100),
		clearedByNtpqID INT, 
		PRIMARY KEY (alarmID),
		FOREIGN KEY (ntpqID) REFERENCES ntpq(ntpqID)
	);

In this table we have also defined a *FOREIGN KEY*. The foreign key points to the primary key in the ntpq table. In our example, this is important because 0 or more alarms can be raised because of a row in the ntpq table. If an alarm is raised, we can trace it back to the ntpq row that caused it. 

If you later find out that you need to add a column, you can use the ALTER command:

	ALTER TABLE ntpq ADD ntpMonitorID VARCHAR(50);

### Example queries

Select all the rows where syncSource = .ATOM. and show all columns:

	select * from ntpq where syncSource = ".ATOM.";

The same, but show only the ntpqID column:

	select ntpqID from ntpq where syncSource = ".ATOM.";

### Useful commands

	SHOW DATABASES;
	USE <dbname> 					//Select DB
	SHOW GRANTS FOR 'user'@'locale'	// Show the permissions given to this user
	GRANT SELECT ON clock_data. * TO 'user'@'locale' // Grant SELECT
	SHOW TABLES	// Shows the tables
	delete * from ntpq where "seenFromIp" == "lol"
	SELECT table_schema "Data Base Name", sum( data_length + index_length) / 1024 / 1024 "Data Base Size in MB" FROM information_schema.TABLES GROUP BY table_schema ;







	


