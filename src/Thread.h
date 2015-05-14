#ifndef THREAD_H_
#define THREAD_H_

class Runnable {
public:
  virtual void run (  ) = 0;
};

#ifdef _WIN32

#include <windows.h>

class Thread:virtual public Runnable {
private:
  HANDLE threadID;
  static DWORD WINAPI __run ( void *cthis ) {
    static_cast < Runnable * >( cthis )->run (  );
    return 0;
} public:
  void start (  ) {
    DWORD i;
    threadID = CreateThread ( NULL, 0, ( LPTHREAD_START_ROUTINE ) __run, ( LPVOID ) this, 0, &i );
  }
  void join (  ) {
    WaitForSingleObject ( threadID, INFINITE );
  }
  void stop (  ) {
    CloseHandle ( threadID );
  }
};

#else

#include <pthread.h>

class Thread:virtual public Runnable {
private:
  Runnable * _runnable;
  pthread_t threadID;
  static void *__run ( void *cthis ) {
    static_cast < Runnable * >( cthis )->run (  );
    return NULL;
} public:
   Thread (  ):_runnable ( NULL ) {
    threadID = 0;
  }

  int start (  ) {
    Runnable *execRunnable = this;
    if ( this->_runnable != NULL ) {
      execRunnable = this->_runnable;
    }
    return pthread_create ( &threadID, NULL, __run, execRunnable );
  }
  void join (  ) {
    pthread_join ( threadID, NULL );
  }
  void stop (  ) {
    if ( threadID )
      pthread_detach ( threadID );
  }
};
#endif
#endif
