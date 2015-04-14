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

    // 全ての局面 all-state_sorted.dat の読込
    unsigned long file_size     = get_file_size("all-state_sorted.dat");
    unsigned long all_state_num = file_size / sizeof(board);
    vector <board> all_state(all_state_num);
    ifstream ifs("all-state_sorted.dat");
    ifs.read((char*)&all_state[0], all_state.size() * sizeof(board));

    // 次の局面 next_state.dat の読み込み
    vector <board> next_state(all_state_num);
    ifstream ifs2("next_state.dat");
    ifs2.read((char*)&next_state[0], next_state.size() * sizeof(board));

    vector <unsigned char> judge(all_state_num);
    ifstream ifs3("judge.dat");
    ifs3.read((char*)&judge[0], judge.size() * sizeof(unsigned char));

    vector <unsigned char> judge_count(all_state_num);
    ifstream ifs4("judge_count.dat");
    ifs4.read((char*)&judge_count[0], judge_count.size() * sizeof(unsigned char));


    board b = regulate(get_init_board());

    int turn = 0;

    // 最も長く続く手順を出力
    for (size_t i=0; i<200; i++) {
        write_board(b);
        fprintf(stderr, "i=%d, turn=%s\n", i, turn==0?"first":"second");

        size_t index = bin_search(all_state, b);
        fprintf(stderr, "index = %d\n", index);
        fprintf(stderr, "judge = %d (@%d) \n", judge[index], judge_count[index]);
        board nb = next_state[index];

        if (nb == 0) {
            printf("this is end point\n");
        }

        b = regulate(nb);
        turn = 1-turn;
    }


    return 0;
}
