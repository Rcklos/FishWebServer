#include "log.h"

char *log_format_time() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm * timeinfo = localtime(&ts.tv_sec);
  static char buffer[20];
  sprintf(buffer, "%02d-%02d %02d:%02d:%02d",
      timeinfo->tm_mon + 1, timeinfo->tm_mday, 
      timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  return buffer;
}
