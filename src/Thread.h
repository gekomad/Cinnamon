#ifndef THREAD_H_
#define THREAD_H_
#include <pthread.h>
class Runnable {
private:
public:
  Runnable (  ) {
  } virtual ~ Runnable (  ) {
  }
  virtual void run (  ) = 0;
};

class Thread:virtual public Runnable {
private:
  Runnable * _runnable;
  pthread_t threadID;
  static void *__run ( void *cthis ) {
    static_cast < Runnable * >( cthis )->run (  );
    return NULL;
} public:
   Thread (  ):_runnable ( NULL ) {
  }
  Thread ( Runnable * r ) {
    this->_runnable = r;
  }

  int start (  ) {
    Runnable *execRunnable = this;
    if ( this->_runnable != NULL ) {
      execRunnable = this->_runnable;
    }
    return pthread_create ( &threadID, NULL, __run, execRunnable );
  }
  int join (  ) {
    return pthread_join ( threadID, NULL );
  }
  int stop (  ) {
    return pthread_detach ( threadID );
  }
};

#endif
