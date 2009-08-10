#!/usr/bin/python

# That's a fucking simple script to help in execution and time measurement. It
# was originaly written by Elvio Vicosa (elvio.vicosa@gmail) in his final University
# work. 

import os

files_to_run = ['app_serial', 'app_athread']
weights = [1, 2, 3, 4, 5, 6, 7]

for file_path in files_to_run:
	for weight in weights:
		cmd = "./" + file_path + " " + str(weight)
		print "Executing %s\n" % (cmd)
		os.system(cmd)