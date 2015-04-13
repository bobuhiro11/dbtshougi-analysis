/*
 * 末端局面を含めて all_state.dat を昇順ソートしておく
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

    all_state.resize(all_state_num);

    ifstream ifs("all-state.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    // ソート
    sort(all_state.begin(), all_state.end());

    ofstream ofs("all-state_sorted.dat");
    ofs.write((char *)&all_state[0], all_state.size()*sizeof(board));

    return 0;
}
