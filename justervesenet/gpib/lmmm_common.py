import datetime
from datetime import date,timedelta
import jdutil
import time
import os
from ConfigParser import SafeConfigParser

def get_today_mjd():
    today = datetime.datetime.utcnow()
    return jdutil.jd_to_mjd(jdutil.datetime_to_jd(today)) 

def get_today_date():
	return time.strftime("%Y/%m/%d")

def get_now_time():
	return time.strftime("%H:%M:%S")

def t_print(message):
    current_time = datetime.datetime.now().time()
    complete_message = "[" + current_time.isoformat() + "] " + message
    print(complete_message)

def init_config(config_path):
	config_parser = SafeConfigParser()
	conf_status = config_parser.read(config_path)

	if(len(conf_status) == 0):
		t_print("Failed to load " + config_path)
		return 0
	else:
		t_print("Config loaded from " + config_path)
		return config_parser
