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

	def __init__(self, P=2.0, I=0.0, D=1.0, Derivator=0.0, Integrator=0.0, Integrator_max=500.0, Integrator_min=-500.0):

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

		self.error = current_value

		self.P_value = self.Kp * self.error
		self.D_value = self.Kd * ( self.error - self.Derivator)
		self.Derivator = self.error

		self.Integrator = self.Integrator + self.error

		#if self.Integrator > self.Integrator_max:
		#	self.Integrator = self.Integrator_max
		#elif self.Integrator < self.Integrator_min:
		#	self.Integrator = self.Integrator_min

		self.I_value = self.Integrator * self.Ki

		PID = self.P_value + self.I_value + self.D_value

		return PID

	def setPoint(self,set_point):
		"""
		Initilize the setpoint of PID
		"""
		self.set_point = set_point
		self.Integrator=0.0
		self.Derivator=0.0

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

# Used for dropping the sample rate. 
# Example GPS 1s to GPS 60s.
def drop_sample_rate(input_file, magnitude, output_name):
	length = len(input_file)
	file_out = open(output_name, 'w+')
	pos = 0

	while(pos < length):
		if(pos % magnitude == 0):
			file_out.write(str (input_file[pos]) + "\n")
		pos = pos + 1
	file_out.close()

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

def getFile(path):
	result = open(path, 'r')
	resultList = result.read()
	resultList = resultList.splitlines()
	result.close()
	resultList = [float(i) for i in resultList]
	return resultList

class old_average:
	count = 0.0 # Number of times called; n.
	total = 0.0	# n-1 + n + n +1
	avg = 0.0	# The current average
	prev_avg = 0.0 # The previous average

		
	def pushpeek(self, value_n):
		self.push(value_n) 
		return self.avg

	def get_average(self, value_n):
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
				self.prev_avg = self.avg
				self.avg = ( ( (self.count-1) * self.prev_avg) + value_n ) / self.count 
		self.count = self.count + 1

class simple_pid:
	ACCUM_ERROR = 0
	PREV_ERROR = 0
	DELTA_Y = 0

	def __init__(self, P, I, D):
		self.Kp=P
		self.Ki=I
		self.Kd=D

	def update(self, FILTERED_ERROR):
		ACCUM_ERROR = ACCUM_ERROR + FILTERED_ERROR	#Accumulating error

		P_VALUE = Kp * FILTERED_ERROR
		I_VALUE = Ki * ACCUM_ERROR
		D_VALUE = Kd * FILTERED_ERROR - PREV_ERROR	# What happens first time?
		PREV_ERROR = FILTERED_ERROR

		DELTA_Y = P_VALUE + I_VALUE + D_VALUE
		return DELTA_Y


class filter:
	f_now = 0.0		# Filtered now
	f_prev = 0.0 	# Filtered previous
	T_SAMPLE = 0
	#u_input		#Unfiltered input

	def __init__(self, T_SAMPLE):
		self.T_SAMPLE = T_SAMPLE

	def get(self, u_input):
		if(self.f_now == 0 and self.f_prev == 0):
				self.f_now = u_input
				return self.f_now
		else:
				self.f_prev = self.f_now
				self.f_now = ( ((self.T_SAMPLE - 1) * (self.f_prev + u_input)) / self.T_SAMPLE )
				return self.f_now

#GPS : GPS reference file
#OSC : Oscillator file
#Output_name : Name of the main file made by the controller | Corrected
#Start : When to start | at which sample
#Interval : At what interval to update PID
#SPS : Samples per second
#T_SAMPLE : Width of the filter | used in average
def new_controller(gps, osci, output_name, start, interval, sps, T_SAMPLE):
	# Python specific init:
	length = len(gps)											# Length of files
	file_freq_cor = open("pid_corr.txt", 'w+')					# "Estimerte frekvenskorreksjoner"
	file_corrected = open(output_name, 'w+')					# "Korrigert tidsavvik"
	pid = simple_pid(0.002,0.0000005,0.00)						# PID controller object
	filtr = filter(T_SAMPLE)									# Average (low pass)

	#pid.setPoint(0)												# =|= BEWARE! =|=

	# Algorithm Init step: (Steps 1 - 2)
	i = 1 														# N, where we are, a counter. Currently
																# it is advanced by 2 in order to initialize
																# it properly
	OSCI_COR = osci[i]											# Corrected is set to X0.
	OSCI_COR_PREV = osci[i] 									# Previous corrected
	ERROR = 0													# Error
	FILTERED_ERROR = 0											# Filtered error
	Yn = osci[i] - osci[i-1]/T_SAMPLE							# "Relativ frekvensavvik"
	DELTA_Y = 0													# Output from PID	# Python specific init:
	length = len(gps)											# Length of files
	file_freq_cor = open("pid_corr.txt", 'w+')					# "Estimerte frekvenskorreksjoner"
	file_corrected = open(output_name, 'w+')					# "Korrigert tidsavvik"
	pid = PID(0.002,0.0000005,0.00)								# PID controller object

	# Algorithm Init step: (Steps 1 - 2)
	i = 1 														# N, where we are, a counter. Currently
																# it is advanced by 2 in order to initialize
																# it properly
	OSCI_COR = osci[i]											# Corrected is set to X0.
	OSCI_COR_PREV = osci[i] 									# Previous corrected
	ERROR = 0													# Error
	FILTERED_ERROR = 0											# Filtered error
	Yn = osci[i] - osci[i-1]/T_SAMPLE							# "Relativ frekvensavvik"
	DELTA_Y = 0													# Output from PID
	
	# Main loop	(Steps 3 - 5)
	while (i < length):
		if((i*sps) > start):							 		# Where to begin
			ERROR = OSCI_COR - gps[i]							# Calculated error updated
			FILTERED_ERROR = filtr.get(ERROR)					# Filtered error updated
			DELTA_Y = pid.update(FILTERED_ERROR)				# Aquiring DELTA_Y from PID
			Yn = (osci[i] - osci[i-1])/T_SAMPLE					# For use in next step...
			OSCI_COR_PREV = OSCI_COR 							# Storing for use in next iteration
			OSCI_COR = OSCI_COR_PREV + T_SAMPLE * (Yn + DELTA_Y)# Calc new corrected

			# Writing to files for use in Timelab
			file_freq_cor.write(str(DELTA_Y) + "\n")			# Write est. freq. corr. + newline
			file_corrected.write(str(OSCI_COR) + "\n")			# Write corrected to file + newline
		else:
			file_freq_cor.write(str(0) + "\n")					# Corrections has not started
			file_corrected.write(str(osci[i]) + "\n")			# Pass through
		i = i + 1 												# Advancing "time"
	
	file_freq_cor.close()										# Closing file, finished writing								
	file_corrected.close()										# Closing file, finished writing

if __name__ == '__main__':
	gps_f = getFile(getConfVal("files","gps_dropped"))
	osc_f = getFile(getConfVal("files","xcsac_60"))
	new_controller(gps_f, osc_f, "xcsac_corrected.txt", 1360,1,60, 100)