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

#ifndef ITERATIVEDEEPING_H_
#define ITERATIVEDEEPING_H_

#include <cstring>
#include <string.h>
#include "util/String.h"
#include "SearchManager.h"
#include "util/Thread.h"
#include "OpenBook.h"
#include <stdio.h>
#include <stdlib.h>
#include "namespaces.h"

class IterativeDeeping : public Thread {

public:
    //TODO private
    mutex mutexIT;//TODO eliminare?


//    void setMaxTimeMillsec(int i);

    virtual ~ IterativeDeeping();

    virtual void run();

    bool getPonderEnabled();

    bool getGtbAvailable();

    bool getUseBook();

    void setUseBook(bool);

    void enablePonder(bool);

    void setMaxDepth(int);

    void loadBook(string);

//    int printDtm();

    bool setParameter(String param, int value);

//    virtual int loadFen(string fen = "");

//    void display();

//    int getHashSize();

//    bool setHashSize(int i);

//    void setRunning(bool i);

//    void startClock();

//    void setPonder(bool i);

//    string getFen();

//    int getSide();

//    int getScore(int side);

//    void clearHash();

//    int getMaxTimeMillsec();

//    void setNullMove(bool i);

//    void makemove(_Tmove *ptr);

    //  void setSide(bool i);

//    int getMoveFromSan(String string, _Tmove *ptr);


//    void pushStackMove();

//    void init();

//    void setRepetitionMapCount(int i);

//    void deleteGtb();

    void createGtb();

//    void setGtbPath(String string);

//    bool setGtbCacheSize(int i);

//    bool setGtbScheme(String string);

//    bool setGtbInstalledPieces(int i);

//    bool setGtbProbeDepth(int i);

//    void restartGtb();

    IterativeDeeping();


private:

#ifdef DEBUG_MODE
    Hash &hash = Hash::getInstance();
#endif
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    int maxDepth;

    bool useBook;
    Tablebase *tablebase = nullptr;
    OpenBook *openBook;
    bool ponderEnabled;
};

#endif
