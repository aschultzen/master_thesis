'''
:Author: Aril Schultzen
:Email: aschultzen@gmail.com 
'''
# This script attempts to connect to the
# Sensor Server at <ip> : "port" and 
# IDs itself as <id>. It will then
# poll the time solved by the GNSS receiver
# connected to Sensor<id> until
# terminated. 

import socket  
import sys  
import time

ip = "10.1.0.46"
port = 10001
id = 1

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except socket.error, msg:
    print 'Failed to create socket. Error code: ' + str(msg[0]) + ' , Error message : ' + msg[1]
    sys.exit();
try:
    remote_ip = socket.gethostbyname( ip )
 
except socket.gaierror:
    print 'Could not resolve hostname'
    sys.exit()
     
s.connect((remote_ip , port))
s.sendall(b'IDENTIFY -10')
recv_buff = s.recv(1024)

while(1):
	s.sendall(b'PRINTTIME' + str(id))
	time.sleep(0.1)
	recv_buff = s.recv(1024)
	recv_buff = recv_buff.strip('>\n')
	print("Sensor " + str(id) + " GNSS solved time: " + recv_buff)
	time.sleep(0.9)