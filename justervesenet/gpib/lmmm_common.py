import datetime
from datetime import date,timedelta
import jdutil
import time

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