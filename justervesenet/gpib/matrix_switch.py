from ConfigParser import SafeConfigParser

class matrix_switch(object):
	''' A class for the matrix switch. It mostly contains state '''
	def __init__(self, handle, config_path):
		self.switch_handle = handle
		self.source_index = 0
		self.config_path = config_path
		self.cf = SafeConfigParser()
		self.cf.read(self.config_path)
		self.number_of_ports = int(self.cf.get('matrix','num_of_ports'))
		self.reload_sources()

		self.switch_codes = [
		"ZEROINDX",
		"01010100",
		"01010200",
		"01010400",
		"01010800",
		"01011000",
		"01012000",
		"01014000",
		"01018000",
		"01010001",
		"01010002",
		"01010004",
		"01010008",
		"01010010",
		"01010020",
		"01010040",
		"01010080",
		]

	def reload_sources(self):
		self.cf.read(self.config_path)
		self.source_list = self.cf.get('sources','list')
		self.source_list = self.source_list.rstrip(" ")
		self.source_list = self.source_list.split("\n")

	def iterate_port(self):
		self.source_index += 1
		if(self.source_index > self.number_of_ports):
			self.source_index = 1
		return self.source_index

	def get_next_source(self):
		# Check if port is supposed to be switched to:
		empty_list_retry_time = int(self.cf.get('sources','empty_list_retry')) 
		line = "NULL"
		counter = 0
		while(True):
			port = self.iterate_port()
			for x in range(0, len(self.source_list)):
				line = self.source_list[x].split(",")
				if(line[1] == str(port) and line[0][0] != "#"):
					return line
			counter += 1
			if(counter > self.number_of_ports):
				t_print("No sources configured. Trying again in: " + str(empty_list_retry_time) + " seconds")
				time.sleep(empty_list_retry_time)
				self.reload_sources()
				counter = 0

	def switch(self):
		self.reload_sources()
		switch_touple = self.get_next_source()
		source_name = switch_touple[0]
		port_to_switch = int(switch_touple[1])
		#gpib.write(switch_handle, self.switch_codes[port_to_switch])
		return (source_name, port_to_switch)