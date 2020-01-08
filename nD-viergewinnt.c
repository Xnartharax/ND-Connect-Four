#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#include "datastructures.h"
#define N_DIMS 100
#define SIZE 5
#define N_PLAYER 2
#define N_AXES (pow(2, N_DIMS)-1)

unsigned int make_lin_index(int coords[]){
    unsigned int lin_index = 0;
    for (int i = 0; i < N_DIMS; i++)
    {
        lin_index += coords[i]*ipow(SIZE, N_DIMS-1-i);
    }
    return lin_index;
}
int ipow(int base, int exp)
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}
hashlist_t player_owned[N_PLAYER];
int current_player = 1;
int cursors[N_PLAYER][N_DIMS];
int moving_dim[N_PLAYER];
int coords_moving_token[N_DIMS];
int moving_token = 0;
int compare_coords(void * coords1, void * coords2){
    int* co1 = (int *) coords1;
    int* co2 = (int *) coords2;
    for (int i = 0; i < N_DIMS; i++)
    {
        if(co1[i] != co2[i]){
            return 0;
        }
    }
    return 1;
    
}
unsigned int hash_coords(void* coords){
    int* co = (int *) coords;
    unsigned int hash = 0;
    for (int i = 0; i < N_DIMS; i++)
    {
        hash += co[i]*ipow(SIZE, i);
    }
    return hash;

    
}
void setup(){
    for (int i = 0; i < N_PLAYER; i++)
    {
        moving_dim[i] = 1;
    }
    
    for (int i = 0; i < N_PLAYER; i++)
    {
        for (int j = 1; j < N_DIMS; j++)
        {
            cursors[i][j] = SIZE/2;
        }
        cursors[i][0] = 0;
        player_owned[i].chain_array = (linked_list_t *) malloc(10*sizeof(linked_list_t));
        player_owned[i].length = 10;
        player_owned[i].hash = &hash_coords;
        player_owned[i].compare = &compare_coords;
    }
    start_color();
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(10, COLOR_BLACK, COLOR_YELLOW);
}
int check_win_rec(int dim, int coords[]){
    if (dim <= N_DIMS-1){
        for (int i = 0; i< SIZE; i++){
            // coords array kopieren.
            int new_coords[N_DIMS];
            for (int d= 0;d<dim;d++){
                new_coords[d] = coords[d];
            }
            new_coords[dim] = i;
            int win = check_win_rec(dim+1, new_coords);
            if (win == 1){
                return 1;
            }
        }
        return 0;
    }
    else{
        int win = check_win_coords(coords);
        return win;
    }
}
int get_coords(int coords[]){
    for(int i = 0; i<N_DIMS;i++){
        if (coords[i] >= SIZE || coords[i] < 0){
            return 0;
        }
    }
    for (int i = 0; i < N_PLAYER; i++)
    {
        if(lookup_hashlist(&player_owned[i], coords) == 1){
            return i+1;
        }
    }
    return 0;
}
void set_coords(int coords[], int n){
    int * coords_append = (int *) malloc(N_DIMS*sizeof(int));
    for (int i = 0; i < N_DIMS; i++)
    {
        coords_append[i] = coords[i];
    }
    
    append_to_hashlist(&player_owned[n-1], (void *) coords_append);
}
int check_win_coords(int coords[]){
    // really bad algortihm expeonential complexity but no other way around I think
    for (int i = 0; i < N_AXES; i++)
    {
        int axis = i+1; //equal or zero mask
        int max_row = 0;
        int new_coords[N_DIMS];
        for (int s = 0; s<=N_AXES;s++){ //s sign mask
            for(int j = 0; j < 4; j++){
                
                for(int k = 0; k < N_DIMS; k++){
                    if((s >> k) & 1 == 1) 
                        new_coords[k] = coords[k] + j*((axis >> k) & 1); //signmask positive
                    else
                        new_coords[k] = coords[k] - j*((axis >> k) & 1); //signmask negative
                    
                }
                if(get_coords(new_coords) == current_player){
                    max_row++;
                    if(max_row == 4){
                        return 1;
                    }
                }
                else{
                    max_row = 0;
                    break;
                }
            }
        }
    }
    return 0;
}
int check_win(){
    int coords[N_DIMS];
    return check_win_rec(0, coords);
}
int is_cursor(int coords[]){
    for(int i = 0;i<N_DIMS;i++){
        if(coords[i] != cursors[current_player-1][i]){
            return 0;
        }
    }
    return 1;
}
int is_moving_token(int coords[]){
    if(moving_token==0){
        return 0;
    }
    for(int i = 0;i<N_DIMS;i++){
        if(coords[i] != coords_moving_token[i]){
            return 0;
        }
    }
    return 1;
}
void print_coords(int coords[]){
    if(is_cursor(coords) == 1 || is_moving_token(coords) == 1){
        attron(COLOR_PAIR(current_player));
        printw(" # ");
        attroff(COLOR_PAIR(current_player));
        return;
    }
    int token = get_coords(coords);
    attron(COLOR_PAIR(token));
    printw(" # ");
    attroff(COLOR_PAIR(token)); 
}
void display(){
    clear();
    int coords[N_DIMS];
    for(int i = 2;i<N_DIMS;i++){
        coords[i] = cursors[current_player-1][i];
    }
    for(int y = 0;y < SIZE;y++){
        for(int x = 0; x < SIZE; x++){
            coords[0] = y;
            coords[1] = x;
            print_coords(coords);
        }
        printw("\n");
    }
    printw("Spieler %d: ", current_player);
    for (int i = 0; i < N_DIMS; i++)
    {  
        if(i == moving_dim[current_player-1]){
            attron(COLOR_PAIR(10));
            printw("%d ", cursors[current_player-1][i]);
            attroff(COLOR_PAIR(10));
        }
        else
            printw("%d ", cursors[current_player-1][i]);
    }
    
    refresh();
}
void handle_input(){
    char c = getch();
    if(c=='a' && moving_dim[current_player-1]>1) moving_dim[current_player-1]--; 
    else if(c=='d' && moving_dim[current_player-1]<N_DIMS-1) moving_dim[current_player-1]++;
    else if(c=='s' && cursors[current_player-1][moving_dim[current_player-1]]<SIZE-1)
        cursors[current_player-1][moving_dim[current_player-1]]++;
    else if(c=='w' && cursors[current_player-1][moving_dim[current_player-1]]>0)
        cursors[current_player-1][moving_dim[current_player-1]]--; 
    else if(c=='x') {
        moving_token = 1; 
        for(int i = 0; i<N_DIMS; i++){
            coords_moving_token[i] = cursors[current_player-1][i];
        }
    }
    while(getch()!=ERR);
}
void update(int frame){
    if(moving_token == 1){
        int new_moving_token_coords[N_DIMS];
        for(int i = 0; i<N_DIMS; i++){
                new_moving_token_coords[i] = coords_moving_token[i];
        }
        new_moving_token_coords[0]++;
        if(new_moving_token_coords[0]<SIZE && get_coords(new_moving_token_coords)==0){
            for(int i = 0; i<N_DIMS; i++){
                coords_moving_token[i] = new_moving_token_coords[i];
            }
        }

        else{
            set_coords(coords_moving_token, current_player);
            if(check_win_coords(coords_moving_token)==1){
                endwin();
                printf("Spieler %d hat gewonnen", current_player);
                exit(0);
            }
            
            current_player = current_player + 1;
            if(current_player == N_PLAYER+1){
                current_player = 1;
            }
            
            moving_token = 0;
        }
    }
}
int main(){
    initscr();
    nodelay(stdscr, TRUE);
    noecho();
    setup();
    int frame = 0;
    while(1){
        display();
        handle_input();
        refresh();
        update(frame);
        frame++;
        usleep(100*1000);
    }
    endwin();
    return 0;
}
