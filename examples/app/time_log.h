#ifndef _HAVE_TIME_LOG
#define _HAVE_TIME_LOG

//#define FILE_LOG_PATH "time.log"

/*
 * starts the time count. ptag define a tag to describe the log
 */
void time_log_init(char *ptag); 

/*
 * stop time count
 */
void time_log_stop();

void compute_time_and_flush();
void set_file_log_path(char *path);

	
#endif
