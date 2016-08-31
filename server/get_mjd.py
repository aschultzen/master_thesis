#! /usr/bin/env python

import datetime
import jdutil
from dateutil import parser

today = datetime.datetime.utcnow()
print(jdutil.jd_to_mjd(jdutil.datetime_to_jd(today))) 
