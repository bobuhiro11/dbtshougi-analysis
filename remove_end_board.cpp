/*
 * 末端局面を削除する
 * all-state.dat を入力し，
 * all-state_removeend.dat
 * を出力する．
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
    // 末端局面を含む all-state.dat を読込
    unsigned long file_size     = get_file_size("all-state.dat");
    unsigned long all_state_num = file_size / sizeof(board);
    vector<board> all_state;
    vector<board> all_state_removeend;

    all_state.resize(all_state_num);
    ifstream ifs("all-state.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    // 削除処理
    for (int i=0; i<all_state_num; i++) {
        board b = all_state[i];
        if (!is_win_state(b) && !is_lose_state(b)) {
            all_state_removeend.push_back(b);
        }
    }

    // ソート
    sort(all_state_removeend.begin(), all_state_removeend.end());

    // 末端局面を含まない all-state-removeend.dat に書込
    // 今後は all_state_removeend.dat を使って解析していく
    fprintf(stderr, "num of all-state-removeend = %ld\n", all_state_removeend.size());
    ofstream ofs("all-state_removeend.dat");
    ofs.write((char *)&all_state_removeend[0],all_state_removeend.size()*sizeof(board));

    return 0;
}
