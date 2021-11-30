#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PvP 0  //pvpモードの時は1にする
#define EvE 1  //自動解析モードの時は1にする. どっちも0ならpve
#define CPU_DEPTH 14

#define P1 0
#define P2 1
#define Q 0
#define R 1
#define B 2
#define INF 99999999

#define N_PLAYER 2
#define N_TYPE_PIECE 3
#define N_SQUARE 9
#define N_LINE 8

#define MAX_IN 32
#define MAX_HISTORY 1024
#define MAX_BOARD 60480


typedef struct Board {
    short bb[N_PLAYER];
    short pos[N_PLAYER][N_TYPE_PIECE];
} board;


void init (void);
int8_t count_bit (short bb);
void print_board (board Aboard);
int8_t user_turn (board *Aboard, int8_t player);
int8_t cpu_turn (board *Aboard, int8_t player);
int8_t cpu_turn_rand (board *Aboard, int8_t player);

void user_action (board *Aboard,  int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t));
void cpu_P1_first (board *Aboard);
void cpu_action (board *Aboard, int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t));
void cpu_action_rand (board *Aboard, int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t));
board int_to_board (int num);
int board_to_int (board Aboard);
int eval_board (board Aboard, int8_t player, int depth, int alpha, int beta);

int8_t drop_piece (board *Aboard, int8_t player, int8_t type, int8_t place);
int8_t move_piece (board *Aboard, int8_t player, int8_t type, int8_t to);
int8_t is_line (short bb);
int8_t cannot_move (board Aboard, int8_t player);
void save_history (void);

const board inited_board = {
    {0,0},
    {{-1,-1,-1}, {-1,-1,-1}}
};
const char type_to_char[N_TYPE_PIECE] = {'Q','R','B'};
const short dist_piece[N_TYPE_PIECE][N_SQUARE] = {
    {0x001a,0x003d,0x0032,0x00d3,0x01ef,0x0196,0x0098,0x0178,0x00b0},
    {0x000a,0x0015,0x0022,0x0051,0x00aa,0x0114,0x0088,0x0150,0x00a0},
    {0x0010,0x0028,0x0010,0x0082,0x0145,0x0082,0x0010,0x0028,0x0010}
};
const short line[N_LINE] = {0x0007,0x0038,0x01c0,0x0049,0x0092,0x0124,0x0111,0x0054};
const short permu[N_PLAYER*N_TYPE_PIECE] = {6720,840,120,20,4,1};


char history[MAX_HISTORY];
int n_history;
int8_t is_three[512];
int8_t value_board[MAX_BOARD];


int main()
{
    init();
    board Mboard = inited_board;
    
    /*
    for (int i = 0; i < 10000; i++) {
        board Mboard = inited_board;
        while(1) {
            printf("%d\n",i);
            if (cpu_turn(&Mboard, P1)) break;
            if (cpu_turn_rand(&Mboard, P2)) return 0;
        }
    }
     */
    
    
    if (PvP) {
        print_board(Mboard);
        while(1) {
            if (user_turn(&Mboard, P1)) break;
            if (user_turn(&Mboard, P2)) break;
        }
    }
    
    else if(EvE) {
        print_board(Mboard);
        while(1) {
            if (cpu_turn(&Mboard, P1)) break;
            if (cpu_turn(&Mboard, P2)) break;
        }
    }
    
    else {
        printf("\n\nSelect Order\n"
               "1:First\n"
               "2:Second\n");
        char in[MAX_IN];
        do scanf("%s", in); while (strcmp(in,"1") && strcmp(in, "2"));

        print_board(Mboard);
        printf("\n");
        while(1) {
            if (in[0] == '1') {
                if (user_turn(&Mboard, P1)) break;
                if (cpu_turn(&Mboard, P2)) break;
            }
            else {
                if (cpu_turn(&Mboard, P1)) break;
                if (user_turn(&Mboard, P2)) break;
            }
        }
    }
     
     
    //save_history();
}

void init()
{
    for (int i = 0; i < 512; i++) {
        if (count_bit(i) == 3) is_three[i] = 1;
    }
    
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < MAX_BOARD; i++) {
        value_board[i] = 0;
        board temp = int_to_board(i);
        if (cannot_move(temp, P2)) value_board[i] = 1;
        if (cannot_move(temp, P1)) value_board[i] = -1;
        
        if (is_line(temp.bb[P1])) value_board[i] = 1;
        if (is_line(temp.bb[P2])) value_board[i] = -1;
    }
}

int8_t count_bit (short bb)
{
    bb = (bb & 0x5555) + ((bb>>1) & 0x5555);
    bb = (bb & 0x3333) + ((bb>>2) & 0x3333);
    bb = (bb & 0x0f0f) + ((bb>>4) & 0x0f0f);
    bb = (bb & 0x00ff) + ((bb>>8) & 0x00ff);
    return bb;
}

void print_board (board Aboard)
{
    char c_board[N_SQUARE];
    for (int i = 0; i < N_SQUARE; i++) c_board[i] = '.';
    
    if (Aboard.pos[P1][Q] >= 0) c_board[Aboard.pos[P1][Q]] = 'Q';
    if (Aboard.pos[P1][R] >= 0) c_board[Aboard.pos[P1][R]] = 'R';
    if (Aboard.pos[P1][B] >= 0) c_board[Aboard.pos[P1][B]] = 'B';
    if (Aboard.pos[P2][Q] >= 0) c_board[Aboard.pos[P2][Q]] = 'q';
    if (Aboard.pos[P2][R] >= 0) c_board[Aboard.pos[P2][R]] = 'r';
    if (Aboard.pos[P2][B] >= 0) c_board[Aboard.pos[P2][B]] = 'b';
    
    printf("\n           * Position *\n\n");
    for (int i = 0; i < N_SQUARE; i++) {
        if (i%3 == 0) printf("  ");
        printf("%c ", c_board[i]);
        if (i%3 == 2) printf("      %d %d %d\n", i-2,i-1,i);
    }
    printf("\n");
    
    
    for (int i = 0; i < N_PLAYER; i++) {
        printf("Player%d ", i+1);
        for (int j = 0; j < N_TYPE_PIECE; j++) {
            if (Aboard.pos[i][j] == -1) {
                if (i==P1) printf("%c ", type_to_char[j]);
                else printf("%c ", type_to_char[j]+32);
            }
            else printf("  ");
        }
        printf("\n");
    }
    printf("\n\n");
}

//ユーザーのターン. ゲームが終了する場合1を, 継続する場合0を返す
int8_t user_turn (board *Aboard, int8_t player)
{
    if (is_three[Aboard->bb[player]]) {
        printf("- Player%d Move Phase\n", player+1);
        user_action(Aboard, player, move_piece);
    }
    else {
        printf("- Player%d drop Phase\n", player+1);
        user_action(Aboard, player, drop_piece);
    }
    print_board(*Aboard);
    printf("\n");
    
    if (is_line(Aboard->bb[player]) || cannot_move(*Aboard, !player)) {
        printf("- Player%d Win \n\n", player+1);
        return 1;
    }
    return 0;
}

//コンピュータのターン
int8_t cpu_turn (board *Aboard, int8_t player)
{
    //最初の1手のみ例外処理
    if (player==P1 && count_bit(Aboard->bb[player])==0) cpu_P1_first(Aboard);
    else {
        if (is_three[Aboard->bb[player]]) cpu_action(Aboard, player, move_piece);
        else cpu_action(Aboard, player, drop_piece);
    }
    print_board(*Aboard);
    
    if (is_line(Aboard->bb[player]) || cannot_move(*Aboard, !player)) {
        printf("- Player%d Win \n\n", player+1);
        return 1;
    }
    return 0;
}

int8_t cpu_turn_rand (board *Aboard, int8_t player)
{
    if (is_three[Aboard->bb[player]]) cpu_action_rand(Aboard, player, move_piece);
    else cpu_action_rand(Aboard, player, drop_piece);
    print_board(*Aboard);
    
    if (is_line(Aboard->bb[player]) || cannot_move(*Aboard, !player)) {
        printf("- Player%d Win \n\n", player+1);
        return 1;
    }
    return 0;
}



void user_action (board *Aboard, int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t))
{
    int8_t type, place;
    char in[MAX_IN];
    
    while(1) {
        while(1) {
            printf("Type(q/r/b): ");
            scanf("%s", in);
            if (!strcmp(in, "q")) {type=Q; break;}
            if (!strcmp(in, "r")) {type=R; break;}
            if (!strcmp(in, "b")) {type=B; break;}
        }
        
        while(1) {
            printf("Place: ");
            scanf("%s", in);
            if ('0'<=in[0] && in[0]<='8' && in[1]=='\0') {
                place = in[0] - '0';
                break;
            }
        }
        
        if (action(Aboard, player, type, place)) break;
        else printf("\n- Error\n");
    }
}


void cpu_P1_first (board *Aboard)
{
    
    const int8_t place[3] = {0,1,4};
    
    int maxi = -INF, maxi_i = 0, maxi_j = 0;
    for (int i = 0; i < N_TYPE_PIECE; i++) {
        for (int j = 0; j < 3; j++) {
            board temp = *Aboard;
            if (!drop_piece(&temp, P1, i, place[j])) continue;
            
            int v = eval_board(temp, P2, CPU_DEPTH, -INF, INF);
            if (maxi < v) {maxi = v;  maxi_i = i;  maxi_j = place[j];}
        }
    }
    if (maxi > 0) printf("!\n");
    if (maxi < 0) printf("><");
    drop_piece(Aboard, P1, maxi_i, maxi_j);
     
     
    
    //drop_piece(Aboard, P1, 1, 4);
}

void cpu_action (board *Aboard, int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t))
{
    if (player == P1) {
        int maxi = -INF, maxi_i = 0, maxi_j = 0;
        for (int i = 0; i < N_TYPE_PIECE; i++) {
            for (int j = 0; j < N_SQUARE; j++) {
                board temp = *Aboard;
                if (!action(&temp, player, i, j)) continue;
                
                int v = eval_board(temp, !player, CPU_DEPTH, -INF, INF);
                if (maxi < v) {maxi = v;  maxi_i = i;  maxi_j = j;}
            }
        }
        if (maxi > 0) printf("!\n");
        if (maxi < 0) printf("><");
        action(Aboard, player, maxi_i, maxi_j);
    }
    
    else {
        int mini = INF, mini_i = 0, mini_j = 0;
        for (int i = 0; i < N_TYPE_PIECE; i++) {
            for (int j = 0; j < N_SQUARE; j++) {
                board temp = *Aboard;
                if (!action(&temp, player, i, j)) continue;
                
                int v = eval_board(temp, !player, CPU_DEPTH, -INF, INF);
                if (v < mini) {mini = v;  mini_i = i;  mini_j = j;}
            }
        }
        if (mini < 0) printf("!\n");
        if (mini > 0) printf("><");
        action(Aboard, player, mini_i, mini_j);
    }
}

void cpu_action_rand (board *Aboard, int8_t player, int8_t (*action)(board*, int8_t, int8_t, int8_t))
{
    int8_t type, place;
    do {
        type = rand() % N_TYPE_PIECE;
        place = rand() % N_SQUARE;
    }
    while (!action(Aboard, player, type, place));
}


//num: 0~60479
board int_to_board (int num)
{
    int8_t unused_num[N_SQUARE] = {0,1,2,3,4,5,6,7,8};
    board ret = inited_board;
    
    for (int i = 0; i < N_PLAYER; i++) {
        for (int j = 0; j < N_TYPE_PIECE; j++) {
            
            int n = i * N_TYPE_PIECE + j;
            int quo = num / permu[n];
            int place = unused_num[quo];
            ret.pos[i][j] = place;
            ret.bb[i] |= 1<<place;
            
            int n_loop = N_SQUARE-1-n;
            for (int k = 0; k < n_loop; k++) {
                if (unused_num[k] >= place) unused_num[k] = unused_num[k+1];
            }
            num %= permu[n];
        }
    }
    return ret;
}

int board_to_int (board Aboard)
{
    int8_t unused_num[N_SQUARE] = {0,1,2,3,4,5,6,7,8};
    int ret = 0;
    
    for (int i = 0; i < N_PLAYER; i++) {
        for (int j = 0; j < N_TYPE_PIECE; j++) {
            
            int n = i * N_TYPE_PIECE + j;
            int k = 0;
            while (Aboard.pos[i][j] != unused_num[k]) ++k;
            ret += permu[n] * k;
            
            int n_loop = N_SQUARE-1-n;
            for (k = 0; k < n_loop; k++) {
                if (unused_num[k] >= Aboard.pos[i][j]) unused_num[k] = unused_num[k+1];
            }
        }
    }
    return ret;
}

//盤面を評価する関数. P1必勝なら正数を, P2必勝なら負数を, それ以外なら0を返す
//playerは与えられたAboardにおける手番を表す
int eval_board (board Aboard, int8_t player, int depth, int alpha, int beta)
{
    int8_t can_move = is_three[Aboard.bb[player]];
    
    //P2が3手目待機状態の例外処理
    if (!can_move && player==P2 && is_line(Aboard.bb[!player])) return depth+1;
    
    if (can_move) {
        int cur_value = value_board[board_to_int(Aboard)];
        if (depth == 0 || cur_value != 0) return cur_value * (depth+1);
    }
    
    if (player == P1) {
        int maxi = -INF;
        for (int i = 0; i < N_TYPE_PIECE; i++) {
            for (int j = 0; j < N_SQUARE; j++) {
                board temp = Aboard;
                if (can_move) {
                    if (!move_piece(&temp, player, i, j)) continue;
                } else {
                    if (!drop_piece(&temp, player, i, j)) continue;
                }
                int v = eval_board(temp, !player, depth-1, alpha, beta);
                if (maxi < v) maxi = v;
                
                if (alpha < v) alpha = v;
                if (beta <= alpha) return maxi;
            }
        }
        return maxi;
    }
    else {
        int mini = INF;
        for (int i = 0; i < N_TYPE_PIECE; i++) {
            for (int j = 0; j < N_SQUARE; j++) {
                board temp = Aboard;
                if (can_move) {
                    if (!move_piece(&temp, player, i, j)) continue;
                } else {
                    if (!drop_piece(&temp, player, i, j)) continue;
                }
                int v = eval_board(temp, !player, depth-1, alpha, beta);
                if (v < mini) mini = v;
                
                if (v < beta) beta = v;
                if (beta <= alpha) return mini;
            }
        }
        return mini;
    }
}

//駒の追加. 成功したら1, 失敗したら0を返す
int8_t drop_piece (board *Aboard, int8_t player, int8_t type, int8_t place)
{
    short both_bb = Aboard->bb[0] | Aboard->bb[1];
    if (both_bb & (1<<place)) return 0;
    if (Aboard->pos[player][type] >= 0) return 0;
    
    Aboard->bb[player] |= 1<<place;
    Aboard->pos[player][type] = place;
    return 1;
}

//駒の移動. 成功したら1, 失敗したら0を返す
int8_t move_piece (board *Aboard, int8_t player, int8_t type, int8_t to)
{
    short b_bb = Aboard->bb[0] | Aboard->bb[1];
    if (b_bb & (1<<to)) return 0;
    
    int8_t from = Aboard->pos[player][type];
    if (dist_piece[type][from] & (1<<to)) {
        Aboard->bb[player] &= ~(1<<from);
        Aboard->bb[player] |= 1<<to;
        Aboard->pos[player][type] = to;
        return 1;
    }
    else return 0;
}

int8_t is_line (short bb)
{
    for (int i = 0; i < N_LINE; i++) {
        if (bb == line[i]) return 1;
    }
    return 0;
}

int8_t cannot_move (board Aboard, int8_t player)
{
    short b_bb = Aboard.bb[0] | Aboard.bb[1];
    short all_dist = dist_piece[Q][Aboard.pos[player][Q]] | dist_piece[R][Aboard.pos[player][R]] | dist_piece[B][Aboard.pos[player][B]];
    
    if ((b_bb ^ all_dist) & all_dist) return 0;
    else return 1;
}

void save_history ()
{
    FILE *file;
    file = fopen("history.txt", "w");
    
    for (int i = 0; i < n_history; i++) {
        fprintf(file, "%c ", history[i]);
        if (i%4==3) fprintf(file, "\n");
    }
    fprintf(file, "\n\n");
    
    fclose(file);
}
