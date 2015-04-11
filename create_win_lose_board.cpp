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

#define UNKNOWN 1
#define WIN     2
#define LOSE    3

/*
 * ファイルサイズ（バイト数）を取得
 */
unsigned long get_file_size(const char *filename) {
    int fd = open(filename, O_RDONLY);
    struct stat statbuf;
    fstat(fd, &statbuf);
    close(fd);
    return (unsigned long) statbuf.st_size;
}

/*
 * その局面が負け局面か勝ち局面か判定
 */
unsigned char get_winorlose(board b, vector<board> &all_state, map<board, unsigned char> &judge)
{
    bool all_win = true;
    vector<board> next_boards;
    unsigned int n;

    get_next_board(next_boards, b);
    n = next_boards.size();

    for (int i=0; i<n; i++) {
        board nb = next_boards[i];

        // 次の局面の内少なくとも１つが負け局面になるなら，今の局面は勝ち局面
        if (judge[nb] == LOSE || is_lose_state(nb)) {
            return WIN;
        } else if (judge[nb] == WIN || is_win_state(nb)) {
        } else if (judge[nb] == UNKNOWN) {
            all_win = false;
        } else {
            // 想定していない局面
            fprintf(stderr, "ERROR: %016ld is unpredictable board.", nb);
        }
    }

    // 次の局面が全て勝ち局面になるなら，今の局面は負け局面
    if (all_win) {
        return LOSE;
    }

    return UNKNOWN;
}

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

    // 全ての局面（末端局面を除く） all-state-removeend.dat の読込
    file_size     = get_file_size("all-state-removeend.dat");
    all_state_num = file_size / sizeof(board);
    all_state.resize(all_state_num);
    ifstream ifs("all-state.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    for (int i=0; i<all_state_num; i++) {
        board b = all_state[i];
        if (is_win_state(b)) {
            judge[b] = WIN;
            count[WIN]++;
        } else if (is_lose_state(b)) {
            judge[b] = LOSE;
            count[LOSE]++;
        } else {
            judge[b] = UNKNOWN;
            count[UNKNOWN]++;
        }
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
