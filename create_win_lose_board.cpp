/*
 * どうぶつしょうぎにおいて，勝ち確定局面と
 * 負け確定局面を出力する．
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
    unsigned long MAX_ITER_NUM = 0;
    unsigned long file_size, all_state_num, iter_num;
    vector<board> all_state;
    map<board, unsigned char> judge;

    uint64_t count[4] = {-1,0,0,0};
    iter_num = 0;
    srand((unsigned)time(NULL));

    // 繰り返し操作の上限
    if (argc == 2) {
        MAX_ITER_NUM = atoi(argv[1]);
    }

    // 全ての局面（末端局面を除く） all-state_removeend.dat の読込
    file_size     = get_file_size("all-state_removeend.dat");
    all_state_num = file_size / sizeof(board);
    all_state.resize(all_state_num);
    ifstream ifs("all-state_removeend.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    for (int i=0; i<all_state_num; i++) {
        board b = all_state[i];
        judge[b] = UNKNOWN;
        count[UNKNOWN]++;
    }

    // 繰り返し勝敗データを更新していく
    for(;;) {
        unsigned int win_add_num = 0;
        unsigned int lose_add_num = 0;
        if (MAX_ITER_NUM != 0 && iter_num >= MAX_ITER_NUM) {
            break;
        }

        for (int i=0; i<all_state_num; i++) {
            board b = all_state[i];
            if (judge[b] == UNKNOWN) {
                unsigned char winorlose = get_winorlose(all_state[i], all_state, judge);

                if (winorlose == WIN) {
                    judge[b] = WIN;
                    count[WIN]++; count[UNKNOWN]--;
                    win_add_num++;
                } else if(winorlose == LOSE) {
                    judge[b] = LOSE;
                    count[LOSE]++; count[UNKNOWN]--;
                    lose_add_num++;
                }

            }
        }

        printf("---------------------\n", iter_num);
        printf("iter_num: %d\n", iter_num);
        printf("win_add_num: %d\n", win_add_num);
        printf("lose_add_num: %d\n", lose_add_num);

        if (win_add_num == 0 && lose_add_num == 0) {
            break;
        }

        iter_num++;
    }

    // 結果を出力
    printf("---------------------\n", iter_num);
    printf("size = %ld\n", all_state.size());
    printf("win_num = %d\n", count[WIN]);
    printf("lose_num = %d\n", count[LOSE]);
    printf("unknown_num = %d\n", count[UNKNOWN]);

    ofstream ofs_win, ofs_lose;
    ofs_win.open("win.dat",ios::out|ios::binary);
    ofs_lose.open("lose.dat",ios::out|ios::binary);

    for (int i=0; i<all_state_num; i++) {
        board b = all_state[i];
        if (judge[b] == WIN) {
            ofs_win.write((char *) &b, sizeof(board));
        } else if (judge[b] == LOSE) {
            ofs_lose.write((char *) &b, sizeof(board));
        }
    }

    ofs_win.close();
    ofs_lose.close();

    return 0;
}
