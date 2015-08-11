# Prosjekt notes

## MySQL setup

The MySQL server can easy be installed with the following command if you are running Debian/Ubuntu:

	sudo apt-get install mysql-server python-mysqldb

The installer will help you set up the root account, i used to following:

	User: root
	pass: sqlroot

The MySQL server is at this stage installed and ready to go and can be accessed with the following command:

	mysql -u root -p

After typing the correct password, you can use the MySQL monitor to issue queries.

## Dealing with users

During the install of MySQL, we set up a root account. This is all nice and dandy, but for everyday use in scripts and such, we need to create a user with a restricted permissions for security reasons. A user can be created with the following command:

	CREATE USER 'monitor'@'yl6l005396.justernett.no' IDENTIFIED BY 'monitor';

Notice how the hostname is specified. Since the hostname is *workstation specific*, the same user could not login from a different workstation. It is also important to grant the newly created user some permissions. In MySQL, permissions are called *grants*. These can be granted to users in a manner that you seem fit. For our script, we only need to be able to *select*, *insert*, *update*. But since we still don't know exactly what the script might end up doing, the user is also granted permission to *create* (tables) and *delete* (rows):

	GRANT SELECT, INSERT, DELETE, UPDATE, CREATE ON clock_data. * TO 'monitor'@'yl6l005396.justernett.no';

At some point you might need to change a user. Maybe the script will be executed from a different workstation. The following command changes the hostname for *someuser*:

	UPDATE mysql.user SET host = "fnt655j" where user = "someuser";

## Creating the database and tables

Log in with root:

	mysql -u root -p

To create a database, use the following command:

	CREATE DATABASE example_base;

The reason why we just used root, is that the monitor does not have the proper *grants* to create a database. Since a database can contain many tables, we need to select the database before we issue any queries:
	
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
		ntpqOffset FLOAT(10,4),h
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

A short note about queries: A query is issued once it has been "terminated" with a semicolon. This means that you can write a line, press enter and then continue on the next as long as you want until you write the ";". An examples of this is the table creation.
#### Show table info

To show the names for all the columns:

	show columns from ntpq;

This generates output similar to this:

	+-----------------+--------------+------+-----+---------+----------------+
	| Field           | Type         | Null | Key | Default | Extra          |
	+-----------------+--------------+------+-----+---------+----------------+
	| alarmID         | int(11)      | NO   | PRI | NULL    | auto_increment |
	| ntpqID          | int(11)      | YES  | MUL | NULL    |                |
	| alarmDescri     | varchar(100) | YES  |     | NULL    |                |
	| alarmState      | int(11)      | YES  |     | NULL    |                |
	| alarmEmail      | varchar(100) | YES  |     | NULL    |                |
	| alarmCondition  | varchar(100) | YES  |     | NULL    |                |
	| clearedByNtpqID | int(11)      | YES  |     | NULL    |                |
	+-----------------+--------------+------+-----+---------+----------------+

#### Select
Select all the rows where syncSource = .ATOM. and show all columns:

	select * from ntpq where syncSource = ".ATOM.";

The same, but show only the ntpqID column:

	select ntpqID from ntpq where syncSource = ".ATOM.";

#### Insert
To insert a row into table:

	INSERT INTO alarm (ntpqID, alarmDescri, alarmEmail) VALUES (2, "Something happened!", "aril@email.com");

In this insertion not all the fields where defined. Depending on the constraints imposed on the table, this might not always be possible. Sometimes a field *has* to be defined for a insertion to take effect. This is usually the case with a field that is a primary key. See section *See table info* for more. 

#### Update
Updating a row (or multiple rows) can be achieved with with the *update* command. In it's simplest form, you just define a criteria and what needs to be changed/updated:

	UPDATE alarm SET alarmDescri = "Alarm cleared by technician" WHERE ntpqID = 2;

All rows matching the criteria will be changed. 

#### Remove
To remove a row for whatever reason, the syntax is simple:

	DELETE FROM alarm WHERE ntpqID = 2;

#### Joins
Earlier in this guide we created two tables, the ntpq table and the alarm table. The alarm table has a column *ntpqID* which refers to the primary key in the ntpq table. We can use this to *join* these tables together as if they where one:

	SELECT ntpq.ntpqID, ntpq.timeStamp, ntpq.stratum, alarm.alarmID, alarm.clearedByNtpqID
    from ntpq
    INNER JOIN alarm
    ON ntpq.ntpqID = alarm.ntpqID;

As the example shows, some of the columns are chosen (similar to earlier select examples) but the tables from which they are selected are defined before the name of the column (nameoftable.column). The *INNER JOIN* shows which table the ntpq table should be join *on*, and lastly at *what* column.

### Useful commands

	SHOW DATABASES;
	USE <dbname> 					//Select DB
	SHOW GRANTS FOR 'user'@'locale'	// Show the permissions given to this user
	GRANT SELECT ON clock_data. * TO 'user'@'locale' // Grant SELECT
	SHOW TABLES	// Shows the tables
	delete * from ntpq where "seenFromIp" == "lol"
	SELECT table_schema "Data Base Name", sum( data_length + index_length) / 1024 / 1024 "Data Base Size in MB" FROM information_schema.TABLES GROUP BY table_schema ;

# Documentation

## Tables

The following tables (represented by the query that made them) has been created on the mySQL server:

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
			ntpqOffset FLOAT(10,4),h
			ntpqJitter FLOAT(10,4),
			ntpMonitorID VARCHAR(100), 
			timeStampBackDated VARCHAR(100),
			PRIMARY KEY (ntpqID)
		);

The ntpq table is used to store data from ntpq queries.

	CREATE TABLE ntpq2 (
			ntpqID INT,
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
			ntpqOffset FLOAT(10,4),h
			ntpqJitter FLOAT(10,4),
			ntpMonitorID VARCHAR(100), 
			timeStampBackDated VARCHAR(100),
			PRIMARY KEY (ntpqID)
		);
Same as ntpq table, but without the auto increment on primary key (ntpqID). This table is used for testing only.



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
The alarm table is used to store alarms raised on the basis of data stored in the ntpq table. They were designed to be joined at *ntpqID*.


	CREATE TABLE clock_measurements (
		clck_msrmID INT NOT NULL AUTO_INCREMENT,
		date DATE, 
		time TIME,
		mjd FLOAT(10,5),
		source VARCHAR(50),
		value DECIMAL(65,30),
		PRIMARY KEY (clck_msrmID)
	);

The clock_measurements table is used to store data collected from clocks and produced by LabView. 





	


