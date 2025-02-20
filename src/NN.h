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

#include <math.h>
#include "ChessBoard.h"


constexpr int MAX_LINE_LEN = 128;
constexpr int BOARD_SIZE = 64;
constexpr int INPUT_SIZE = 12 * BOARD_SIZE + 1 + 1 + 1;
constexpr int CASTLE_IDX = 12 * BOARD_SIZE;
constexpr int ENPASSANT_IDX = CASTLE_IDX + 1;
constexpr int HIDDEN_SIZE = 128;
constexpr int LEARNING_RATE = 0.001f;
constexpr int BATCH_SIZE = 32;

typedef struct {
    float *w1; // INPUT_SIZE * HIDDEN_SIZE
    float *b1; // HIDDEN_SIZE
    float *w2; // HIDDEN_SIZE
    float *hidden; // HIDDEN_SIZE
    float b2;
} NeuralNetwork;

class NN {
public:
    explicit NN(const string &root);

    NN();

    void train_and_save(ChessBoard &chessboard);

    void load();

    float nn_forward(const float *input, const uchar side) const;

private:
    NeuralNetwork *nn = nullptr;
    float **inputs = nullptr;
    float *targets = nullptr;
    size_t inputs_size = 0;
    size_t epoch = 1;

    void nn_free() const;

    void nn_new();

    void save() const;

    static float tanh_activation(float x);

    static void shuffle_indices(size_t **indices, size_t n);

    void train(ChessBoard &chessboard);

    void load_dataset(ChessBoard &chessboard);

    string W1;
    string W2;
    string B1;
    string B2;
    string B2_TEXT;
    string EPOCH_TEXT;
    string HIDDEN;
    string INPUT_FILE;
    string TARGET_FILE;
    // string TRAINING_FILE = "/home/geko/a.epd";
    string TRAINING_FILE;
};
