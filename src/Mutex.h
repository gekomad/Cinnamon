#ifndef MUTEX_H_
#define MUTEX_H_

#ifdef _WIN32
#include <windows.h>

class Mutex {
public:
  Mutex (  ) {
    mutex = CreateMutex ( NULL, FALSE, NULL );
  } ~Mutex (  ) {
    CloseHandle ( mutex );
  }

  void lockMutex ( bool b ) {
    b ? WaitForSingleObject ( mutex, INFINITE ) : ReleaseMutex ( mutex );
  }

private:
  HANDLE mutex;
};

#else
#include <pthread.h>

class Mutex {
public:

  Mutex (  ) {
    mutex = PTHREAD_MUTEX_INITIALIZER;
  } ~Mutex (  ) {
  }

  void lockMutex ( bool b ) {
    b ? pthread_mutex_lock ( &mutex ) : pthread_mutex_unlock ( &mutex );
  }

private:
  pthread_mutex_t mutex;
};
#endif

#endif
