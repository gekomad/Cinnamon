/*
    Cinnamon is a UCI chess engine
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

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include <mutex>

template<class T>
class Singleton {
public:

    static T &getInstance() {
        static lock_guard<mutex> lock(singletonMutex);
        if (!_instanceSingleton) {
            _instanceSingleton = new T();
        }
        return *_instanceSingleton;
    }

private:
    static T *_instanceSingleton;
    static mutex singletonMutex;
};


template<class T> T *Singleton<T>::_instanceSingleton = nullptr;
template<class T> mutex Singleton<T>::singletonMutex;

#endif
