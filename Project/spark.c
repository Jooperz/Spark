// spark.c written by Jasper Gray, message.c message.h, util.c, util.h, socket.h written by curtsinger
/*
;                                                                          
;                                                                          
;             ;     ;                                 ;                    
;                   ;                                                      
;   ;               ;                                                      
;   ;               ;                                                      
;   ;       ;;;     ;;;;;    ;;;;     ;;;    ;;;;   ;;;      ;;;;    ;;;;  
;   ;         ;     ;;  ;;   ;;  ;   ;   ;   ;;  ;    ;     ;;  ;;  ;    ; 
;   ;         ;     ;    ;   ;           ;   ;        ;     ;    ;  ;      
;   ;         ;     ;    ;   ;       ;;;;;   ;        ;     ;;;;;;   ;;;;  
;   ;         ;     ;    ;   ;      ;    ;   ;        ;     ;            ; 
;   ;         ;     ;;  ;;   ;      ;   ;;   ;        ;     ;;   ;  ;    ; 
;   ;;;;;;; ;;;;;   ;;;;;    ;       ;;; ;   ;      ;;;;;    ;;;;    ;;;;  
;                                                                          
;                                                                          
;                                                                          
;                                                                          
*/
#include <curses.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "socket.h"
#include "message.h"
#include "util.h"

void post_round_loop(WINDOW* window, int placement_value);
/*                                                                  
;                                                          
;                                                          
;           ;;;             ;               ;;;            
;             ;             ;                 ;            
;     ;;;;    ;             ;                 ;            
;    ;    ;   ;             ;                 ;            
;   ;         ;      ;;;;   ;;;;;     ;;;     ;      ;;;;  
;   ;         ;     ;;  ;;  ;;  ;;   ;   ;    ;     ;    ; 
;   ;    ;;   ;     ;    ;  ;    ;       ;    ;     ;      
;   ;     ;   ;     ;    ;  ;    ;   ;;;;;    ;      ;;;;  
;   ;     ;   ;     ;    ;  ;    ;  ;    ;    ;          ; 
;    ;    ;   ;     ;;  ;;  ;;  ;;  ;   ;;    ;     ;    ; 
;     ;;;;     ;;;   ;;;;   ;;;;;    ;;; ;     ;;;   ;;;;  
;                                                          
;                                                          
;                                                          
;                                                          
*/
#define BOARD_WIDTH 25
#define BOARD_HEIGHT 25
#define MENUE_WIDTH 50
#define MENUE_HEIGHT 4
#define WATER_SPRITE_SPEED 20
#define ROUND_NUM 5
/**
 * In-memory representation of the game board
 * Zero represents an empty cell
 * 1 represents the character
 * 2 represents money
 * 3 represents a wall
 * 4 represesnts a water sprite
 * 5 represents a goal
 */
int client_board[BOARD_HEIGHT][BOARD_WIDTH];
int server_board[BOARD_HEIGHT][BOARD_WIDTH];

char client_board_str[625];

// PLayer character to move around the screen
typedef struct spark {
  int x_pos;
  int y_pos;
} spark_t;

// enemies that track the player character
typedef struct sprite {
  int x_pos;
  int y_pos;
  spark_t a;
} sprite_t;

// is a round running?
bool round_running = true;

// player scores
int server_score = 0;
int client_score = 0;

// player money
int money;

int winning_player;

// have you lost
bool loser;

// clock which countdowns and kicks once it hits 0 to stop players from staying in the maze and collecting money after losing
int clock_val = -1;

// round number
int round_number = 1;

// char that player pressed
int server_input;

// lock for my thready thingy
pthread_mutex_t lock;
/*
;                                                          
;                                                          
;                   ;;;                                    
;                     ;                                    
;   ;     ;           ;                                    
;   ;     ;           ;                                    
;   ;     ;  ;;;;     ;     ;;;;;    ;;;;    ;;;;    ;;;;  
;   ;     ; ;;  ;;    ;     ;;  ;;  ;;  ;;   ;;  ;  ;    ; 
;   ;;;;;;; ;    ;    ;     ;    ;  ;    ;   ;      ;      
;   ;     ; ;;;;;;    ;     ;    ;  ;;;;;;   ;       ;;;;  
;   ;     ; ;         ;     ;    ;  ;        ;           ; 
;   ;     ; ;;   ;    ;     ;;  ;;  ;;   ;   ;      ;    ; 
;   ;     ;  ;;;;      ;;;  ;;;;;    ;;;;    ;       ;;;;  
;                           ;                              
;                           ;                              
;                           ;                              
;                                                          
*/
/**
 * Convert a board row number to a screen position
 * \param   row   The board row number to convert
 * \return        A corresponding row number for the ncurses screen
 */
int screen_row(int row) {
  return 6 + row;
}

/**
 * Convert a board column number to a screen position
 * \param   col   The board column number to convert
 * \return        A corresponding column number for the ncurses screen
 */
int screen_col(int col) {
  return 12 + col;
}
/**
 * Convert a board row number to a screen position
 * \param   row   The board row number to convert
 * \return        A corresponding row number for the ncurses screen
 */
int menue_row(int row) {
  return 1 + row;
}
/**
 * Convert a board row number to a screen position
 * \param   row   The board row number to convert
 * \return        A corresponding row number for the ncurses screen
 */
int menue_col(int row) {
  return 1 + row;
}

/**
 * clear the contents of the game board
 * 
 */
void clear_window() {
  for (int i = 0; i < BOARD_HEIGHT; i++) {
    for (int j = 0; j < BOARD_WIDTH; j++) {
      mvaddch(screen_row(i), screen_col(j), ' ');
    }
  }
}
/**
 * Initialize the board display by printing the title and edges
 */
void init_display() {
  // Print corners
  mvaddch(0, 0, ACS_ULCORNER);
  mvaddch(0, MENUE_WIDTH, ACS_URCORNER);
  mvaddch(MENUE_HEIGHT, 0, ACS_LLCORNER);
  mvaddch(MENUE_HEIGHT, MENUE_WIDTH, ACS_LRCORNER);
    // Print top and bottom edges
  for (int col = 1; col < MENUE_WIDTH; col++) {
    mvaddch(0, col, ACS_HLINE);
    mvaddch(MENUE_HEIGHT, col, ACS_HLINE);
  }

  // Print left and right edges
  for (int row = 1; row < MENUE_HEIGHT; row++) {
    mvaddch(row, 0, ACS_VLINE);
    mvaddch(row, MENUE_WIDTH, ACS_VLINE);
  }
  mvprintw(menue_row(0), menue_col(0), "Score");
  mvprintw(menue_row(1), menue_col(0), "You:  0");
  mvprintw(menue_row(2), menue_col(0), "Them: 0");

  mvprintw(menue_row(1), menue_col(MENUE_WIDTH / 2) - 3, "SPARK");

  mvprintw(menue_row(0), menue_col(MENUE_WIDTH) - 8, "$$:0");

  // Print corners
  mvaddch(screen_row(-1), screen_col(-1), ACS_ULCORNER);
  mvaddch(screen_row(-1), screen_col(BOARD_WIDTH), ACS_URCORNER);
  mvaddch(screen_row(BOARD_HEIGHT), screen_col(-1), ACS_LLCORNER);
  mvaddch(screen_row(BOARD_HEIGHT), screen_col(BOARD_WIDTH), ACS_LRCORNER);

  // Print top and bottom edges
  for (int col = 0; col < BOARD_WIDTH; col++) {
    mvaddch(screen_row(-1), screen_col(col), ACS_HLINE);
    mvaddch(screen_row(BOARD_HEIGHT), screen_col(col), ACS_HLINE);
  }

  // Print left and right edges
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    mvaddch(screen_row(row), screen_col(-1), ACS_VLINE);
    mvaddch(screen_row(row), screen_col(BOARD_WIDTH), ACS_VLINE);
  }

  // Refresh the display
  refresh();
}
/**
 * Start Game window screen waiting for client
 */
void start_game_server(unsigned short port) {
  mvprintw(screen_row(BOARD_HEIGHT / 2), screen_col(BOARD_WIDTH / 2) - 8, "Welcome to spark!");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 2, screen_col(BOARD_WIDTH / 2) - 9, "Waiting for client");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 3, screen_col(BOARD_WIDTH / 2) - 10, "Server Socket: %d", port);
  refresh();
}
/**
 * CLient Connected screen, waiting dor the server to choose a puzzle
 */
void start_game_client(int client_socket_d) {
  mvprintw(screen_row(BOARD_HEIGHT / 2), screen_col(BOARD_WIDTH / 2) - 8, "Welcome to spark!");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 2, screen_col(BOARD_WIDTH / 2) - 9, "Waiting for server");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 3, screen_col(BOARD_WIDTH / 2) - 9, "To choose a puzzle");
  refresh();
}
/**
 * changes and displays the amount of money
 * 
 * @param change amount the money should be changed by
 */
void update_money(int change) {
  money += change;
  mvprintw(menue_row(0), menue_col(MENUE_WIDTH - 5), "%d  ", money);
}
/**
 * Update the client board
 */
void update_client_board() {
  // Loop over cells of the game board
  for (int r = 0; r < BOARD_HEIGHT; r++) {
    for (int c = 0; c < BOARD_WIDTH; c++) {
      if (client_board[r][c] == 0) {  // Draw blank spaces
        mvaddch(screen_row(r), screen_col(c), ' ');
      } else if (client_board[r][c] == 1) {  // Draw character
        mvaddch(screen_row(r), screen_col(c), 'A');
      } else if (client_board[r][c] == 2) { // monmey 
        mvaddch(screen_row(r), screen_col(c), 'x');
      } else if (client_board[r][c] == 3) { // wall 
        attron(A_REVERSE);
        mvaddch(screen_row(r), screen_col(c), ' ');
        attroff(A_REVERSE);
      } else if (client_board[r][c] == 4) { // Draw water sprite
        mvaddch(screen_row(r), screen_col(c), 'w');
      } else if (client_board[r][c] == 5) { // Draw goal
        mvaddch(screen_row(r), screen_col(c), '0');
      }
    }
  }
  refresh();
}
/**
 * Map selector window **NOT FINISHED**
 */
char* puzzle_selector(WINDOW* win, int client_id) {
  wrefresh(win);
  clear_window();
  mvprintw(screen_row(0), screen_col(BOARD_WIDTH / 2) - 12, "Choose a starting puzzle");
  mvprintw(screen_row(1), screen_col(BOARD_WIDTH / 2) - 12, "========================");

  // menue system
  int highlight = 0;
  char input = '\0';
  char* choices[3] = {"Standard", "Such Walls", "Bullet Hell"};
  char* choices_txt[3] = {"Standard.txt", "Such Walls.txt", "Bullet Hell.txt"};
  while(input != 10) {
    for (int i = 0; i < 3; i++) {
      if (i == highlight) attron(A_REVERSE);
      mvprintw(screen_row(2 + i), screen_col(0), choices[i]);
      attroff(A_REVERSE);
    }
    input = wgetch(win);
    if (input == 'w') {
      highlight--;
      if (highlight == -1) highlight = 2;
    }
    if (input == 's') {
      highlight++;
      if (highlight == 3) highlight = 0;
    }
  }
  return choices_txt[highlight];
}
/**
 * Post round window screen
 */
void post_round(WINDOW* win) {
  wrefresh(win);
  update_client_board();
  // menue system
  mvprintw(menue_row(0), menue_col(MENUE_WIDTH / 2) - 3, "SHOP:");
  int highlight = 0;
  char input = '\0';
  char* choices[2] = {"Walls 5$", "Sprite 5$"};
  while(input != 'n') {
    for (int i = 0; i < 2; i++) {
      if (i == highlight) attron(A_REVERSE);
      mvprintw(menue_row(i + 1), (menue_col(MENUE_WIDTH / 2) - 5), choices[i]);
      attroff(A_REVERSE);
    }
    input = wgetch(win);
    if (input == 'w') {
      highlight--;
      if (highlight == -1) highlight = 1;
    }
    if (input == 's') {
      highlight++;
      if (highlight == 2) highlight = 0;
    }
    if (input == 10) {
      // f the player wants to place something go into the post round loop
      post_round_loop(win, (highlight + 3));
    }
  }
}
/**
 * End game window screen
 */
void end_game() {
  clear_window();
  mvprintw(screen_row(BOARD_HEIGHT / 2) - 1, screen_col(BOARD_WIDTH / 2) - 6, "            ");
  mvprintw(screen_row(BOARD_HEIGHT / 2), screen_col(BOARD_WIDTH / 2) - 6, " Game Over! ");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 1, screen_col(BOARD_WIDTH / 2) - 6, "            ");
  mvprintw(screen_row(BOARD_HEIGHT / 2) + 2, screen_col(BOARD_WIDTH / 2) - 11, "Player %d won!", winning_player);
  refresh();
  // wait to exit
  getchar();
}
/**
 * get a board from client_socket_d and load it into the client board
 */
void receive_board(int client_socket_d) {
    char* board = receive_message(client_socket_d);
    if (board == NULL) {
      perror("Failed to read message from client");
      exit(EXIT_FAILURE);
    }
    int i = 0;
    int j = 0;
    for (int r = 0; r < 625; r++) {
      server_board[i][j] = board[r] - '0';
      j++;
      if (j == 25) {
        j = 0;
        i++;
      }
  }
}
/**
 * Update the server board
 */
void update_server_board() {
  // Loop over cells of the game board
  for (int r = 0; r < BOARD_HEIGHT; r++) {
    for (int c = 0; c < BOARD_WIDTH; c++) {
      if (server_board[r][c] == 0) {  // Draw blank spaces
        mvaddch(screen_row(r), screen_col(c), ' ');
      } else if (server_board[r][c] == 1) {  // Draw character
        mvaddch(screen_row(r), screen_col(c), 'A');
      } else if (server_board[r][c] == 2) { // fuel 
        mvaddch(screen_row(r), screen_col(c), 'x');
      } else if (server_board[r][c] == 3) { // wall 
        attron(A_REVERSE);
        mvaddch(screen_row(r), screen_col(c), ' ');
        attroff(A_REVERSE);
      } else if (server_board[r][c] == 4) { // Draw water sprite
        mvaddch(screen_row(r), screen_col(c), 'w');
      } else if (server_board[r][c] == 5) { // Draw water sprite
        mvaddch(screen_row(r), screen_col(c), '0');
      }
    }
  }
  refresh();
}
/**
 * thread for losers
 */
void* loser_thread(void* id) {
  int* thread_id = (int*) id;
  // if you lose start the clock
  char* mes = receive_message(*thread_id);
  clock_val = 5;
  if (mes[0] == '1') {
    loser = TRUE;
    for (int i = 5; i > 0; i++) {
      sleep_ms(1000);
      clock_val--;
    }
  }
  clock_val = -1;
  return NULL;
}

/**
 * returns true if the sprite cannot go into its current pos
 * 
 * @param s sprite
 * @return true 
 * @return false 
 */
bool sprite_check(sprite_t* s) {
  return (server_board[s->y_pos][s->x_pos] == 3 || 
          server_board[s->y_pos][s->x_pos] == 2 || 
          server_board[s->y_pos][s->x_pos] == 5 ||
          s->y_pos < 0 || 
          s->y_pos> (BOARD_HEIGHT - 1) || 
          s->x_pos < 0 || 
          s->x_pos  > (BOARD_WIDTH - 1));
}
/**
 * Water Sprite Thread
 */
void* sprite_thread(void* s) {
  // create a sprite;
  sprite_t* sprite = (sprite_t*)s;
  int favor[4];
  int hor_dir;
  int vert_dir;
  int dir;
  while(round_running) {
    // make a preference for the sprite to move towards the character
    int original_x = sprite->x_pos;
    int original_y = sprite->y_pos;
    favor[0] = 25 - abs(sprite->x_pos - sprite->a.x_pos);
    favor[1] = 25 - abs(sprite->a.x_pos - sprite->x_pos);
    favor[2] = 25 - abs(sprite->y_pos - sprite->a.y_pos);
    favor[3] = 25 - abs(sprite->a.y_pos - sprite->y_pos);
    if (sprite->x_pos > sprite->a.x_pos) favor[0] +=7;
    else favor[1] +=7;
    if (sprite->y_pos > sprite->a.y_pos) favor[3] +=7;
    else favor[3] +=7;
    for (int i = 0; i < 4; i++) {
      favor[i] = favor[i] * 2;
    }
    // randoml choose the direction to go depending on the waited values
    if (favor[0] != 0) favor[0] = rand() % favor[0];
    if (favor[1] != 0) favor[1] = rand() % favor[1];
    if (favor[2] != 0) favor[2] = rand() % favor[2];
    if (favor[3] != 0) favor[3] = rand() % favor[3];

    if (favor[0] > favor[1]) vert_dir = 0;
    else vert_dir = 1;
    if (favor[2] > favor[3]) hor_dir = 2;
    else hor_dir = 3;
    if (favor[hor_dir] > favor[vert_dir]) dir = hor_dir;
    else dir = vert_dir;

    // go in that direction unless it cant then go in the second best direction
    if (dir == 0) {
      sprite->x_pos--;
      if (sprite_check(sprite)) {
        sprite->x_pos++;
        if (vert_dir == 3) {
          sprite->y_pos--;
          if (sprite_check(sprite)) {
            sprite->y_pos += 2;
            if (sprite_check(sprite)) {
              sprite->y_pos--;
              sprite->x_pos++;
            }
          }
        }
      }
    } else if (dir == 1) {
      sprite->x_pos++;
      if (sprite_check(sprite)) {
        sprite->x_pos--;
        if (vert_dir == 3) {
          sprite->y_pos--;
          if (sprite_check(sprite)) {
            sprite->y_pos += 2;
            if (sprite_check(sprite)) {
              sprite->y_pos--;
              sprite->x_pos--;
            }
          }
        }
      }
    } else if (dir == 2) {
      sprite->y_pos--;
      if (sprite_check(sprite)) {
        sprite->y_pos++;
        if (hor_dir == 0) {
          sprite->x_pos--;
          if (sprite_check(sprite)) {
            sprite->x_pos += 2;
            if (sprite_check(sprite)) {
              sprite->x_pos--;
              sprite->y_pos++;
            } 
          }
        }
      }
    } else if (dir == 3) {
      sprite->y_pos++;
      if (sprite_check(sprite)) {
        sprite->y_pos--;
        if (hor_dir == 0) {
          sprite->x_pos--;
          if (sprite_check(sprite)) {
            sprite->x_pos += 2;
            if (sprite_check(sprite)) {
              sprite->x_pos--;
              sprite->y_pos--;
            }
          }
        }
      }
    }

    // finalize sprite position
    server_board[original_y][original_x] = 0;
    server_board[sprite->y_pos][sprite->x_pos] = 4;
    // wait to update again
    sleep_ms(500);
  }
  return NULL;
}
/*                                                 
;                                                                          
;                                                                          
;                                                                          
;                                                                          
;     ;;;;                                  ;                              
;    ;    ;                                 ;                              
;   ;         ;;;   ;;;;;;;  ;;;;           ;        ;;;;    ;;;;   ;;;;;  
;   ;        ;   ;  ;  ;  ; ;;  ;;          ;       ;;  ;;  ;;  ;;  ;;  ;; 
;   ;    ;;      ;  ;  ;  ; ;    ;          ;       ;    ;  ;    ;  ;    ; 
;   ;     ;  ;;;;;  ;  ;  ; ;;;;;;          ;       ;    ;  ;    ;  ;    ; 
;   ;     ; ;    ;  ;  ;  ; ;               ;       ;    ;  ;    ;  ;    ; 
;    ;    ; ;   ;;  ;  ;  ; ;;   ;          ;       ;;  ;;  ;;  ;;  ;;  ;; 
;     ;;;;   ;;; ;  ;  ;  ;  ;;;;           ;;;;;;;  ;;;;    ;;;;   ;;;;;  
;                                                                   ;      
;                                                                   ;      
;                                                                   ;      
;                                                                          
*/
/**
 * Master loop handling running a round
 */
void game_loop(WINDOW* window, int client_id) {
  loser = false;
  pthread_t lost_thread;
  pthread_create(&lost_thread, NULL, &loser_thread, &client_id);
  mvprintw(menue_row(1), menue_col(MENUE_WIDTH / 2) - 5, "         ");
  mvprintw(menue_row(0), menue_col(MENUE_WIDTH / 2) - 5, "  SPARK  ");
  mvprintw(menue_row(2), menue_col(MENUE_WIDTH / 2) - 5, "         ");
  update_server_board();
  refresh();
  // create player spark
  spark_t server_spark;
  server_spark.y_pos = BOARD_HEIGHT - 3;
  server_spark.x_pos = BOARD_WIDTH / 2;
  // create sprites
  sprite_t sprites[300];
  int sprite_counter = 0;

  // create threads for every sprite
  for (int r = 0; r < BOARD_HEIGHT; r++) {
    for (int c = 0; c < BOARD_WIDTH; c++) {
        if (server_board[r][c] == 4) { // Draw water sprite
        pthread_t new_sprite;
        sprite_t sprite;
        sprite.a = server_spark;
        sprite.x_pos = c;
        sprite.y_pos = r;
        sprites[sprite_counter] = sprite;
        pthread_create(&new_sprite, NULL, &sprite_thread, &sprites[sprite_counter]);
        sprite_counter++;
      }
    }
  }

  // start the game loop
  round_running = TRUE;
  while(round_running) {
    if (clock_val > 0) {
      mvprintw(menue_row(2), menue_col(MENUE_WIDTH / 2), "%d", clock_val);
    }
    if (clock_val == 0) {
      round_running = false;
      clock_val = -1;
    }
    int original_x = server_spark.x_pos;
    int original_y = server_spark.y_pos;
    // get player input and move into that direction
    char input = wgetch(window);
    if (input == 'w') {
      server_spark.y_pos--;
    } else if (input == 's') {
      server_spark.y_pos++;
    } else if (input == 'd') {
      server_spark.x_pos++;
    } else if (input == 'a') {
      server_spark.x_pos--;
    }
    // in a wall
    if (server_board[server_spark.y_pos][server_spark.x_pos] == 3) {
      server_spark.x_pos = original_x;
      server_spark.y_pos = original_y;
    }
    // in a sprite
    if (server_board[server_spark.y_pos][server_spark.x_pos] == 4) {
      server_spark.y_pos = BOARD_HEIGHT - 1;
      server_spark.x_pos = BOARD_WIDTH / 2;
    }
    // in money
    if (server_board[server_spark.y_pos][server_spark.x_pos] == 2) {
      update_money(1);
    }
    // at the end
    if (server_board[server_spark.y_pos][server_spark.x_pos] == 5) {
      round_running = FALSE;
    }
    // on a border
    if (server_spark.y_pos < 0 || 
        server_spark.y_pos > (BOARD_HEIGHT - 1) || 
        server_spark.x_pos < 0 || 
        server_spark.x_pos  > (BOARD_WIDTH - 1)) {
          server_spark.x_pos = original_x;
          server_spark.y_pos = original_y;
    }
    // finalize spark position
    server_board[original_y][original_x] = 0;
    server_board[server_spark.y_pos][server_spark.x_pos] = 1;
    // update both boards
    update_server_board();
  }
  // if the player has lost
  if (loser == false) {
    send_message(client_id, "1");
    server_score++;
    mvprintw(menue_row(1), menue_col(6), "%d", server_score);
    clear_window();
    mvprintw(screen_row(BOARD_HEIGHT / 2), screen_col(BOARD_WIDTH / 2) - 8, "You finished first");
    mvprintw(screen_row(BOARD_HEIGHT / 2) - 1, screen_col(BOARD_WIDTH / 2) - 8, "waiting for client");
    refresh();
    pthread_join(lost_thread, NULL);
  } else {
    send_message(client_id, "2");
    client_score++;
    mvprintw(menue_row(2), menue_col(6), "%d", client_score);
  }
}
/**
 * Master loop handling the post round
 */
void post_round_loop(WINDOW* window, int placement_value) {
  spark_t server_cursor;
  server_cursor.y_pos = BOARD_HEIGHT - 3;
  server_cursor.x_pos = BOARD_WIDTH / 2;
  char input = '\0';
  int prev_ft = 0;
  while(true) {
    // move the server curser
    int original_x = server_cursor.x_pos;
    int original_y = server_cursor.y_pos;
    input = wgetch(window);
    if (input == 'w') {
      server_cursor.y_pos--;
    } else if (input == 's') {
      server_cursor.y_pos++;
    } else if (input == 'd') {
      server_cursor.x_pos++;
    } else if (input == 'a') {
      server_cursor.x_pos--;
    } else if (input == ' ') {
      if ((money - 5) >= 0) {
        update_money(-5);
        client_board[server_cursor.y_pos][server_cursor.x_pos] = placement_value;
        prev_ft = placement_value;
        client_board_str[(server_cursor.y_pos * 25) + server_cursor.x_pos] = (placement_value + '0');
      }
    } else if (input == 'b') {
      client_board[original_y][original_x] = prev_ft;
      update_client_board();
      break;
    }
    // on a border
    if (server_cursor.y_pos < 0 || 
        server_cursor.y_pos > (BOARD_HEIGHT - 3) || 
        server_cursor.x_pos < 0 || 
        server_cursor.x_pos  > (BOARD_WIDTH - 1)) {
          server_cursor.x_pos = original_x;
          server_cursor.y_pos = original_y;
    }
    if (server_cursor.y_pos != original_y || server_cursor.x_pos != original_x) {
      client_board[original_y][original_x] = prev_ft;
      // finalize curser position
      prev_ft = client_board[server_cursor.y_pos][server_cursor.x_pos];
      client_board[server_cursor.y_pos][server_cursor.x_pos] = placement_value;
    }
    
    // update both boards
    update_client_board();
  }
}
/*                                                                        
;                                  
;                                  
;                     ;            
;                                  
;   ;;   ;;                        
;   ;;   ;;                        
;   ; ; ; ;   ;;;   ;;;     ; ;;;  
;   ; ; ; ;  ;   ;    ;     ;;   ; 
;   ; ; ; ;      ;    ;     ;    ; 
;   ;  ;  ;  ;;;;;    ;     ;    ; 
;   ;     ; ;    ;    ;     ;    ; 
;   ;     ; ;   ;;    ;     ;    ; 
;   ;     ;  ;;; ;  ;;;;;   ;    ; 
;                                  
;                                  
;                                  
;                                  
*/
int main(int argc, char** argv) {
  srand(time(NULL));
  if (argc != 1 && argc != 3) {
    fprintf(stderr, "Usage: %s [<peer> <port number>]\n", argv[0]);
    exit(1);
  }
  // make screen
  // Initialize the ncurses window
  WINDOW* mainwin = initscr();
  noecho();
  curs_set(0);
  nodelay(mainwin, true);

  if (mainwin == NULL) {
    fprintf(stderr, "Error initializing ncurses.\n");
    exit(2);
  }
  // start game screen and wait for client
  init_display();
  // server Screen
  if (argc == 1) {
    unsigned short port = 0;
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
      perror("Server socket was not opened");
      exit(EXIT_FAILURE);
    }
    start_game_server(port);
    // Open a server socket
    // Start listening for connections, with a maximum of one queued connection
    if (listen(server_socket_fd, 1)) {
      perror("listen failed");
      exit(EXIT_FAILURE);
   }
    // Wait for a client to connect
    int client_id = server_socket_accept(server_socket_fd);
    if (client_id == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    // once client found, map selector
    char* map = puzzle_selector(mainwin, client_id);
    FILE *map_file = fopen(map, "r");
    if (map_file == NULL) {
      perror("Failed: ");
      return 1; 
    }
    char cell = 0;
    int i = 0;
    int j = 0;
    for(int r = 0; r < 649; r++) {
       cell = fgetc(map_file);
       if (cell != '\n') {
        server_board[i][j] = cell - '0';
        client_board[i][j] = cell - '0';
        strncat(client_board_str, &cell, 1);
        j++;
        if (j == 25) {
          j = 0;
          i++;
       }
      }
    }
    // send the board to the client
    send_message(client_id, client_board_str);
    // once both ready start first round
    // round loop
    for (int i = 0; i < ROUND_NUM; i++) {
      game_loop(mainwin, client_id);
      // post round
      post_round(mainwin);
      send_message(client_id, client_board_str);
      receive_board(client_id);
    } // loop
    // end game window
    if (server_score >= client_score) {
      winning_player = 1;
    } else winning_player = 2;
    end_game();
    // Clean up window
    delwin(mainwin);
    endwin();

  }
  //======================================================================================================================//
  // For a client
  if (argc == 3) {
    // Unpack arguments
    char* peer_hostname = argv[1];
    unsigned short peer_port = atoi(argv[2]);
    // Connect to the server
    int socket_fd = socket_connect(peer_hostname, peer_port);
    if (socket_fd == -1) {
      perror("Failed to connect");
      exit(EXIT_FAILURE);
    }

    init_display();
    start_game_client(socket_fd);

    char* board = receive_message(socket_fd);
    if (board == NULL) {
      perror("Failed to read message from client");
      exit(EXIT_FAILURE);
    }
    int i = 0;
    int j = 0;
    for (int r = 0; r < 625; r++) {
      server_board[i][j] = board[r] - '0';
      client_board[i][j] = board[r] - '0';
      strncat(client_board_str, &board[r], 1);
      j++;
      if (j == 25) {
        j = 0;
        i++;
      }
    }
    // once both ready start first round
    // round loop
    for (int i = 0; i < ROUND_NUM; i++) {
      round_running = TRUE;
      game_loop(mainwin, socket_fd);
      // post round
      post_round(mainwin);
      send_message(socket_fd, client_board_str);
      receive_board(socket_fd);
    } // loop
    // end game window
    if (server_score >= client_score) {
      winning_player = 1;
    } else winning_player = 2;
    end_game();
    // Clean up window
    delwin(mainwin);
    endwin();
  }
  return 0;
}