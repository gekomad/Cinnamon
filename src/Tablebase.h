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


class Tablebase : public Singleton<Tablebase> {
    friend class Singleton<Tablebase>;

public:

    ~Tablebase();

    void cacheInit(int mb);

    bool getAvailable();

    int getCache();

    string getPath();

    string getSchema();

    bool setCacheSize(int mb);

    void setPath(string path);

    bool setScheme(string s);

    void restart();

    bool setProbeDepth(int d);

    bool setInstalledPieces(int n);

    bool isInstalledPieces(int p) {
        return 1;
    }

    int getProbeDepth() {
        return 1;
    }

    template<int side, bool doPrint>
    int getDtm(_Tchessboard &chessboard, uchar rightCastle, int depth) {
	return 0;
    }

private:
    Tablebase();

};


