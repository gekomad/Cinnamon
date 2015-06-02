#ifndef THREAD_H_
#define THREAD_H_
#include <thread>
class Runnable {
public:
  virtual void run (  ) = 0;
};

class Thread:virtual public Runnable {
private:
  thread * theThread;
  Runnable *_runnable;
  Runnable *execRunnable;
  static void *__run ( void *cthis ) {
    static_cast < Runnable * >( cthis )->run (  );
    return nullptr;
} public:
   Thread (  ):_runnable ( nullptr ) {
    theThread = nullptr;
    execRunnable = this;
  }

  void start (  ) {
    ASSERT ( !theThread );
    if ( this->_runnable != nullptr ) {
      execRunnable = this->_runnable;
    }
    theThread = new thread ( __run, execRunnable );
  }

  void join (  ) {
    if ( theThread ) {
      theThread->join (  );
      delete theThread;
      theThread = nullptr;
    }
  }

  bool isJoinable (  ) {
    return theThread->joinable (  );
  }

  void stop (  ) {
    if ( theThread ) {
      theThread->detach (  );
      delete theThread;
      theThread = nullptr;
    }
  }

  ~Thread (  ) {
    if ( theThread ) {
      theThread->detach (  );
      delete theThread;
    }
  }
};
#endif
