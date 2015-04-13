/*
 * どうぶつしょうぎにおいて，あり得る全ての盤面を
 * 取得する．１つの盤面は64ビットで表現され，２GB
 * 強程度のファイル（all-state.dat）を出力する．
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
    vector<board> all_state;
    deque<board>  q;
    set<board>  h;
    unsigned int MAX_BOARD_NUM;

    vector<board> next_boards;

    unsigned int i, j, n;
    board b;
    set<board>::iterator iter;

    if (argc == 2) {
        MAX_BOARD_NUM = atoi(argv[1]);
    } else{
        MAX_BOARD_NUM = 10;
    }

    b = get_init_board();
    write_board(b);
    b = regulate(b);
    write_board(b);

    i = 0;
    q.push_back(b);
    while(!q.empty()) {
        if (i>=MAX_BOARD_NUM) {
            break;
        }
        b = q.front(); q.pop_front();
        iter = h.find(b);

        if (iter == h.end()) {
            all_state.push_back(b);
            h.insert(b);
            i++;
            // 終わった盤面からは解析しない
            if (!is_win_state(b) && !is_lose_state(b)) {

                next_boards.clear();
                get_next_board(next_boards, b);
                n = next_boards.size();

                for (j=0; j<n; j++) {
                    // ちゃんと正規化して，手番を合わせて保存
                    b = get_reverse(next_boards[j]);
                    b = regulate(b);
                    q.push_back(b);
                }
            }
        }
    }
    // ソート
    sort(all_state.begin(), all_state.end());

    fprintf(stderr, "num of all-state = %ld\n", all_state.size());
    ofstream ofs("all-state.dat");
    ofs.write((char *)&all_state[0],all_state.size()*sizeof(board));

    return 0;
}
