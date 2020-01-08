#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#define N_DIMS 4
#define SIZE 8
#define N_PLAYER 2
#define N_AXES (pow(2, N_DIMS)-1)
int current_player = 1;
int cursors[N_PLAYER][N_DIMS];
int *axes;
int game[SIZE][SIZE][SIZE][SIZE];
int coords_moving_token[N_DIMS];
int moving_token = 0;
void setup(){
    axes = (int *) malloc(N_AXES*sizeof(int));
    for (int i = 0; i<=pow(2, N_DIMS); i++){
        axes[i-1] = i;
    }
    for (int i = 0; i < N_PLAYER; i++)
    {
        for (int j = 1; j < N_DIMS; j++)
        {
            cursors[i][j] = SIZE/2;
        }
        cursors[i][0] = 0;
        
    }
    memset(&game, 0, pow(SIZE, N_DIMS));
    start_color();
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
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
    return game[coords[0]][coords[1]][coords[2]][coords[3]];
    
}
void set_coords(int coords[], int n){
    for(int i= 0; i<N_DIMS;i++){
        if (coords[i] >= SIZE){
            return;
        }
    }
    game[coords[0]][coords[1]][coords[2]][coords[3]] = n;
    
}
int check_win_coords(int coords[]){
    for (int i = 0; i < N_AXES; i++)
    {
        int axis = axes[i]; //equal or zero mask
        int max_row = 0;
        int new_coords[N_DIMS];
        for (int s = 0; s<=N_AXES;s++){ //s sign mask
            for(int j = 0;j < SIZE; j++){
                
                for(int k = 0; k < N_DIMS; k++){
                    if((s >> k) & 1 == 1) 
                        new_coords[k] = coords[k] + j*((axis >> k) & 1); //signmask positive
                    else
                        new_coords[k] = coords[k] - j*((axis >> k) & 1); //signmask negative
                    
                }
                if(get_coords(new_coords)==current_player){
                    max_row++;
                    if(max_row == 4){
                        return 1;
                    }
                }
                else{
                    max_row = 0;
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
       printw("%d ", cursors[current_player-1][i]);
    }
    
    refresh();
}
void handle_input(){
    char c = getch();
    if(c=='a' && cursors[current_player-1][1]>0) cursors[current_player-1][1]--; 
    else if(c=='d' && cursors[current_player-1][1]<SIZE-1) cursors[current_player-1][1]++;
    else if(c=='s' && cursors[current_player-1][2]<SIZE-1) cursors[current_player-1][2]++;
    else if(c=='w' && cursors[current_player-1][2]>0) cursors[current_player-1][2]--; 
    else if(c=='e' && cursors[current_player-1][3]<SIZE-1) cursors[current_player-1][3]++;
    else if(c=='q' && cursors[current_player-1][3]>0) cursors[current_player-1][3]--;
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
            if(check_win()==1){
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
    int win = check_win();
    printw("%d", win);
    refresh();
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
