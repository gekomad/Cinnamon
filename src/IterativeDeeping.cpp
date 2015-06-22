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
    search.setMaxTimeMillsec(i);
}

void IterativeDeeping::setMaxDepth(int d) {
    maxDepth = d;
}

bool IterativeDeeping::getGtbAvailable() {
    return search.getGtbAvailable();
}

IterativeDeeping::~IterativeDeeping() {
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

Tablebase &IterativeDeeping::getGtb() {
    search.createGtb();
    return search.getGtb();
}

int IterativeDeeping::printDtm() {
    int side = search.getSide();
    u64 friends = side == WHITE ? search.getBitBoard<WHITE>() : search.getBitBoard<BLACK>();
    u64 enemies = side == BLACK ? search.getBitBoard<WHITE>() : search.getBitBoard<BLACK>();
    search.display();
    int res = side ? getGtb().getDtm<WHITE, true>(search.chessboard, search.rightCastle, 100) : getGtb().getDtm<BLACK, true>(search.chessboard, search.rightCastle, 100);
    cout << " res: " << res;
    search.incListId();
    search.generateCaptures(side, enemies, friends);
    search.generateMoves(side, friends | enemies);
    _Tmove *move;
    u64 oldKey = 0;
    cout << "\n succ. \n";
    int best = -_INFINITE;
    for (int i = 0; i < search.getListSize(); i++) {
        move = &search.gen_list[search.listId].moveList[i];
        search.makemove(move, false, false);
        cout << "\n" << search.decodeBoardinv(move->type, move->from, search.getSide()) << search.decodeBoardinv(move->type, move->to, search.getSide()) << " ";
        res = side ? -getGtb().getDtm<BLACK, true>(search.chessboard, search.rightCastle, 100) : getGtb().getDtm<WHITE, true>(search.chessboard, search.rightCastle, 100);
        if (res != -INT_MAX) {
            cout << " res: " << res;
        }
        cout << "\n";
        search.takeback(move, oldKey, false);
        if (res > best) {
            best = res;
        }
    }
    if (best > 0) {
        best = _INFINITE - best;
    } else if (best < 0) {
        best = -(_INFINITE - best);
    }
    cout << endl;
    search.decListId();
    return best;
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
    lock_guard<mutex> lock(mutex1);
    _Tmove resultMove;
    struct timeb start1;
    struct timeb end1;

    _TpvLine line;
    int val = 0, tmp;
    string pvv;
    _Tmove move2;
    int TimeTaken = 0;
    search.setRunning(2);
    int mply = 0;
    if (useBook) {
        ASSERT(openBook);
        string obMove = openBook->search(search.boardToFen());
        if (!obMove.empty()) {
            _Tmove move;
            search.getMoveFromSan(obMove, &move);
            search.makemove(&move);
            cout << "bestmove " << obMove << endl;
            return;
        }
    }
    int sc = 0;
    u64 totMoves = 0;
    string ponderMove;
    val = 0;
    mply = 0;
    search.startClock();
    search.clearKillerHeuristic();
    search.clearAge();
    ftime(&start1);
    memset(&resultMove, 0, sizeof(resultMove));
    ponderMove = "";
    int mateIn = INT_MAX;
    search.setForceCheck(false);
    bool inMate = false;
    int extension = 0;
    string bestmove;
    while (search.getRunning() && mateIn == INT_MAX) {
        search.init();
        ++mply;
        search.setMainPly(mply);
        if (mply == 1) {
            memset(&line, 0, sizeof(_TpvLine));
            mateIn = INT_MAX;
            search.search(mply, -_INFINITE, _INFINITE, &line, &mateIn);
            search.start();
            search.join();
            val = search.getValue();
        } else {
            memset(&line, 0, sizeof(_TpvLine));
            mateIn = INT_MAX;
            search.search(mply, val - VAL_WINDOW, val + VAL_WINDOW, &line, &mateIn);
            search.start();
            search.join();
            tmp = search.getValue();
            if (tmp <= val - VAL_WINDOW || tmp >= val + VAL_WINDOW) {
                memset(&line, 0, sizeof(_TpvLine));
                mateIn = INT_MAX;
                search.search(mply, val - VAL_WINDOW * 2, val + VAL_WINDOW * 2, &line, &mateIn);
                search.start();
                search.join();
                tmp = search.getValue();
            }
            if (tmp <= val - VAL_WINDOW * 2 || tmp >= val + VAL_WINDOW * 2) {
                memset(&line, 0, sizeof(_TpvLine));
                mateIn = INT_MAX;
                search.search(mply, val - VAL_WINDOW * 4, val + VAL_WINDOW * 4, &line, &mateIn);
                search.start();
                search.join();
                tmp = search.getValue();
            }
            if (tmp <= val - VAL_WINDOW * 4 || tmp >= val + VAL_WINDOW * 4) {
                memset(&line, 0, sizeof(_TpvLine));
                mateIn = INT_MAX;
                search.search(mply, -_INFINITE, _INFINITE, &line, &mateIn);
                search.start();
                search.join();
                tmp = search.getValue();
            }
            val = tmp;
        }
        if (!search.getRunning()) {
            break;
        }
        totMoves = 0;
        if (mply == 2) {
            search.setRunning(1);
        }
        memcpy(&move2, line.argmove, sizeof(_Tmove));
        pvv.clear();
        string pvvTmp;
        for (int t = 0; t < line.cmove; t++) {
            pvvTmp.clear();
            pvvTmp += search.decodeBoardinv(line.argmove[t].type, line.argmove[t].from, search.getSide());
            if (pvvTmp.length() != 4) {
                pvvTmp += search.decodeBoardinv(line.argmove[t].type, line.argmove[t].to, search.getSide());
            }
            pvv += pvvTmp;
            if (t == 1) {
                ponderMove = pvvTmp;
            }
            pvv += " ";
        };
        memcpy(&resultMove, &move2, sizeof(_Tmove));
        search.incKillerHeuristic(resultMove.from, resultMove.to, 0x800);
        ftime(&end1);
        TimeTaken = _time::diffTime(end1, start1);
        totMoves += search.getTotMoves();
        if (!pvv.length()) {
            break;
        }
        sc = resultMove.score;
        if (resultMove.score > _INFINITE - MAX_PLY) {
            sc = 0x7fffffff;
        }
#ifdef DEBUG_MODE
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
        if (TimeTaken) {
            nps = totMoves * 1000 / TimeTaken;
        }
        if (nCutAB) {
            betaEfficiencyCumulative += betaEfficiency / totGen * 10;
            cout << "info string beta efficiency: " << (int) betaEfficiencyCumulative << "%\n";
            betaEfficiency = totGen = 0.0;
        }
        cout << "info string millsec: " << TimeTaken << "  (" << nps / 1000 << "k nodes per seconds) \n";
        cout << "info string alphaBeta cut: " << nCutAB << "\n";
        cout << "info string lazy eval cut: " << LazyEvalCuts << "\n";
        cout << "info string futility pruning cut: " << nCutFp << "\n";
        cout << "info string razor cut: " << nCutRazor << "\n";
        cout << "info string null move cut: " << nNullMoveCut << "\n";
        cout << "info string insufficientMaterial cut: " << nCutInsufficientMaterial << endl;
#endif
        ///is invalid move?
        bool print = true;
        if (abs(sc) > _INFINITE - MAX_PLY) {
            bool b = search.getForceCheck();
            u64 oldKey = search.zobristKey;
            search.setForceCheck(true);
            bool valid = search.makemove(&resultMove);
            if (!valid) {
                extension++;
                print = false;
            }
            search.takeback(&resultMove, oldKey, true);
            search.setForceCheck(b);
        }
        if (print) {
            resultMove.capturedPiece = (resultMove.side ^ 1) == WHITE ? search.getPieceAt<WHITE>(POW2[resultMove.to]) : search.getPieceAt<BLACK>(POW2[resultMove.to]);
            bestmove = search.decodeBoardinv(resultMove.type, resultMove.from, resultMove.side);
            if (!(resultMove.type & (Search::KING_SIDE_CASTLE_MOVE_MASK | Search::QUEEN_SIDE_CASTLE_MOVE_MASK))) {
                bestmove += search.decodeBoardinv(resultMove.type, resultMove.to, resultMove.side);
                if (resultMove.promotionPiece != -1) {
                    bestmove += tolower(FEN_PIECE[(uchar) resultMove.promotionPiece]);
                }
            }
            if (abs(sc) > _INFINITE - MAX_PLY) {
                cout << "info score mate 1 depth " << mply << " nodes " << totMoves << " time " << TimeTaken << " pv " << pvv << endl;
            } else {
                cout << "info score cp " << sc << " depth " << mply - extension << " nodes " << totMoves << " time " << TimeTaken << " pv " << pvv << endl;
            }
        }
        if (search.getForceCheck()) {
            search.setForceCheck(false);
            search.setRunning(1);
        } else if (abs(sc) > _INFINITE - MAX_PLY) {
            search.setForceCheck(true);
            search.setRunning(2);
        }
        if (mply >= maxDepth + extension && (search.getRunning() != 2 || inMate)) {
            break;
        }
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
#if defined(CLOP) || defined(DEBUG_MODE)
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
