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
    unsigned long file_size, all_state_num, iter_num;
    vector<board> all_state;

    uint64_t count[4] = {0,0,0,0};
    uint64_t count4created[4] = {0,0,0,0};
    iter_num = 1;

    printf("program started.\n");

    // 全ての局面 all-state_sorted.dat の読込
    file_size     = get_file_size("all-state_sorted.dat");
    all_state_num = file_size / sizeof(board);
    all_state.resize(all_state_num);
    ifstream ifs("all-state_sorted.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    printf("read all-state_sorted\n");

    // 勝敗判定の結果，INDEXは all_state 内の局面の位置
    vector<unsigned char> judge(all_state_num,0);

    // 勝敗判定が着くまでの手数
    vector<unsigned char> judge_count(all_state_num,0);

    // 繰り返し操作の上限
    unsigned long MAX_ITER_NUM = 0;
    unsigned long MAX_ITER_STATE_NUM = all_state_num;
    if (argc == 3) {
        MAX_ITER_NUM = atoi(argv[1]);
        MAX_ITER_STATE_NUM = atoi(argv[2]);
    }

    // 初期化
    for (size_t i=0; i<MAX_ITER_STATE_NUM; i++) {
        board b = all_state[i];
        if (is_win_state(b)) {
            judge[i] = WIN;
            count[WIN]++;
        } else if (is_lose_state(b)) {
            judge[i] = LOSE;
            count[LOSE]++;
        } else {
            judge[i] = UNKNOWN;
            count[UNKNOWN]++;
        }
        judge_count[i] = 0;
    }


    printf("begin create win lose baord\n");

    // 繰り返し勝敗データを更新していく
    for(;;) {
        vector<unsigned char> judge_new(judge);

        unsigned int win_add_num = 0;
        unsigned int lose_add_num = 0;
        if (MAX_ITER_NUM != 0 && iter_num > MAX_ITER_NUM) {
            break;
        }

        for (size_t i=0; i<MAX_ITER_STATE_NUM; i++) {
            board b = all_state[i];
            if (judge[i] == UNKNOWN) {
                unsigned char winorlose = get_winorlose(b, all_state, judge);

                if (winorlose == WIN) {
                    judge_new[i] = WIN;
                    judge_count[i] = iter_num;
                    count[WIN]++;
                    count[UNKNOWN]--;
                    win_add_num++;
                } else if(winorlose == LOSE) {
                    judge_new[i] = LOSE;
                    judge_count[i] = iter_num;
                    count[LOSE]++;
                    count[UNKNOWN]--;
                    lose_add_num++;
                }

            }
        }

        count4created[LOSE] += lose_add_num;
        count4created[WIN]  += win_add_num;

        printf("---------------------\n");
        printf("iter_num: %ld\n", iter_num);
        printf("win_add_num: %d\n", win_add_num);
        printf("lose_add_num: %d\n", lose_add_num);
        printf("count4created[win] = %ld\n", count4created[WIN]);
        printf("count4created[lose] = %ld\n", count4created[LOSE]);

        judge.swap(judge_new);

        if (win_add_num == 0 && lose_add_num == 0) {
            break;
        }

        iter_num++;
    }

    // 結果を出力
    printf("---------------------\n");
    printf("size = %ld\n", all_state.size());
    printf("win_num = %ld\n", count[WIN]);
    printf("lose_num = %ld\n", count[LOSE]);
    printf("count4created[win] = %ld\n", count4created[WIN]);
    printf("count4created[lose] = %ld\n", count4created[LOSE]);
    printf("unknown_num = %ld\n", count[UNKNOWN]);
    printf("iter_num = %ld\n", iter_num);

    ofstream ofs1("judge.dat");
    ofs1.write((char *)&judge[0], judge.size()*sizeof(unsigned char));

    ofstream ofs2("judge_count.dat");
    ofs2.write((char *)&judge_count[0], judge_count.size()*sizeof(unsigned char));

    return 0;
}
