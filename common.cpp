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
#include "common.h"

struct point   hiyoko_movable[] = {{-1,0}};
struct point    kirin_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1}};
struct point      zou_movable[] = {{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
struct point     lion_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1},{-1,-1},{-1,+1},{+1,-1},{+1,+1}};
struct point niwatori_movable[] = {{-1,0},{+1,0},{0,-1},{0,+1},{-1,-1},{-1,+1}};

/*
 * $B$I$&$V$D$N$$$J$$6u$$$?%;%k$N0lMw(B
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
 * $B:BI8(B(i,j)$B$K$$$k$I$&$V$D$,F0$1$k:BI8$N0lMw(B
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
 * $B$I$&$V$D$r;}$A%4%^$K2C$($k(B
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
 * $B$I$&$V$D$r;}$A%4%^$+$i>C$9(B
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
 * $B;}$A%4%^$K$"$k$I$&$V$D$N?t(B
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
 * $BHWLL$r=i4|2=$9$k(B
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

    return b;
}

/*
 * $BHWLL$r@55,2=$9$k(B
 * - $B:81&$rH?E>(B
 * - $B<j;}$A$b@55,2=(B
 * - $B$G$-$k$@$1>.$5$JCM$K(B
 */
board regulate(board b)
{
    board new_b = b;
    int i;

    // $BHWLL$r=i4|2=(B
    for (i=0;i<HEIGHT;i++) {
       new_b = SET_CELL(new_b, i,  WIDTH-1, GET_CELL(b, i,       0));
       new_b = SET_CELL(new_b, i,        0, GET_CELL(b, i, WIDTH-1));
    }
    new_b = (new_b < b) ? new_b : b;
    new_b = new_b & 0xFFFFFFFFFFFF;

    // $B;}$A%4%^$r=i4|2=(B
    int my_hiyoko   = get_hand_num(b, HIYOKO, MY_HAND);
    int my_kirin    = get_hand_num(b, KIRIN,  MY_HAND);
    int my_zou      = get_hand_num(b, ZOU,    MY_HAND);
    int your_hiyoko = get_hand_num(b, HIYOKO, YOUR_HAND);
    int your_kirin  = get_hand_num(b, KIRIN,  YOUR_HAND);
    int your_zou    = get_hand_num(b, ZOU,    YOUR_HAND);

    for (i=0;i<your_hiyoko;i++) new_b = add_hand(new_b, HIYOKO, YOUR_HAND);
    for (i=0;i<your_kirin;i++)  new_b = add_hand(new_b, KIRIN,  YOUR_HAND);
    for (i=0;i<your_zou;i++)    new_b = add_hand(new_b, ZOU,    YOUR_HAND);
    for (i=0;i<my_hiyoko;i++)   new_b = add_hand(new_b, HIYOKO, MY_HAND);
    for (i=0;i<my_kirin;i++)    new_b = add_hand(new_b, KIRIN,  MY_HAND);
    for (i=0;i<my_zou;i++)      new_b = add_hand(new_b, ZOU,    MY_HAND);

    return new_b;
}

/*
 * $B>!$A3NDj6ILL$+$I$&$+(B
 *
 * $B>!$A3NDj6ILL(B = $B<jHV$N%W%l%$%d$,$"$$$F$N%i%$%*%s$rJa$^$($i$l$k>uBV(B
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
 * $BIi$13NDj6ILL$+$I$&$+(B
 *
 * $BIi$13NDj6ILL(B = $B>!$A3NDj6ILL$G$J$/!$$"$$$F$N%i%$%*%s$,<+?X?<$/$K$$$k>uBV(B
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
 * $BHWLL$r8+$d$9$/@07A$7$F!$%3%s%=!<%k$K=PNO(B
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
 * $B$"$kHWLL$+$i?d0\$G$-$kHWLL$N0lMw(B
 */
void get_next_board(vector<board> &v, board b)
{
    vector<struct point> vec_movable, vec_empty;
    unsigned int i,j,k,c,get_animal,n,animal,n_c;
    board new_b;


    // $B$I$&$V$D$rF0$+$9$3$H$GHWLL$r?d0\$9$k(B
    for (i=0; i<HEIGHT; i++) {
        for (j=0; j<WIDTH; j++) {
            vec_movable.clear();
            c = GET_CELL(b,i,j);

            get_movable(vec_movable, b, i, j);
            for (k=0; k<vec_movable.size(); k++) {
                // $B$I$&$V$D$r0\F0(B
                n_c = (GET_ANIMAL(c) == HIYOKO && vec_movable[k].y == 0) ? (MY_CELL | NIWATORI) : c;
                new_b      = SET_CELL(CLEAR_CELL(b, i, j), vec_movable[k].y, vec_movable[k].x, n_c);

                // $B?7$?$K<j$KF~$l$?$I$&$V$D(B
                get_animal = GET_ANIMAL(GET_CELL(b,vec_movable[k].y,vec_movable[k].x));
                if (get_animal == NIWATORI)
                    get_animal = HIYOKO;
                if (!IS_EMPTY(GET_CELL(b,vec_movable[k].y,vec_movable[k].x)))
                    new_b = add_hand(new_b, get_animal, MY_HAND);

                v.push_back(new_b);
            }
        }
    }

    // $B$I$&$V$D$r;}$A%4%^$+$iBG$D$3$H$GHWLL$r?d0\$9$k(B
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

    // $B<j;}$A$N%3%^$b8r49$9$k(B
    p = ((board)0xFFF) << 48;
    new_b = p ^ new_b;

    // 0b11$B$K$J$C$A$c$C$?$H$3$m$r(B0b00$B$K$b$I$9(B
    for (i=0; i<6; i++) {
        if (((new_b >> (2*i + 48)) & 0x3) == 0x3) {
            new_b = new_b & (~(((board)0x3) << (2*i+48)));
        }
    }

    // $BHWLL$N$I$&$V$D$r8r49$9$k(B
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