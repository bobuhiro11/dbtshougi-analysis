#ifndef COMMON_H
#define COMMON_H

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
extern struct point   hiyoko_movable[];
extern struct point    kirin_movable[];
extern struct point      zou_movable[];
extern struct point     lion_movable[];
extern struct point niwatori_movable[];

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

void get_empty_cell(vector<struct point> &v, board b);
void get_movable(vector<struct point> &v, board b, unsigned int i, unsigned int j);
board add_hand(board b, unsigned int animal, unsigned int belong);
board remove_hand(board b, unsigned int animal, unsigned int belong);
unsigned int get_hand_num(board b, unsigned int animal, unsigned int belong);
board get_init_board(void);
board regulate(board b);
int is_win_state(board b);
int is_lose_state(board b);
void write_board(board b);
void get_next_board(vector<board> &v, board b);
board get_reverse(board b);

#endif
