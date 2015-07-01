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
#include <unistd.h>
#include <atomic>
#include "IterativeDeeping.h"

IterativeDeeping::IterativeDeeping() : maxDepth(MAX_PLY), openBook(nullptr), ponderEnabled(false) {

    setUseBook(false);
#if defined(DEBUG_MODE)
    string parameterFile = "parameter.txt";
    if (!_file::fileExists(parameterFile)) {
        cout << "warning file not found  " << parameterFile << endl;
        return;
    }
    ifstream inData;
    string svalue, line;
    String param;
    int value;
    inData.open(parameterFile);
    while (!inData.eof()) {
        getline(inData, line);
        stringstream ss(line);
        ss >> param;
        ss >> svalue;
        value = stoi(svalue);
        setParameter(param, value);
    }
    inData.close();
#endif

}

void IterativeDeeping::setMaxTimeMillsec(int i) {
    searchManager.setMaxTimeMillsec(i);
}

void IterativeDeeping::setMaxDepth(int d) {
    maxDepth = d;
}

bool IterativeDeeping::getGtbAvailable() {
    return searchManager.getGtbAvailable();
}

IterativeDeeping::~IterativeDeeping() {
    delete &searchManager;
    if (openBook) {
        delete openBook;
    }
}

void IterativeDeeping::enablePonder(bool b) {
    ponderEnabled = b;
}

bool IterativeDeeping::getPonderEnabled() {
    return ponderEnabled;
}

bool IterativeDeeping::getUseBook() {
    return useBook;
}

void IterativeDeeping::loadBook(string f) {
    if (!openBook) {
        openBook = new OpenBook();
    }
    useBook = openBook->load(f);
    if (!useBook) {
        delete openBook;
        openBook = nullptr;
    }
}

void IterativeDeeping::setUseBook(bool b) {
    useBook = b;
    bool valid = true;
    if (b && openBook == nullptr) {
        openBook = new OpenBook();
        valid = useBook = openBook->load("cinnamon.bin");
    }
    if ((!b && openBook) || !valid) {
        delete openBook;
        openBook = nullptr;
        useBook = false;
    }
}

void IterativeDeeping::run() {
    lock_guard<mutex> lock1(mutexIT);

    struct timeb start1;
    struct timeb end1;

    int timeTaken = 0;
    searchManager.setRunningAll(2);
    searchManager.setRunningAllThread(2);
    int mply = 0;
    if (useBook) {
        ASSERT(openBook);
        string obMove = openBook->search(searchManager.boardToFen());
        if (!obMove.empty()) {
            _Tmove move;
            searchManager.getMoveFromSan(obMove, &move);
            searchManager.makemove(&move);
            cout << "bestmove " << obMove << endl;
            return;
        }
    }
    int sc = 0;
    u64 totMoves = 0;

    mply = 0;

    searchManager.startClock();
    searchManager.clearKillerHeuristic();
    searchManager.clearAge();
    searchManager.setForceCheck(false);
    searchManager.setRunning(2);

    ftime(&start1);

    bool inMate = false;
    int extension = 0;
    string bestmove;
    string ponderMove;
    int val = 0;
    int mateIn = INT_MAX;
    string pvv;
    _Tmove resultMove;
    while (searchManager.getRunning(0) && mateIn == INT_MAX && mply < maxDepth) {
        mateIn = INT_MAX;
        totMoves = 0;
        ++mply;
        searchManager.parallelSearch(mply);
        searchManager.setRunningAllThread(1);
        if (mply == 2) {
            searchManager.setRunningAll(1);
        }

        if (!searchManager.getRes(resultMove, ponderMove, pvv)) {
            break;
        }
        searchManager.incKillerHeuristic(resultMove.from, resultMove.to, 0x800);

        ftime(&end1);
        timeTaken = _time::diffTime(end1, start1);
        totMoves += searchManager.getTotMoves();

        sc = resultMove.score;
        if (resultMove.score > _INFINITE - MAX_PLY) {
            sc = 0x7fffffff;
        }
#ifdef DEBUG_MODEkk
        int totStoreHash = nRecordHashA + nRecordHashB + nRecordHashE + 1;
        int percStoreHashA = nRecordHashA * 100 / totStoreHash;
        int percStoreHashB = nRecordHashB * 100 / totStoreHash;
        int percStoreHashE = nRecordHashE * 100 / totStoreHash;
        int totCutHash = n_cut_hashA + n_cut_hashB + n_cut_hashE + 1;
        int percCutHashA = n_cut_hashA * 100 / totCutHash;
        int percCutHashB = n_cut_hashB * 100 / totCutHash;
        int percCutHashE = n_cut_hashE * 100 / totCutHash;
        cout << "\ninfo string ply: " << mply << "\n";
        cout << "info string tot moves: " << totMoves << "\n";
        cout << "info string hash stored " << totStoreHash * 100 / (1 + cumulativeMovesCount) << "% (alpha=" << percStoreHashA << "% beta=" << percStoreHashB << "% exact=" << percStoreHashE << "%)" << endl;
        cout << "info string cut hash " << totCutHash * 100 / (1 + cumulativeMovesCount) << "% (alpha=" << percCutHashA << "% beta=" << percCutHashB << "% exact=" << percCutHashE << "%)" << endl;
        u64 nps = 0;
        if (timeTaken) {
            nps = totMoves * 1000 / timeTaken;
        }
        if (nCutAB) {
            betaEfficiencyCumulative += betaEfficiency / totGen * 10;
            cout << "info string beta efficiency: " << (int) betaEfficiencyCumulative << "%\n";
            betaEfficiency = totGen = 0.0;
        }
        cout << "info string millsec: " << timeTaken << "  (" << nps / 1000 << "k nodes per seconds) \n";
        cout << "info string alphaBeta cut: " << nCutAB << "\n";
        cout << "info string lazy eval cut: " << LazyEvalCuts << "\n";
        cout << "info string futility pruning cut: " << nCutFp << "\n";
        cout << "info string razor cut: " << nCutRazor << "\n";
        cout << "info string null move cut: " << nNullMoveCut << "\n";
        cout << "info string insufficientMaterial cut: " << nCutInsufficientMaterial << endl;
#endif
        ///is valid move?
        bool print = true;
        if (abs(sc) > _INFINITE - MAX_PLY) {
            bool b = searchManager.getForceCheck();
            u64 oldKey = searchManager.getZobristKey();
            searchManager.setForceCheck(true);
            bool valid = searchManager.makemove(&resultMove);
            if (!valid) {
                extension++;
                print = false;
            }
            searchManager.takeback(&resultMove, oldKey, true);
            searchManager.setForceCheck(b);
        }
        if (print) {
            int mateIn = searchManager.getMateIn();
            //resultMove.capturedPiece = (resultMove.side ^ 1) == WHITE ? searchManager.getPieceAt<WHITE>(POW2[resultMove.to]) : searchManager.getPieceAt<BLACK>(POW2[resultMove.to]);
            resultMove.capturedPiece = searchManager.getPieceAt(resultMove.side ^ 1, POW2[resultMove.to]);
            bestmove = Search::decodeBoardinv(resultMove.type, resultMove.from, resultMove.side);
            if (!(resultMove.type & (Search::KING_SIDE_CASTLE_MOVE_MASK | Search::QUEEN_SIDE_CASTLE_MOVE_MASK))) {
                bestmove += Search::decodeBoardinv(resultMove.type, resultMove.to, resultMove.side);
                if (resultMove.promotionPiece != -1) {
                    bestmove += tolower(FEN_PIECE[(uchar) resultMove.promotionPiece]);
                }
            }
            if (abs(sc) > _INFINITE - MAX_PLY) {
                cout << "info score mate 1 depth " << mply << " nodes " << totMoves << " time " << timeTaken << " pv " << pvv << endl;
            } else {
                cout << "info score cp " << sc << " depth " << mply - extension << " nodes " << totMoves << " time " << timeTaken << " pv " << pvv << endl;
            }
        }

        if (searchManager.getForceCheck()) {
            searchManager.setForceCheck(false);
            searchManager.setRunning(1);

        } else if (abs(sc) > _INFINITE - MAX_PLY) {
            searchManager.setForceCheck(true);
            searchManager.setRunning(2);

        }
//        if (mply >= maxDepth + extension && (searchManager.getRunning(0) != 2 || inMate)) {
//            ASSERT(0);
//            break;
//        }

        if (abs(sc) > _INFINITE - MAX_PLY) {
            inMate = true;
        }
    }

    cout << "bestmove " << bestmove;
    if (ponderEnabled && ponderMove.size()) {
        cout << " ponder " << ponderMove;
    }
    cout << "\n" << flush;
}

bool IterativeDeeping::setParameter(String param, int value) {
#if defined(CLOP) || defined(DEBUG_MODEkk)
    param.toUpper();
    bool res = true;
    if (param == "FUTIL_MARGIN") {
        FUTIL_MARGIN = value;
    } else if (param == "EXT_FUTILY_MARGIN") {
        EXT_FUTILY_MARGIN = value;
    } else if (param == "RAZOR_MARGIN") {
        RAZOR_MARGIN = value;
    } else if (param == "ATTACK_KING") {
        ATTACK_KING = value;
    } else if (param == "BACKWARD_PAWN") {
        BACKWARD_PAWN = value;
    } else if (param == "BISHOP_ON_QUEEN") {
        BISHOP_ON_QUEEN = value;
    } else if (param == "NO_PAWNS") {
        NO_PAWNS = value;
    } else if (param == "BONUS2BISHOP") {
        BONUS2BISHOP = value;
    } else if (param == "CONNECTED_ROOKS") {
        CONNECTED_ROOKS = value;
    } else if (param == "DOUBLED_ISOLATED_PAWNS") {
        DOUBLED_ISOLATED_PAWNS = value;
    } else if (param == "DOUBLED_PAWNS") {
        DOUBLED_PAWNS = value;
    } else if (param == "END_OPENING") {
        END_OPENING = value;
    } else if (param == "ENEMY_NEAR_KING") {
        ENEMY_NEAR_KING = value;
    } else if (param == "FRIEND_NEAR_KING") {
        FRIEND_NEAR_KING = value;
    } else if (param == "BISHOP_NEAR_KING") {
        BISHOP_NEAR_KING = value;
    } else if (param == "HALF_OPEN_FILE_Q") {
        HALF_OPEN_FILE_Q = value;
    } else if (param == "KNIGHT_TRAPPED") {
        KNIGHT_TRAPPED = value;
    } else if (param == "OPEN_FILE") {
        OPEN_FILE = value;
    } else if (param == "OPEN_FILE_Q") {
        OPEN_FILE_Q = value;
    } else if (param == "PAWN_7H") {
        PAWN_7H = value;
    } else if (param == "PAWN_CENTER") {
        PAWN_CENTER = value;
    } else if (param == "PAWN_IN_RACE") {
        PAWN_IN_RACE = value;
    } else if (param == "PAWN_ISOLATED") {
        PAWN_ISOLATED = value;
    } else if (param == "PAWN_NEAR_KING") {
        PAWN_NEAR_KING = value;
    } else if (param == "PAWN_BLOCKED") {
        PAWN_BLOCKED = value;
    } else if (param == "ROOK_7TH_RANK") {
        ROOK_7TH_RANK = value;
    } else if (param == "ROOK_BLOCKED") {
        ROOK_BLOCKED = value;
    } else if (param == "ROOK_TRAPPED") {
        ROOK_TRAPPED = value;
    } else if (param == "UNDEVELOPED") {
        UNDEVELOPED = value;
    } else if (param == "UNDEVELOPED_BISHOP") {
        UNDEVELOPED_BISHOP = value;
    } else if (param == "VAL_WINDOW") {
        VAL_WINDOW = value;
    } else if (param == "UNPROTECTED_PAWNS") {
        UNPROTECTED_PAWNS = value;
    } else if (param == "ENEMIES_PAWNS_ALL") {
        ENEMIES_PAWNS_ALL = value;
    } else if (param == "NULLMOVE_DEPTH") {
        NULLMOVE_DEPTH = value;
    } else if (param == "NULLMOVES_MIN_PIECE") {
        NULLMOVES_MIN_PIECE = value;
    } else if (param == "NULLMOVES_R1") {
        NULLMOVES_R1 = value;
    } else if (param == "NULLMOVES_R2") {
        NULLMOVES_R2 = value;
    } else if (param == "NULLMOVES_R3") {
        NULLMOVES_R3 = value;
    } else if (param == "NULLMOVES_R4") {
        NULLMOVES_R4 = value;
    } else {
        res = false;
    }
    return res;
#else
    cout << param << value;
    assert(0);
#endif
}

int IterativeDeeping::loadFen(string fen) {
    return searchManager.loadFen(fen);
}

void IterativeDeeping::display() {
    searchManager.display();
}

int IterativeDeeping::getHashSize() {
    return searchManager.getHashSize();
}

bool IterativeDeeping::setHashSize(int i) {
    return searchManager.setHashSize(i);
}

void IterativeDeeping::setRunning(bool i) {
    searchManager.setRunning(i);
}

void IterativeDeeping::startClock() {
    searchManager.startClock();
}

string IterativeDeeping::getFen() {
    return searchManager.getFen();
}

void IterativeDeeping::setPonder(bool i) {
    searchManager.setPonder(i);
}

int IterativeDeeping::getSide() {
    return searchManager.getSide();
}

int IterativeDeeping::getScore(int side) {
    return searchManager.getScore(side);
}

//u64 IterativeDeeping::getBitBoard() {
//    return searchManager.getBitBoard();
//}

void IterativeDeeping::clearHash() {
    searchManager.clearHash();
}

int IterativeDeeping::getMaxTimeMillsec() {
    return searchManager.getMaxTimeMillsec();
}

void IterativeDeeping::setNullMove(bool i) {
    searchManager.setNullMove(i);
}

void IterativeDeeping::makemove(_Tmove *ptr) {
    searchManager.makemove(ptr);
}

void IterativeDeeping::setSide(bool i) {
    searchManager.setSide(i);
}

int IterativeDeeping::getMoveFromSan(String string, _Tmove *ptr) {
    return searchManager.getMoveFromSan(string, ptr);
}

void IterativeDeeping::pushStackMove() {
    searchManager.pushStackMove();
}

void IterativeDeeping::init() {
    searchManager.init();
}

void IterativeDeeping::setRepetitionMapCount(int i) {
    searchManager.setRepetitionMapCount(i);
}

void IterativeDeeping::deleteGtb() {
    searchManager.deleteGtb();
}

int IterativeDeeping::printDtm() {
    return searchManager.printDtm();
}

void IterativeDeeping::createGtb() {
    tablebase = &Tablebase::getInstance();
    searchManager.setGtb(*tablebase);
}

void IterativeDeeping::setGtbPath(String path) {
    tablebase->setPath(path);
}

bool IterativeDeeping::setGtbCacheSize(int i) {
    return tablebase->setCacheSize(i);
}

bool IterativeDeeping::setGtbScheme(String s) {
    return tablebase->setScheme(s);
}

bool IterativeDeeping::setGtbInstalledPieces(int i) {
    return tablebase->setInstalledPieces(i);
}

bool IterativeDeeping::setGtbProbeDepth(int i) {
    return tablebase->setProbeDepth(i);
}

void IterativeDeeping::restartGtb() {
    tablebase->restart();
}
