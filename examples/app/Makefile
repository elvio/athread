all: time_log.o app_serial app_athread app_tcc

app_serial: app_serial.c time_log.o
	gcc -Wall -lm app_serial.c time_log.o -o app_serial

app_athread: app_athread.c time_log.o
	gcc -Wall -lm -lmpi -lathread app_athread.c time_log.o -o app_athread
	
app_tcc: app_tcc.c time_log.o
	gcc -Wall -lm -lmpi -lathread app_tcc.c time_log.o -o app_tcc


time_log.o: time_log.c time_log.h 
	gcc -c time_log.c -o time_log.o

clean:
	rm -f app_serial
	rm -f app_athread
	rm -f app_athread_dis