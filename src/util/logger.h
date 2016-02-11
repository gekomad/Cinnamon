/*
    Cinnamon UCI chess engine
    Copyright (C) Giuseppe Cannella

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../util/Singleton.h"
#include "../threadPool/Spinlock.h"

using namespace std;

namespace _logger {

    static enum LOG_LEVEL {
        _TRACE = 0, _DEBUG = 1, _INFO = 2, _WARN = 3, _ERROR = 4, _FATAL = 5, _OFF = 6, _ALWAYS = 7
    } _LOG_LEVEL;

#if !defined DLOG_LEVEL
#if defined DEBUG_MODE
#define DLOG_LEVEL _TRACE
#else
#define DLOG_LEVEL _OFF
#endif
#endif

    static const string LOG_LEVEL_STRING[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "OFF", "LOG"};

    class Logger : public Singleton<Logger>, public ofstream {
        friend class Singleton<Logger>;

    public:
        void setLogfile(const string &f, const bool append = false) {
            this->open(f, append ? std::ofstream::app : std::ofstream::out);
        }

        template<LOG_LEVEL type, typename T, typename... Args>
        void _log(T t, Args... args) {
            _CoutSyncSpinlock.lock();
            cout << Time::getLocalTime() << " " << LOG_LEVEL_STRING[type] << " ";
            *this << Time::getLocalTime() << " " << LOG_LEVEL_STRING[type] << " ";
            __log(t, args...);
            cout << endl;
            *this << endl;
            _CoutSyncSpinlock.unlock();
        }

    private:
        Spinlock _CoutSyncSpinlock;

        template<typename T>
        void __log(T t) {
            cout << (t);
            *this << (t);
        }

        template<typename T, typename... Args>
        void __log(T t, Args... args) {
            cout << (t);
            *this << (t);
            __log(args...);
        }

        ~Logger() {
            this->close();
        }
    };

    static Logger &logger = Logger::getInstance();

#if defined(_WIN32)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define LINE_INFO __FILENAME__,":",__LINE__," "

#define log(...)                             {logger._log<LOG_LEVEL::_ALWAYS>(LINE_INFO,__VA_ARGS__);}
#define trace(...) if (_TRACE >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_TRACE>( LINE_INFO,__VA_ARGS__);}
#define debug(...) if (_DEBUG >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_DEBUG>( LINE_INFO,__VA_ARGS__);}
#define info(...)  if (_INFO  >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_INFO> ( LINE_INFO,__VA_ARGS__);}
#define warn(...)  if (_WARN  >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_WARN> ( LINE_INFO,__VA_ARGS__);}
#define error(...) if (_ERROR >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_ERROR>( LINE_INFO,__VA_ARGS__);}
#define fatal(...) if (_FATAL >= DLOG_LEVEL) {logger._log<LOG_LEVEL::_FATAL>( LINE_INFO,__VA_ARGS__);}

}

using namespace _logger;