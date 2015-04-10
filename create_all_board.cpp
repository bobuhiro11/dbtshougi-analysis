#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <deque>
#include <set>
#include <algorithm>

#define HIYOKO    0x1 // (0b001)
#define KIRIN     0x2 // (0b010)
#define ZOU       0x3 // (0b011)
#define LION      0x4 // (0b100)
#define NIWATORI  0x5 // (0b101)

#define MY_CELL   0x0 // (0b0000)
#define YOUR_CELL 0x8 // (0b1000)

#define MY_HAND   0x1 // (0b01)
#define YOUR_HAND 0x2 // (0b10)

#define WIDTH      (3)
#define HEIGHT     (4)
#define HAND_NUM   (6)

using namespace std;

struct point{
    int y;
    int x;
};

#define   HIYOKO_MOVABLE_NUM (1)
#define    KIRIN_MOVABLE_NUM (4)
#define      ZOU_MOVABLE_NUM (4)
#define     LION_MOVABLE_NUM (8)
#define NIWATORI_MOVABLE_NUM (6)

/*
 * どうぶつが動ける方向（範囲）
 */
struct point   hiyoko_movable[] = {{-1,0}};
struct point    kirin_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1}};
struct point      zou_movable[] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
struct point     lion_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1},{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
struct point niwatori_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1},{-1,-1},{-1,+1}};

/*
 * MSB（最上位ビット）
 *
 * 0000 4bit
 *
 * 10(your ZOU) 00 10(your KIRIN) 01(my KIRIN) 10(your HIYOKO) 01(my HIYOKO)   12bit
 *
 * 0100(my   lion,i=0,j=0) 1001(your hiyoko) 1011(your   zou)
 * 1100(your lion)         0010(my    kirin) 1010(your kirin)
 * 1100(your lion)         0010(my    kirin) 1010(your kirin)
 * 1100(your lion)         0010(my    kirin) 1010(your kirin,i=3,j=2)    4x12=48bit
 *
 * LSB（最下位ビット）
 */
typedef uint64_t board;

/*
 * 盤面上のセルに関する操作
 */
#define INDEX(i,j)         (((WIDTH*(HEIGHT-i-1)+(WIDTH-1-j)))*4)
#define CLEAR_CELL(b,i,j)  ((b) & (~((board)0xF << INDEX((i),(j)))))
#define GET_CELL(b,i,j)    (((b) >> INDEX((i),(j))) & 0xF)
#define SET_CELL(b,i,j,c)  (CLEAR_CELL((b),(i),(j)) | (((board)c) << INDEX((i),(j))))
#define IS_EMPTY(c)        ((c) == 0x0)
#define GET_ANIMAL(c)      ((c) & 0x7)
#define GET_BELONG(c)      ((c) & 0x8)

/*
 * どうぶつのいない空いたセルの一覧
 */
void get_empty_cell(vector<struct point> &v, board b)
{
    unsigned int i,j,c;
    struct point p;

    for (i=0;i<HEIGHT;i++) {
        for (j=0;j<WIDTH;j++) {
            c = GET_CELL(b, i, j);
            if (IS_EMPTY(c)) {
                p.y = i;
                p.x = j;
                v.push_back(p);
            }
        }
    }
}

/*
 * 座標(i,j)にいるどうぶつが動ける座標の一覧
 */
void get_movable(vector<struct point> &v, board b, unsigned int i, unsigned int j)
{
    struct point p, direction;
    unsigned long c, animal, n, k, n_c;
    struct point *movable;

    c = GET_CELL(b,i,j);

    if (IS_EMPTY(c) || GET_BELONG(c) != MY_CELL)
        return;

    animal = GET_ANIMAL(c);

    n = 0;
    switch (animal) {
        case HIYOKO:   movable =   hiyoko_movable; n =   HIYOKO_MOVABLE_NUM; break;
        case KIRIN:    movable =    kirin_movable; n =    KIRIN_MOVABLE_NUM; break;
        case ZOU:      movable =      zou_movable; n =      ZOU_MOVABLE_NUM; break;
        case LION:     movable =     lion_movable; n =     LION_MOVABLE_NUM; break;
        case NIWATORI: movable = niwatori_movable; n = NIWATORI_MOVABLE_NUM; break;
        default: fprintf(stderr, "ERROR: in get_movable. b=%016lx, i=%d,j=%d.\n",b,i,j); break;
    }

    for (k=0; k<n; k++) {
        direction = movable[k];
        p.y = i + direction.y;
        p.x = j + direction.x;
        n_c = GET_CELL(b, p.y, p.x);

        if ((GET_BELONG(n_c) == YOUR_CELL || IS_EMPTY(n_c))
                && 0 <= p.x && p.x < WIDTH
                && 0 <= p.y && p.y < HEIGHT) {
            v.push_back(p);
        }
    }
}

/*
 * どうぶつを持ちゴマに加える
 */
board add_hand(board b, unsigned int animal, unsigned int belong)
{
    unsigned int offset;

    switch (animal) {
        case HIYOKO:  offset = 48; break;
        case KIRIN:   offset = 52; break;
        case ZOU:     offset = 56; break;
        default: fprintf(stderr, "ERROR: in add_hand(). animal = %x.\n", animal);break;
    }

    if (((b >> offset) & 0x3) != 0x0)
        offset += 2;

    return (b | (((board)belong & 0x3) << offset));
}

/*
 * どうぶつを持ちゴマから消す
 */
board remove_hand(board b, unsigned int animal, unsigned int belong)
{
    unsigned int offset;

    switch (animal) {
        case HIYOKO:  offset = 48 + 2; break;
        case KIRIN:   offset = 52 + 2; break;
        case ZOU:     offset = 56 + 2; break;
    }

    if (((b >> offset) & 0x3) == belong) return b & (~((board)0x3 << offset));
    else                                 return b & (~((board)0x3 << (offset-2)));
}

/*
 * 持ちゴマにあるどうぶつの数
 */
unsigned int get_hand_num(board b, unsigned int animal, unsigned int belong)
{
    unsigned int offset;
    unsigned int n = 0;

    switch (animal) {
        case HIYOKO:  offset = 48; break;
        case KIRIN:   offset = 52; break;
        case ZOU:     offset = 56; break;
    }

    if (((b >>       offset) & 0x3) == belong) n++;
    if (((b >> (offset + 2)) & 0x3) == belong) n++;

    return n;
}

/*
 * 盤面を初期化する
 */
board get_init_board(void)
{
    board b = 0x0;

    b = SET_CELL(b,0,0, (YOUR_CELL| KIRIN));
    b = SET_CELL(b,0,1, (YOUR_CELL| LION));
    b = SET_CELL(b,0,2, (YOUR_CELL| ZOU));
    b = SET_CELL(b,1,1, (YOUR_CELL| HIYOKO));
    b = SET_CELL(b,2,1, (MY_CELL  | HIYOKO));
    b = SET_CELL(b,3,0, (MY_CELL  | ZOU));
    b = SET_CELL(b,3,1, (MY_CELL  | LION));
    b = SET_CELL(b,3,2, (MY_CELL  | KIRIN));

    //b = SET_CELL(b,0,0, (YOUR_CELL| KIRIN));
    //b = SET_CELL(b,3,2, (YOUR_CELL| LION));
    //b = SET_CELL(b,1,1, (MY_CELL  | HIYOKO));
    //b = SET_CELL(b,1,0, (MY_CELL  | HIYOKO));
    //b = SET_CELL(b,3,1, (MY_CELL  | ZOU));
    //b = SET_CELL(b,3,0, (MY_CELL  | LION));

    //b = add_hand(b, KIRIN, MY_HAND);
    //b = add_hand(b, ZOU, MY_HAND);
    //b = add_hand(b, ZOU, YOUR_HAND);

    return b;
}

/*
 * 盤面を左右反転して，正規化する
 */
board regulate(board b)
{
    board new_b = b;
    int i;

    for (i=0;i<HEIGHT;i++) {
       new_b = SET_CELL(new_b, i,  WIDTH-1, GET_CELL(b, i,       0));
       new_b = SET_CELL(new_b, i,        0, GET_CELL(b, i, WIDTH-1));
    }
    return (new_b < b) ? new_b : b;
}

/*
 * 勝ち確定局面かどうか
 *
 * 勝ち確定局面 = 手番のプレイヤがあいてのライオンを捕まえられる状態
 */
int is_win_state(board b)
{
    int i,j,n;
    struct point p;
    vector<struct point> vec_movable;

    for(i=0;i<HEIGHT;i++) {
        for(j=0;j<WIDTH;j++) {
            get_movable(vec_movable, b, i, j);
        }
    }

    n = vec_movable.size();
    for(i=0;i<n;i++) {
        p = vec_movable[i];
        if (GET_CELL(b,p.y,p.x) == (YOUR_CELL | LION)) {
            return 1;
        }
    }
    return 0;
}

/*
 * 負け確定局面かどうか
 *
 * 負け確定局面 = 勝ち確定局面でなく，あいてのライオンが自陣深くにいる状態
 */
int is_lose_state(board b)
{
    int j;

    if (is_win_state(b))
        return 0;

    for(j=0;j<WIDTH;j++) {
        if (GET_CELL(b,HEIGHT-1,j) == (YOUR_CELL | LION))
            return 1;
    }
    return 0;
}

/*
 * 盤面を見やすく整形して，コンソールに出力
 */
void write_board(board b)
{
    unsigned int i,j,c,belong,animal,c_animal;

    fprintf(stderr, "======================================\n");
    fprintf(stderr, "binary expression: %016lx\n", b);
    for (i=0;i<HEIGHT;i++) {
        for (j=0;j<WIDTH;j++) {
            c = GET_CELL(b, i, j);
            if (IS_EMPTY(c)) {
                fprintf(stderr, "   ");
            } else {
                belong = GET_BELONG(c);
                animal = GET_ANIMAL(c);
                switch (animal) {
                    case HIYOKO:   c_animal = 'H'; break;
                    case KIRIN:    c_animal = 'K'; break;
                    case ZOU:      c_animal = 'Z'; break;
                    case LION:     c_animal = 'L'; break;
                    case NIWATORI: c_animal = 'N'; break;
                }
                fprintf(stderr, "%c%c ", belong == MY_CELL ?  'M' : 'Y', c_animal);
            }
        }
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "my   hand: ");
    fprintf(stderr, "H-%d ", get_hand_num(b, HIYOKO, MY_HAND));
    fprintf(stderr, "K-%d ", get_hand_num(b, KIRIN,  MY_HAND));
    fprintf(stderr, "Z-%d ", get_hand_num(b, ZOU,    MY_HAND));
    fprintf(stderr, "\nyour hand: ");
    fprintf(stderr, "H-%d ", get_hand_num(b, HIYOKO, YOUR_HAND));
    fprintf(stderr, "K-%d ", get_hand_num(b, KIRIN,  YOUR_HAND));
    fprintf(stderr, "Z-%d ", get_hand_num(b, ZOU,    YOUR_HAND));
    fprintf(stderr, "\n      win: %d\n",  is_win_state(b));
    fprintf(stderr, "     lose: %d\n", is_lose_state(b));
    fprintf(stderr, "\n");
}

/*
 * ある盤面から推移できる盤面の一覧
 *
 * todo: ゲームおわり局面からは推移しない
 */
void get_next_board(vector<board> &v, board b)
{
    vector<struct point> vec_movable, vec_empty;
    unsigned int i,j,k,c,get_animal,n,animal,n_c;
    board new_b;


    // どうぶつを動かすことで盤面を推移する
    for (i=0; i<HEIGHT; i++) {
        for (j=0; j<WIDTH; j++) {
            vec_movable.clear();
            c = GET_CELL(b,i,j);

            get_movable(vec_movable, b, i, j);
            for (k=0; k<vec_movable.size(); k++) {
                // どうぶつを移動
                n_c = (GET_ANIMAL(c) == HIYOKO && vec_movable[k].y == 0) ? (MY_CELL | NIWATORI) : c;
                new_b      = SET_CELL(CLEAR_CELL(b, i, j), vec_movable[k].y, vec_movable[k].x, n_c);

                // 新たに手に入れたどうぶつ
                get_animal = GET_ANIMAL(GET_CELL(b,vec_movable[k].y,vec_movable[k].x));
                if (get_animal == NIWATORI)
                    get_animal = HIYOKO;
                if (!IS_EMPTY(GET_CELL(b,vec_movable[k].y,vec_movable[k].x)))
                    new_b = add_hand(new_b, get_animal, MY_HAND);

                v.push_back(new_b);
            }
        }
    }

    // どうぶつを持ちゴマから打つことで盤面を推移する
    get_empty_cell(vec_empty, b);
    for (animal=HIYOKO; animal<=ZOU; animal++) {
        n = get_hand_num(b, animal, MY_HAND);
        if (n>0) {
            for (k=0; k<vec_empty.size(); k++) {
                new_b = remove_hand(b, animal, MY_HAND);
                new_b = SET_CELL(new_b, vec_empty[k].y, vec_empty[k].x, (MY_CELL | animal));
                v.push_back(new_b);
            }
        }
    }
}

board get_reverse(board b)
{
    unsigned int i,j,ni,nj,c;
    board p;
    board new_b = b;

    // 手持ちのコマも交換する
    p = ((board)0xFFF) << 48;
    new_b = p ^ new_b;

    // 0b11になっちゃったところを0b00にもどす
    for (i=0; i<6; i++) {
        if (((new_b >> (2*i + 48)) & 0x3) == 0x3) {
            new_b = new_b & (~(((board)0x3) << (2*i+48)));
        }
    }

    // 盤面のどうぶつを交換する
    for (i=0; i<HEIGHT; i++) {
        for (j=0; j<WIDTH; j++) {
            ni = HEIGHT -i -1;
            nj = WIDTH  -j -1;

            c = GET_CELL(b, i, j);
            if (!IS_EMPTY(c)) {
                new_b = SET_CELL(new_b, ni, nj, c^0x8);
            } else {
                new_b = CLEAR_CELL(new_b, ni, nj);
            }
        }
    }

    return new_b;
}


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
                    b = regulate(get_reverse(next_boards[j]));
                    q.push_back(b);
                }
            }
        }
    }

    fprintf(stderr, "num of all-state = %ld\n", all_state.size());
    ofstream ofs("all-state.dat");
    ofs.write((char *)&all_state[0],all_state.size()*sizeof(board));

    return 0;
}
