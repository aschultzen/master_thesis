import sys
import time

from lmmm_common import (t_print, init_config)
from lmmm_db import (db_connector, dbClose, create_query, upload)

CONFIG_PATH = "lmmm_config.ini"

# Returns number of uploaded lines. 
# Will attempt to upload forever.
def upload_dump(file, db_con, config_parser):
	file_contents = file.read().split("\n")

	uploaded_lines = 0
	for x in range(0, len(file_contents)):
		# Update progress
		file_line = file_contents[x]
		# If * is found at end of line, upload it
		if(file_line[len(file_line)-1] == '*'):
			file_line = file_line.split(" ")

			# Making the structure needed by upload
			measurement = {
				'date': file_line[0],
				'time': file_line[1],
				'mjd' : file_line[2],
				'source': file_line[3],
				'value' : file_line[4],
				'ref_clock': file_line[5],
				'measurerID': file_line[6],
			}

			upload_status = upload(db_con, measurement)

			while(upload_status == 0):
				t_print("Upload failed, retrying...")
				time.sleep(2)
				upload_status = upload(db_con, measurement)

			uploaded_lines += 1

	return uploaded_lines


if __name__ == '__main__':
	t_print("Starting LMMM dump-file uploader...")
	
	# Opening files
	file_list = []

	for x in range(1, len(sys.argv)):
		try:
			file_list.append(open(sys.argv[x], "r"))
		except IOError:
			t_print("Could not open file: " + sys.argv[x] + ". Aborting.")
			sys.exit()

	# Initializes config
	config_parser = init_config(CONFIG_PATH)
	if(config_parser == 0):
		sys.exit()

	# Connecting to the database
	db_con = db_connector(config_parser)
	if(db_con == 0):
		sys.exit()

	# Main loop, files are uploaded in the same sequnce as
	# as they were passed as parameters

	t_print("Inserting...")
	
	counter = 0
	while(counter < len(file_list)):
		num_lines = upload_dump(file_list[counter], db_con, config_parser)
		t_print("Uploaded " + str(num_lines) + " lines from " + file_list[counter].name)
		counter += 1


