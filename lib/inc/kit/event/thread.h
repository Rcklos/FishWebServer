#ifndef __KIT_EVENT_THREAD_H__
#define __KIT_EVENT_THREAD_H__

class Runnable {
public:
  virtual void run();
};

class Thread {
private:
  char *name_;
public:
  Thread(Runnable &runnable);
  Thread(const char *name);
  Thread(char *name);
  Thread();
  ~Thread();

  char *get_thread_name();
  virtual void run(); // inherit
  void start();
};

#endif
