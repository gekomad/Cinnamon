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

/*
echo "#pragma once" >  inc.h
cat b2.txt          >> inc.h
xxd -i w1.bin       >> inc.h
xxd -i w2.bin       >> inc.h
xxd -i b1.bin       >> inc.h
xxd -i hidden.bin   >> inc.h
 */

#include "NN.h"
#include "NN_inc.h"
#include "util/util.h"

float NN::nn_forward(const float *input, const uchar side) const {
    assert(nn);
    for (size_t j = 0; j < HIDDEN_SIZE; j++) {
        float sum = nn->b1[j];
        for (size_t i = 0; i < INPUT_SIZE; i++) {
            sum += input[i] * nn->w1[i * HIDDEN_SIZE + j];
        }
        nn->hidden[j] = tanh_activation(sum);
    }

    float out = nn->b2;
    for (size_t j = 0; j < HIDDEN_SIZE; j++) {
        out += nn->hidden[j] * nn->w2[j];
    }
    return side ? -out : out;
}

// for training
NN::NN(const string &root) : W1(root + "/w1.bin"), W2(root + "/w2.bin"), B1(root + "/b1.bin"), B2(root + "/b2.bin"),
                             B2_TEXT(root + "/b2.txt"), EPOCH_TEXT(root + "/epoch.txt"), HIDDEN(root + "/hidden.bin"),
                             INPUT_FILE(root + "/input.bin"), TARGET_FILE(root + "/target.bin"),
                             TRAINING_FILE(root + "/training.epd") {
    if (!FileUtil::fileExists(TRAINING_FILE)) {
        cout << "error file " << TRAINING_FILE << " not exists" << endl;
        exit(1);
    }
}

// for search
NN::NN() {
    nn = static_cast<NeuralNetwork *>(malloc(sizeof(NeuralNetwork)));
    nn->b1 = reinterpret_cast<float *>(neunet::b1_bin);
    nn->w1 = reinterpret_cast<float *>(neunet::w1_bin);
    nn->w2 = reinterpret_cast<float *>(neunet::w2_bin);
    nn->b2 = neunet::__b2;
    nn->hidden = reinterpret_cast<float *>(neunet::hidden_bin);
}

float NN::tanh_activation(const float x) {
    return tanhf(x);
}


#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

void NN::nn_free() const {
    free(nn->w1);
    free(nn->b1);
    free(nn->w2);
    free(nn->hidden);
    free(nn);
}


void NN::nn_new() {
    nn = static_cast<NeuralNetwork *>(xmalloc(sizeof(NeuralNetwork)));
    nn->b1 = static_cast<float *>(xmalloc(HIDDEN_SIZE * sizeof(float)));
    nn->w1 = static_cast<float *>(xmalloc(sizeof(float) * INPUT_SIZE * HIDDEN_SIZE));
    nn->w2 = static_cast<float *>(xmalloc(sizeof(float) * HIDDEN_SIZE));
    nn->b2 = 0.0f;
    nn->hidden = static_cast<float *>(xmalloc(HIDDEN_SIZE * sizeof(float)));
    srand(static_cast<unsigned int>(time(nullptr)));
    for (size_t i = 0; i < INPUT_SIZE * HIDDEN_SIZE; i++) {
        nn->w1[i] = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.1f;
    }
    for (size_t i = 0; i < HIDDEN_SIZE; i++) {
        nn->w2[i] = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.1f;
    }
}


void NN::load() { {
        nn_new();
        std::ifstream file1(W1, std::ios::binary);
        file1.read(reinterpret_cast<char *>(nn->w1), sizeof(float) * INPUT_SIZE * HIDDEN_SIZE);
        file1.close();

        std::ifstream file2(B1, std::ios::binary);
        file2.read(reinterpret_cast<char *>(nn->b1), sizeof(float) * HIDDEN_SIZE);
        file2.close();

        std::ifstream file3(W2, std::ios::binary);
        file3.read(reinterpret_cast<char *>(nn->w2), sizeof(float) * HIDDEN_SIZE);
        file3.close();

        std::ifstream file4(HIDDEN, std::ios::binary);
        file4.read(reinterpret_cast<char *>(nn->hidden), sizeof(float) * HIDDEN_SIZE);
        file4.close();

        std::ifstream file5(EPOCH_TEXT);
        file5 >> epoch;
        file5.close();
        epoch++;

        std::ifstream file6(B2);
        file6 >> nn->b2;
        file6.close();
    } {
        const size_t size = FileUtil::fileSize(TARGET_FILE);
        targets = static_cast<float *>(xmalloc(size));
        std::ifstream file(TARGET_FILE, std::ios::binary);
        file.read(reinterpret_cast<char *>(targets), sizeof(float) * HIDDEN_SIZE);
        file.close();
    } {
        const size_t size = FileUtil::fileSize(INPUT_FILE);
        inputs_size = size / (INPUT_SIZE * sizeof(float));
        inputs = static_cast<float **>(xmalloc(inputs_size * sizeof(float *)));
        std::ifstream file(INPUT_FILE, std::ios::binary);
        for (size_t ii = 0; ii < inputs_size; ii++) {
            const auto a = static_cast<float *>(xmalloc(INPUT_SIZE * sizeof(float)));
            file.read(reinterpret_cast<char *>(a), INPUT_SIZE * sizeof(float));
            inputs[ii] = a;
        }
        file.close();
    }
}

void NN::save() const {
    cout << getDateTime() << " - Save..." << endl;
    auto const w1 = "/tmp/w1";
    auto const w2 = "/tmp/w2";

    auto const b1 = "/tmp/b1";
    auto const b2 = "/tmp/b2";

    auto const hidden = "/tmp/hidden";
    auto const i = "/tmp/inputs";
    auto const t = "/tmp/target"; {
        std::ofstream file(w1, std::ios::binary);
        file.write(reinterpret_cast<const char *>(nn->w1), INPUT_SIZE * HIDDEN_SIZE * sizeof(float));
        file.close();

        std::ofstream file1(b1, std::ios::binary);
        file1.write(reinterpret_cast<const char *>(nn->b1), HIDDEN_SIZE * sizeof(float));
        file1.close();

        std::ofstream file2(w2, std::ios::binary);
        file2.write(reinterpret_cast<const char *>(nn->w2), HIDDEN_SIZE * sizeof(float));
        file2.close();

        std::ofstream file3(hidden, std::ios::binary);
        file3.write(reinterpret_cast<const char *>(nn->hidden), HIDDEN_SIZE * sizeof(float));
        file3.close();

        std::ofstream file4(b2);
        file4 << nn->b2;
        file4.close();

        std::ofstream file5(B2_TEXT);
        file5 << "float __b2 =";
        file5 << nn->b2;
        file5 << ";\n";
        file5.close();

        std::ofstream file6(EPOCH_TEXT);
        file6 << epoch;
        file6.close();
    } {
        std::ofstream file(t, std::ios::binary);
        file.write(reinterpret_cast<const char *>(targets), inputs_size * sizeof(float));
        file.close();
    } {
        std::ofstream file(i, std::ios::binary);
        _ASSERT(file);
        for (size_t ii = 0; ii < inputs_size; ii++) {
            file.write(reinterpret_cast<const char *>(inputs[ii]), INPUT_SIZE * sizeof(float));
        }
        file.close();
    }

    FileUtil::rename(i, INPUT_FILE);
    FileUtil::rename(w1, W1);
    FileUtil::rename(w2, W2);
    FileUtil::rename(b1, B1);
    FileUtil::rename(b2, B2);
    FileUtil::rename(hidden, HIDDEN);
    FileUtil::rename(t, TARGET_FILE);
}

void NN::shuffle_indices(size_t **indices, const size_t n) {
    srand(static_cast<unsigned int>(time(nullptr)));
    for (size_t i = n - 1; i > 0; i--) {
        const size_t j = rand() % (i + 1);
        const size_t temp = (*indices)[i];
        (*indices)[i] = (*indices)[j];
        (*indices)[j] = temp;
    }
}

void NN::train(ChessBoard &chessboard) {
    const auto dw1 = static_cast<float *>(xmalloc(INPUT_SIZE * HIDDEN_SIZE * sizeof(float)));
    auto *db1 = static_cast<float *>(xmalloc(HIDDEN_SIZE * sizeof(float)));
    auto *dw2 = static_cast<float *>(xmalloc(HIDDEN_SIZE * sizeof(float)));
    auto indices = static_cast<size_t *>(xmalloc(sizeof(size_t) * inputs_size));
    for (size_t i = 0; i < inputs_size; i++) indices[i] = i;

    while (true) {
        cout << getDateTime() << " - train epoch " << epoch << " ..." << endl;
        float total_loss = 0.0f;

        shuffle_indices(&indices, inputs_size);

        for (size_t batch_start = 0; batch_start < inputs_size; batch_start += BATCH_SIZE) {
            const size_t batch_end = batch_start + BATCH_SIZE < inputs_size
                                         ? batch_start + BATCH_SIZE
                                         : inputs_size;
            const size_t current_batch_size = batch_end - batch_start;

            // Reset gradienti per il batch
            memset(dw1, 0, sizeof(float) * INPUT_SIZE * HIDDEN_SIZE);
            memset(db1, 0, sizeof(float) * HIDDEN_SIZE);
            memset(dw2, 0, sizeof(float) * HIDDEN_SIZE);
            float db2 = 0.0f;

            for (size_t k = batch_start; k < batch_end; k++) {
                const size_t i = indices[k];
                const float *input = inputs[i];
                assert(chessboard.sideToMove==WHITE);
                const float output = nn_forward(input, chessboard.sideToMove);
                assert(i < inputs_size);
                const float error = output - targets[i];
                total_loss += error * error;
                const float d_out = 2.0f * error;

                db2 += d_out;
                for (size_t j = 0; j < HIDDEN_SIZE; j++) {
                    assert(j < HIDDEN_SIZE);
                    dw2[j] += d_out * nn->hidden[j];
                }

                // Hidden layer
                for (size_t j = 0; j < HIDDEN_SIZE; j++) {
                    // Derivata di tanh è 1 - tanh(x)^2. nn->hidden[j] è già tanh(sum)
                    const float d_hidden = d_out * nn->w2[j] * (1.0f - nn->hidden[j] * nn->hidden[j]);

                    db1[j] += d_hidden;
                    for (size_t k_input = 0; k_input < INPUT_SIZE; k_input++) {
                        dw1[k_input * HIDDEN_SIZE + j] += d_hidden * input[k_input];
                    }
                }
            }

            // Aggiornamento pesi (gradient descent)
            const float learning_rate_scaled = LEARNING_RATE / static_cast<float>(current_batch_size);

            for (size_t i = 0; i < INPUT_SIZE * HIDDEN_SIZE; i++) {
                nn->w1[i] -= learning_rate_scaled * dw1[i];
            }
            for (size_t i = 0; i < HIDDEN_SIZE; i++) {
                nn->b1[i] -= learning_rate_scaled * db1[i];
                nn->w2[i] -= learning_rate_scaled * dw2[i];
            }
            nn->b2 -= learning_rate_scaled * db2;
        }
        cout << getDateTime() << " - Epoch " << epoch << " | Loss: " << total_loss / static_cast<float>(inputs_size) <<
                endl;
        chessboard.loadFen("1B1bkr2/p2b1ppp/1p6/4N3/2P5/8/PP2BPPP/R3R1K1 w - - 0 18");
        const auto a = nn_forward(chessboard.boardToInput(), chessboard.sideToMove);
        cout << getDateTime() << " - 1B1bkr2/p2b1ppp/1p6/4N3/2P5/8/PP2BPPP/R3R1K1 w - - 0 18: " << a << " --> " << 9.9
                << endl;
        save();
        // calculate_mse_dataset(*nn);
        epoch++;
    }

    free(dw1);
    free(db1);
    free(dw2);
    free(indices);
}

void NN::load_dataset(ChessBoard &chessboard) {
    if (FileUtil::fileExists(W1) && FileUtil::fileExists(W2) && FileUtil::fileExists(B1)
        && FileUtil::fileExists(B2)
        && FileUtil::fileExists(HIDDEN) &&
        FileUtil::fileExists(INPUT_FILE) && FileUtil::fileExists(TARGET_FILE)) {
        cout << getDateTime() << " - Found bin files" << endl;
        load();
    } else {
        std::ifstream fp(TRAINING_FILE);
        std::string line;
        epoch = 0;
        inputs_size = 0;
        const size_t lines = FileUtil::countLines(TRAINING_FILE);
        inputs = static_cast<float **>(xmalloc(sizeof(float *) * lines));
        targets = static_cast<float *>(xmalloc(sizeof(float) * lines));
        while (std::getline(fp, line)) {
            const char *fen = strtok(const_cast<char *>(line.c_str()), ",");
            const char *eval_str = strtok(nullptr, ",");
            float eval = strtof(eval_str, nullptr);
            if (strstr(fen, " b ")) eval = -eval;
            chessboard.loadFen(fen);
            inputs[inputs_size] = chessboard.boardToInput();
            targets[inputs_size] = eval;
            inputs_size++;
        }
        fp.close();
        nn_new();
        save();

        cout << getDateTime() << " - " << inputs_size << " fen positions loaded." << endl;
    }
}

// string W1 = "/w1.bin";
// string W2 = "/w2.bin";
// string B1 = "/b1.bin";
// string B2 = "/b2.bin";
// string B2_TEXT = "/b2.txt";
// string EPOCH_TEXT = "/epoch.txt";
// string HIDDEN = "/hidden.bin";
// string INPUT_FILE = "/input.bin";
// string TARGET_FILE = "/target.bin";
// // string TRAINING_FILE = "/home/geko/a.epd";
// string TRAINING_FILE = "/home/geko/res2.epd";

void NN::train_and_save(ChessBoard &chessboard) {
    // W1 = string(root_path + "/w1.bin");
    // W2 = string(root_path + "/w2.bin");
    // B1 = string(root_path + "/b1.bin");
    // B2 = string(root_path + "/b2.bin");
    // B2_TEXT = string(root_path + "/b2.txt");
    // EPOCH_TEXT = string(root_path + "/epoch.txt");
    // HIDDEN = string(root_path + "/hidden.bin");
    // TARGET_FILE = string(root_path + "/target.bin");
    // TRAINING_FILE = string(root_path + "/training.epd");
    cout << getDateTime() << " - Neural network training use " << TRAINING_FILE << "..." << endl;

    load_dataset(chessboard);
    train(chessboard);
    for (size_t i = 0; i < inputs_size; i++) {
        free(inputs[i]);
    }
    free(inputs);
    free(targets);
    nn_free();
}
