import ctypes
import configparser
import fileinput, sys
import datetime
import time
import os
import sys


## Not my code START
class PID:
	"""
	Discrete PID control
	"""

	def __init__(self, P=2.0, I=0.0, D=1.0, Derivator=0, Integrator=0, Integrator_max=500, Integrator_min=-500):

		self.Kp=P
		self.Ki=I
		self.Kd=D
		self.Derivator=Derivator
		self.Integrator=Integrator
		self.Integrator_max=Integrator_max
		self.Integrator_min=Integrator_min

		self.set_point=0.0
		self.error=0.0

	def update(self,current_value):
		"""
		Calculate PID output value for given reference input and feedback
		"""

		self.error = self.set_point - current_value

		self.P_value = self.Kp * self.error
		self.D_value = self.Kd * ( self.error - self.Derivator)
		self.Derivator = self.error

		self.Integrator = self.Integrator + self.error

		if self.Integrator > self.Integrator_max:
			self.Integrator = self.Integrator_max
		elif self.Integrator < self.Integrator_min:
			self.Integrator = self.Integrator_min

		self.I_value = self.Integrator * self.Ki

		PID = self.P_value + self.I_value + self.D_value

		return PID

	def setPoint(self,set_point):
		"""
		Initilize the setpoint of PID
		"""
		self.set_point = set_point
		self.Integrator=0
		self.Derivator=0

	def setIntegrator(self, Integrator):
		self.Integrator = Integrator

	def setDerivator(self, Derivator):
		self.Derivator = Derivator

	def setKp(self,P):
		self.Kp=P

	def setKi(self,I):
		self.Ki=I

	def setKd(self,D):
		self.Kd=D

	def getPoint(self):
		return self.set_point

	def getError(self):
		return self.error

	def getIntegrator(self):
		return self.Integrator

	def getDerivator(self):
		return self.Derivator
## Not my code END

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " +"[" + message + "]"
    print(complete_message)

def getConfVal(column, value):
	config = configparser.ConfigParser()
	configFile = "assignment2.ini"
	config.read(configFile)
	conf_value = config[column][value]
	return conf_value

class average:
	count = 0.0 # Number of times called; n.
	total = 0.0	# n-1 + n + n +1
	avg = 0.0	# The current average
	prev_avg = 0.0 # The previous average
	smart_switch = 0

	def __init__(self, smart_switch):
		self.smart_switch = smart_switch
		t_print("Smart switch: " + str(smart_switch))
		
	def pushpeek(self, value_n):
		self.push(value_n) 
		return self.avg

	def get_count(self):
		return self.count
	
	def peek(self):
		return self.avg

	def push(self, value_n):
		if(self.count == 0.0):
				self.avg = value_n
		else:
			if(self.smart_switch == 0): # Naive but accurate method
				self.total = self.total + value_n
				self.avg = self.total / self.count
			else: 						# The coolest method
				self.prev_avg = self.avg
				self.avg = ( ( (self.count-1) * self.prev_avg) + value_n ) / self.count 
		self.count = self.count + 1

def getFile(path):
	result = open(path, 'r')
	resultList = result.read()
	resultList = resultList.splitlines()
	result.close()
	resultList = [float(i) for i in resultList]
	return resultList


## Debug method
def write_average_file(input_file, output_name, a_type):
	avg = average(a_type)	
	length = len(input_file)
	file_out = open(output_name, 'w+')
	count = 0
	while(count < length):
		file_out.write(str(avg.pushpeek(input_file[count])) + "\n")
		count = count + 1
	file_out.close()
	t_print("Finished")

def controller(ref_file, disc_file, output_name):
	length = len(ref_file)
	file_out = open(output_name, 'w+')
	time = 1
	reference = 0.0
	avg = average(0)
	my_pid = PID(0.01,0.001,1.0)
	pid_v = 0.0

	while(time < length):
		reference = avg.pushpeek(ref_file[time])	# Building average

		if(time > 56):
			if(time % 100 == 0):	
				current = disc_file[time]	
				my_pid.setPoint(reference)
				pid_v = my_pid.update(current)
			file_out.write(str (disc_file[time] + pid_v) + "\n")	
		else:
			file_out.write(str (disc_file[time]) + "\n")

		time = time + 1
	file_out.close()

def test():
	count = 0
	while(count < 40000):
		if(count % 1000 == 0):
			print(count)
		count = count + 1

if __name__ == '__main__':
	gps = getFile(getConfVal("files","gps"))
	#write_average_file(gps, "gps_average", 1)
	ocxo = getFile(getConfVal("files","ocxo"))
	controller(gps, ocxo, "ocxo_disc.txt")
