#pragma once

#ifndef JS_MODE

#include "syzygy/SYZYGY.h"
#include "../util/Singleton.h"
#include "../Search.h"
#include "gaviota/GTB.h"

class TB {
private:
    TB() {};
public:
    static int probeWdl(const int depth, const uchar side, const int N_PIECE, const int mainDepth,
                        uchar rightCastle, const _Tchessboard &chessboard) {
        SYZYGY *syzygy = &SYZYGY::getInstance();

//    if (N_PIECE == 3 &&  depth > 2 && depth != mainDepth && (board[WHITE] || board[BLACK])) {
//        const int kw = BITScanForward(board[KING_WHITE]);
//        const int kb = BITScanForward(board[KING_BLACK]);
//        const int pawnPos = BITScanForward(board[PAWN_BLACK]) | BITScanForward(board[PAWN_WHITE]);
//
//        const int winSide = board[PAWN_BLACK] ? BLACK : WHITE;
//
//        if (isDraw(winSide, side, kw, kb, pawnPos)) {
//            if (winSide == side) return -_INFINITE + depth;
//            else return _INFINITE - depth;
//        } else {
//            if (winSide == side) return _INFINITE - depth;
//            else return -_INFINITE + depth;
//        }
//    }
        if (N_PIECE < 8 && depth > 2 && depth != mainDepth) {
            int tbResult = INT_MAX;
            //syzygy
            const unsigned res = syzygy->SZtbProbeWDL(chessboard, side, N_PIECE);
            if (res != TB_RESULT_FAILED) {
                tbResult = TB_GET_WDL(res);
            }
            unsigned pliestomate;
            //gaviota
            if (GTB::getInstance().isInstalledPieces(N_PIECE)) {
                BENCH_START("gtbTime")
                tbResult = GTB::getInstance().getDtmWdl(GTB_STM(side), 0, chessboard, &pliestomate, false, rightCastle);
                BENCH_STOP("gtbTime")
            }
            switch (tbResult) {
                case TB_LOSS :
                    return -_INFINITE + depth;
                case TB_BLESSED_LOSS :
                    return -_INFINITE + depth + 50;
                case TB_WIN :
                    return _INFINITE - depth;
                case TB_CURSED_WIN :
                    return _INFINITE - depth - 50;
            }
        }
        return INT_MAX;
    }

    static bool probeRootTB(_Tmove *res, GenMoves &genMoves) {

        const _Tchessboard &chessboard = genMoves.chessboard;
        SYZYGY *syzygy = &SYZYGY::getInstance();
        const u64 white = board::getBitmap<WHITE>(chessboard);
        const u64 black = board::getBitmap<BLACK>(chessboard);
        const auto tot = bitCount(white | black);

        const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
        uchar oldEnpassant = genMoves.enPassant;

        //gaviota
        if (GTB::getInstance().isInstalledPieces(tot)) {
            u64 friends = genMoves.sideToMove == WHITE ? white : black;
            u64 enemies = genMoves.sideToMove == BLACK ? white : black;
            _Tmove *bestMove = nullptr;
            genMoves.incListId();
            genMoves.generateCaptures(genMoves.sideToMove, enemies, friends);
            genMoves.generateMoves(genMoves.sideToMove, friends | enemies);

            _Tmove *drawMove = nullptr;
            _Tmove *worstMove = nullptr;

            int minDtz = -1;
            unsigned maxDtzWorst = 1000;

            const u64 allPieces = white | black;

            unsigned dtz;

            for (int i = 0; i < genMoves.getListSize(); i++) {
                _Tmove *move = &genMoves.genList[genMoves.listId].moveList[i];
                if (!genMoves.makemove(move, false)) {
                    genMoves.takeback(move, oldKey, oldEnpassant, false);
                    continue;
                }
                BENCH_START("gtbTime")
                const auto res = GTB::getInstance().getDtmWdl(GTB_STM(X(genMoves.sideToMove)), 0, chessboard, &dtz,
                                                              true,
                                                              genMoves.rightCastle);
                BENCH_STOP("gtbTime")
                if (res == TB_WIN && !worstMove && !drawMove) {
                    if ((int) dtz > minDtz) {
                        bestMove = move;
                        minDtz = dtz;
                        if (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))
                            minDtz++;
                    }
                } else if (res == TB_DRAW) {
                    if (!drawMove || (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))) {
                        drawMove = move;
                    }
                } else if (res == TB_LOSS) {
                    if ((int) dtz < (int) maxDtzWorst) {
                        worstMove = move;
                        maxDtzWorst = dtz;
                        if (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))
                            maxDtzWorst--;
                    }
                }
                genMoves.takeback(move, oldKey, oldEnpassant, false);
            }

            if (worstMove) {
                debug("worstMove ***********\n")
                memcpy(res, worstMove, sizeof(_Tmove));
                if (res->promotionPiece != NO_PROMOTION)
                    res->promotionPiece = FEN_PIECE[res->promotionPiece];
                genMoves.decListId();
                return true;
            }

            if (drawMove) {
                debug("drawMove ***********\n")
                memcpy(res, drawMove, sizeof(_Tmove));
                if (res->promotionPiece != NO_PROMOTION)res->promotionPiece = FEN_PIECE[res->promotionPiece];
                genMoves.decListId();
                return true;
            }
            if (bestMove) {
                debug("best ***********\n")
                memcpy(res, bestMove, sizeof(_Tmove));
                if (res->promotionPiece != NO_PROMOTION)
                    res->promotionPiece = FEN_PIECE[res->promotionPiece];
                genMoves.decListId();
                return true;
            }

            genMoves.decListId();
            return false;
        }

        if (syzygy->getInstalledPieces() >= tot) {

            unsigned bestMove = 0;
            unsigned bestMove50 = 0;
            unsigned worstMove = 0;
            unsigned worstMove50 = 0;
            unsigned drawMove = 0;
            unsigned minDtz = 1000;
            unsigned minDtz50 = 1000;
            int maxDtzWorst = -1;
            int maxDtzWorst50 = -1;
            unsigned results[TB_MAX_MOVES];

            const u64 allPieces = white | black;
            BENCH_START("syzygyTime")
            const auto sz = syzygy->SZtbProbeRoot(white, black, chessboard, genMoves.sideToMove, results);
            BENCH_STOP("syzygyTime")
            if (sz == TB_RESULT_FAILED) return false;

            for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {

                const unsigned dtz = TB_GET_DTZ(results[i]) * 10;
                const unsigned res = TB_GET_WDL(results[i]);

                if (res == TB_WIN) {
                    if (dtz < minDtz) {
                        bestMove = results[i];
                        minDtz = dtz;
                        if (TB_GET_PROMOTES(bestMove) ||
                            board::isOccupied(_decodeSquare[TB_GET_TO(bestMove)], allPieces))
                            minDtz--;
                    }
                } else if (res == TB_CURSED_WIN && !bestMove) {
                    if (dtz < minDtz50) {
                        bestMove50 = results[i];
                        minDtz50 = dtz;
                        if (TB_GET_PROMOTES(bestMove50) ||
                            board::isOccupied(_decodeSquare[TB_GET_TO(bestMove50)], allPieces))
                            minDtz50--;
                    }
                } else if (res == TB_DRAW) {
                    if (!drawMove ||
                        (TB_GET_PROMOTES(results[i]) ||
                         board::isOccupied(_decodeSquare[TB_GET_TO(results[i])], allPieces))) {
                        drawMove = results[i];
                    }
                } else if (res == TB_BLESSED_LOSS && !bestMove && !drawMove && !bestMove50) {
                    if ((int) dtz > maxDtzWorst50) {
                        worstMove50 = results[i];
                        maxDtzWorst50 = dtz;
                        if (TB_GET_PROMOTES(worstMove50) ||
                            board::isOccupied(_decodeSquare[TB_GET_TO(worstMove50)], allPieces))
                            maxDtzWorst50++;
                    }
                } else if (res == TB_LOSS && !bestMove && !drawMove && !bestMove50 && !worstMove50) {
                    if ((int) dtz > maxDtzWorst) {
                        worstMove = results[i];
                        maxDtzWorst = dtz;
                        if (TB_GET_PROMOTES(worstMove) ||
                            board::isOccupied(_decodeSquare[TB_GET_TO(worstMove)], allPieces))
                            maxDtzWorst++;
                    }
                }
            }

            res->type = GenMoves::STANDARD_MOVE_MASK;

            if (bestMove) {
                debug("best ***********\n")
                res->from = _decodeSquare[TB_GET_FROM(bestMove)];
                res->to = _decodeSquare[TB_GET_TO(bestMove)];
                res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove));
                return true;
            }
            if (bestMove50) {
                debug("bestMove50 ***********\n")
                res->from = _decodeSquare[TB_GET_FROM(bestMove50)];
                res->to = _decodeSquare[TB_GET_TO(bestMove50)];
                res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove50));
                return true;
            }
            if (drawMove) {
                debug("drawMove ***********\n")
                res->from = _decodeSquare[TB_GET_FROM(drawMove)];
                res->to = _decodeSquare[TB_GET_TO(drawMove)];
                res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(drawMove));
                return true;
            }
            if (worstMove50) {
                debug("worstMove50 ***********\n")
                res->from = _decodeSquare[TB_GET_FROM(worstMove50)];
                res->to = _decodeSquare[TB_GET_TO(worstMove50)];
                res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove50));
                return true;
            }
            if (worstMove) {
                debug("worstMove ***********\n")
                res->from = _decodeSquare[TB_GET_FROM(worstMove)];
                res->to = _decodeSquare[TB_GET_TO(worstMove)];
                res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove));
                return true;
            }
            return false;
        }
        return false;
    }

    static string probeRootTB1(GenMoves &genMoves) {
        _Tmove bestMove;

        if (probeRootTB(&bestMove, genMoves)) {
            string best = string(
                    genMoves.decodeBoardinv(&bestMove, genMoves.sideToMove));

            return best;
        } else
            return "";
    }

    static int SZtbProbeWDL(GenMoves &genMoves) {
        SYZYGY *syzygy = &SYZYGY::getInstance();
        const _Tchessboard &chessboard = genMoves.chessboard;
        const auto tot = bitCount(board::getBitmap<WHITE>(chessboard) | board::getBitmap<BLACK>(chessboard));
        return syzygy->SZtbProbeWDL(chessboard, genMoves.sideToMove, tot);
    }

    static void printWdlSyzygy(GenMoves &genMoves) {
        genMoves.perftMode = true;
        const _Tchessboard &chessboard = genMoves.chessboard;
        SYZYGY *syzygy = &SYZYGY::getInstance();
        u64 friends = genMoves.sideToMove == WHITE ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(
                chessboard);
        u64 enemies = genMoves.sideToMove == BLACK ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(
                chessboard);

        genMoves.incListId();
        genMoves.generateCaptures(genMoves.sideToMove, enemies, friends);
        genMoves.generateMoves(genMoves.sideToMove, friends | enemies);
        _Tmove *move;
        u64 oldKey = chessboard[ZOBRISTKEY_IDX];
        uchar oldEnpassant = genMoves.enPassant;
        genMoves.display();

        unsigned res = syzygy->SZtbProbeWDL(chessboard, genMoves.sideToMove);
        cout << "current: ";
        if (res != TB_RESULT_FAILED) {
            res = TB_GET_WDL(res);
            if (res == TB_DRAW)cout << " draw" << endl;
            else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
                cout << " loss" << endl;
            else
                cout << " win" << endl;

        } else
            cout << " none" << endl;

        for (int i = 0; i < genMoves.getListSize(); i++) {
            move = &genMoves.genList[genMoves.listId].moveList[i];
            if (!genMoves.makemove(move, false)) {
                genMoves.takeback(move, oldKey, oldEnpassant, false);
                continue;
            }
            genMoves.print(move);

            unsigned res = syzygy->SZtbProbeWDL(chessboard, X(genMoves.sideToMove));

            if (res != TB_RESULT_FAILED) {
                res = TB_GET_WDL(res);
                if (res == TB_DRAW)cout << " draw" << endl;
                else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
                    cout << " win" << endl;
                else
                    cout << " loss" << endl;

            } else
                cout << " none" << endl;
            genMoves.takeback(move, oldKey, oldEnpassant, false);
        }
        cout << endl;
        genMoves.decListId();
    }

    static void printDtzSyzygy(GenMoves &genMoves) {
        genMoves.perftMode = true;
        SYZYGY *syzygy = &SYZYGY::getInstance();
        if ((genMoves.sideToMove == BLACK) ? board::inCheck1<WHITE>(genMoves.chessboard) : board::inCheck1<BLACK>(
                genMoves.chessboard)) {
            cout << "invalid position" << endl;
            return;
        }

        unsigned results[TB_MAX_MOVES];

        const u64 white = board::getBitmap<WHITE>(genMoves.chessboard);
        const u64 black = board::getBitmap<BLACK>(genMoves.chessboard);

        unsigned res = syzygy->SZtbProbeWDL(genMoves.chessboard, genMoves.sideToMove);
        genMoves.display();
        cout << "current: ";
        if (res != TB_RESULT_FAILED) {
            res = TB_GET_WDL(res);
            if (res == TB_DRAW)cout << " draw" << endl;
            else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
                cout << " loss" << endl;
            else
                cout << " win" << endl;

        } else
            cout << " none" << endl;

        const unsigned res1 = syzygy->SZtbProbeRoot(white, black, genMoves.chessboard, genMoves.sideToMove, results);
        if (res1 != TB_RESULT_FAILED) {
            for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {

                string from = BOARD[_decodeSquare[TB_GET_FROM(results[i])]];
                string to = BOARD[_decodeSquare[TB_GET_TO(results[i])]];

                cout << from << to;
                auto a = TB_GET_PROMOTES(results[i]);
                if (a)cout << SYZYGY::getPromotion(a);

                cout << " ";
                unsigned dtz = TB_GET_DTZ(results[i]);
                unsigned res = TB_GET_WDL(results[i]);
                if (res == TB_DRAW) cout << " draw" << endl;
                else if (res == TB_LOSS)
                    cout << " loss dtz: " << dtz << endl;
                else if (res == TB_BLESSED_LOSS)
                    cout << " loss but 50-move draw dtz: " << dtz << endl;
                else if (res == TB_WIN)
                    cout << " win dtz: " << dtz << endl;
                else if (res == TB_CURSED_WIN)
                    cout << " win but 50-move draw dtz: " << dtz << endl;

            }
            cout << endl;
        }
    }

    static int printDtmWdlGtb(GenMoves &genMoves, const bool dtm) {
        genMoves.perftMode = true;

        u64 friends =
                genMoves.sideToMove == WHITE ? board::getBitmap<WHITE>(genMoves.chessboard) : board::getBitmap<BLACK>(
                        genMoves.chessboard);
        u64 enemies =
                genMoves.sideToMove == BLACK ? board::getBitmap<WHITE>(genMoves.chessboard) : board::getBitmap<BLACK>(
                        genMoves.chessboard);
        genMoves.display();
        cout << "current: ";
        unsigned pliestomate;
        const int res = GTB::getInstance().getDtmWdl(GTB_STM (genMoves.sideToMove), 2, genMoves.chessboard,
                                                     &pliestomate, dtm,
                                                     genMoves.rightCastle);
        genMoves.incListId();
        genMoves.generateCaptures(genMoves.sideToMove, enemies, friends);
        genMoves.generateMoves(genMoves.sideToMove, friends | enemies);
        _Tmove *move;
        u64 oldKey = genMoves.chessboard[ZOBRISTKEY_IDX];
        uchar oldEnpassant = genMoves.enPassant;
        for (int i = 0; i < genMoves.getListSize(); i++) {
            move = &genMoves.genList[genMoves.listId].moveList[i];
            if (!genMoves.makemove(move, false)) {
                genMoves.takeback(move, oldKey, oldEnpassant, false);
                continue;
            }
            genMoves.print(move);

            GTB::getInstance().getDtmWdl(GTB_STM(X(genMoves.sideToMove)), 1, genMoves.chessboard, &pliestomate, dtm,
                                         genMoves.rightCastle);

            genMoves.takeback(move, oldKey, oldEnpassant, false);

        }
        cout << endl;
        genMoves.decListId();
        return res;
    }

};

#endif
    
