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

#include <fstream>
#include "Eval.h"

#if defined(_WIN32)
#include "Time.h"
#endif

class OpenBook : public Singleton<OpenBook> {
    friend class Singleton<OpenBook>;

public:
    static OpenBook &getInstance() =delete;
    static OpenBook *getInstance(const string &fileName) {

        if (!FileUtil::fileExists(fileName)) {
            cout << fileName << " not found" << endl;
            return nullptr;
        }

        static OpenBook openBook;

        openBook.openBookFile = fopen(fileName.c_str(), "rb");
        openBook.Random64 = (u64 *) malloc(781 * sizeof(u64));
        int k = 0;
        for (int i = 0; i < 15 && k < 781; i++) {
            for (int j = 0; j < 64 && k < 781; j++) {
                openBook.Random64[k++] = _random::RANDOM_KEY[i][j];
            }
        }
        return &openBook;
    }

    virtual ~OpenBook();


    string search(string fen);

    void dispose();

private:
    OpenBook();

    typedef struct {
        u64 key;
        unsigned short move;
        unsigned short weight;
        unsigned learn;
    } entry_t;

    FILE *openBookFile;

    u64 createKey(string fen);

    int intFromFile(int l, u64 *r);

    int entryFromFile(entry_t *entry);

    int findKey(u64 key, entry_t *entry);

    void moveToString(char move_s[6], unsigned short move);

    u64 *Random64;
};

