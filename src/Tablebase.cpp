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

#include "Tablebase.h"

Tablebase::Tablebase() {
}

Tablebase::~Tablebase() {
}

void Tablebase::load() {
}

int Tablebase::getCache() {
    return cacheSize;
}

string Tablebase::getPath() {
    return path;
}

string Tablebase::getSchema() {
    return "tb_UNCOMPRESSED";
}

bool Tablebase::getAvailable() {
    return false;
}

void Tablebase::print(unsigned stm1, unsigned info1, unsigned pliestomate1) {
}

bool Tablebase::setCacheSize(int mb) {
    return true;
}

bool Tablebase::setScheme(string s) {
    return 1;
}

void Tablebase::restart() {
}

bool Tablebase::setProbeDepth(int d) {
    return true;
}

bool Tablebase::setInstalledPieces(int n) {
    return true;
}

void Tablebase::setPath(string path1) {
}
