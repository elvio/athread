all: time_log.o app_serial app_athread

app_serial: app_serial.c time_log.o
	gcc -Wall app_serial.c time_log.o -o app_serial

app_athread: app_athread.c time_log.o
	gcc -Wall -lathread app_athread.c time_log.o -o app_athread


time_log.o: time_log.c time_log.h 
	gcc -c time_log.c -o time_log.o

clean:
	rm -f app_serial
	rm -f app_athread
	rm -f app_athread_dis