/*
 * どうぶつしょうぎにおいて，勝ち確定局面と
 * 負け確定局面を出力する．
 * all-state_sorted.dat を入力として，judge
 * .dat と judje_count.dat を出力する．
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <deque>
#include <set>
#include <map>
#include <algorithm>
#include "common.h"

int main(int argc, char *argv[])
{
    printf("program started.\n");

    // 全ての局面 all-state_sorted.dat の読込
    unsigned long file_size = get_file_size("all-state_sorted.dat");
    unsigned long all_state_num = file_size / sizeof(board);
    vector<board> all_state(all_state_num);
    ifstream ifs("all-state_sorted.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    // 勝敗判定結果 judge.dat と judge_count.dat の読み込み
    vector<unsigned char> judge(all_state_num);
    ifstream ifs2("judge.dat");
    ifs2.read((char*)&judge[0], judge.size() * sizeof(unsigned char));

    vector<unsigned char> judge_count(all_state_num);
    ifstream ifs3("judge_count.dat");
    ifs3.read((char*)&judge_count[0], judge_count.size() * sizeof(unsigned char));

    // 次の局面
    vector<board> next_state(all_state_num,0);

    for (size_t i=0; i<all_state_num; i++) {
        board b = all_state[i];

        if (i % 10000000 == 0) {
            printf("%d\%%\n", (100 * i) / all_state_num);
        }

        if (judge[i] == WIN) {
            if (judge_count[i] == 0) {
                // 勝ち，末端局面 => これ以降はライオンを取るだけ（自明）なので，"0"を返す
                next_state[i] = 0;
            } else {
                // 勝ち，非末端局面 => 次の負け状態の中で最もjudge_countの小さい局面を返す
                vector<board> next_boards;
                get_next_board(next_boards, b);
                int min = INT_MAX, n=next_boards.size();
                board nb;

                for (size_t j=0; j<n; j++) {
                    board nnb = regulate(get_reverse(next_boards[j]));
                    size_t index = bin_search(all_state, nnb);
                    if (judge[index] == LOSE && judge_count[index] < min) {
                        nb = nnb;
                        min = judge_count[index];
                    }
                }

                next_state[i] = nb;
            }
        } else if (judge[i] == LOSE) {
            if (judge_count[i] == 0) {
                // 負け，末端局面 => もう既に死んでいるので，"0"を返す
                next_state[i] = 0;
            } else {
                // 負け，非末端局面 => 次の局面の中で最もjudge_countの大きい局面を返す
                vector<board> next_boards;
                get_next_board(next_boards, b);
                int max = INT_MIN, n=next_boards.size();
                board nb;

                for (size_t j=0; j<n; j++) {
                    board nnb = regulate(get_reverse(next_boards[j]));
                    size_t index = bin_search(all_state, nnb);
                    if (judge_count[index] < max) {
                        nb = nnb;
                        max = judge_count[index];
                    }
                }

                next_state[i] = nb;
            }
        } else if (judge[i] == UNKNOWN) {
            // 引き分け，非末端局面 => 次の引き分け局面の中で適当にうつ
            vector<board> next_boards;
            get_next_board(next_boards, b);
            vector<board> next_draw_boards;
            int n = next_boards.size();

            for (size_t j=0; j<n; j++) {
                board nnb = regulate(get_reverse(next_boards[j]));
                size_t index = bin_search(all_state, nnb);
                if (judge[index] == UNKNOWN) {
                    next_draw_boards.push_back(nnb);
                }
            }

            next_state[i] = next_draw_boards[ rand() % next_draw_boards.size() ];
        } else {
            fprintf(stderr, "ERROR: invalid state\n");
        }
    }

    // 結果の出力
    ofstream ofs1("next_state.dat");
    ofs1.write((char *)&next_state[0], next_state.size()*sizeof(board));

    return 0;
}
