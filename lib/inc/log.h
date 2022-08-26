#ifndef __LOG_H___
#define __LOG_H___
#include <iostream>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL 0
#define LOGI(format, ...) printf("INFO [%s] " format "\n", log_format_time(), ##__VA_ARGS__)
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOGD(format, ...) printf("DEBUG [%s] " format "\n", log_format_time(), ##__VA_ARGS__)
#else
#define LOGD(format, ...)
#endif


char *log_format_time();

#endif /** __LOG_H___ **/
