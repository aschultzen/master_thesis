import smtplib
import subprocess
import string
import time
import os

def send_mail(msg):
	open_vpn = subprocess.call(['nmcli', 'con', 'up', 'id', 'Paris'])

	HOST = "smtp.gmail.com:587"
	SUBJECT = "SSS (Sensor Server Script) Report"
	TO = "aschultzen@gmail.com"
	FROM = "soridarusnake@gmail.com"
	BODY = string.join((
	        "From: %s" % FROM,
	        "To: %s" % TO,
	        "Subject: %s" % SUBJECT ,
	        "",
	        msg
	        ), "\r\n")
	
	try: 
		server = smtplib.SMTP(HOST)
		server.ehlo()
		server.starttls()
		server.ehlo()
		server.login('soridarusnake@gmail.com', 'horseclockneversleeps') 
		server.sendmail("soridarusnake@gmail.com", "aschultzen@gmail.com", BODY)
		server.quit()
	except socket.error as e:
   		print "Something happened with the mail-thingy, i don't care."

	open_vpn = subprocess.call(['nmcli', 'con', 'down', 'id', 'Paris'])

def main_routine():
	print("Starting SSS checker..")

	previous_size = 0

	while(True):
		print("Rolling!")
		message = ""
		rsync = subprocess.call(['rsync', '-avz', '-e', 'ssh', 'aril@10.1.0.45:/home/aril/dp.txt', '.'])
		file_size = os.path.getsize("dp.txt")
		if(file_size > previous_size):
			message = "This is Snake! Everything is going great. The file is now " + str(file_size) + " bytes big and has grown with " + str(file_size - previous_size) + " bytes.\n\n\nLove, Snake!"
		else:
			message = "This is Snake! Something is wrong, the file has not grown in size.\n\n\nLove, Snake!"
		previous_size = file_size
		send_mail(message)
		##Sleep for 12 hours
		time.sleep(43200)

if __name__ == '__main__':
    main_routine()