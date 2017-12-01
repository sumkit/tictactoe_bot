#include <iostream>
#include <climits>

#include "main.h"

const int N = 3; //dimensions of board

int min(int a, int b) {
  return (a < b) ? a : b;
}
int max(int a, int b) {
  return (a > b) ? a : b;
}

/**
 * Calculate the heuristic of playing a letter on a square for this player
 * player either equals 'X' (user) or 'O' (bot)
 */
int calculateValue(char *board, int row, int col, char player) {
  int rowSquare = 1;
  int colSquare = 1;
  int value = 0;
  
  for(int i = 0; i < N; i++) {
    //check col
    if(board[row*N+i] == player) {
      value+=(colSquare*colSquare);
      colSquare++;
    }
    if(board[row*N+i] == 0) {
      value++;
    }
    //check row
    if(board[i*N+col] == player) {
      value+=(rowSquare*rowSquare);
      rowSquare++;
    }
    if(board[i*N+col] == 0) {
      value++;
    }
  }
  return value;
}

bool isWinner(int row, int col, char* board, char player) {
  for(int i = 0; i < N; i++) {
    if(board[i*N+col] != player) {
      //check col
      return false;
    }
    if(board[row*N+i] != player) {
      //check row
      return false;
    }
    if(row == col) {
      //check diagonal
      if(board[i*N+i] != player) {
        return false;
      }
    }
  }
  return true;
}

/**
 * alpha - worst case for maximizing player
 * beta - best case for minimizing player
 * botMaximizing - true if its the bot's turn
 * keep track of how many X's and O's have already been placed on the board
 */
int alphabeta(node_t node, int depth, int alpha, int beta, bool botMaximizing, char* board, int numTurns) {
  //generate tree
  node_t *children = (node_t *) malloc(((N*N) - numTurns) * sizeof(node_t));
  int childIndex = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      int index = i*N + j;
      if(board[index] == 0) {
        int val = calculateValue(board, i, j, botMaximizing ? 'O' : 'X');
        children[childIndex].value = val;
        children[childIndex].row = i;
        children[childIndex].col = j; 
        childIndex++;
      }
    }
  }

  if(depth == 0 || (N*N) - numTurns == 0) {
    printf("depth: %d, %d, %p\n", depth, (N*N) - numTurns, (void *)&children);
    free(children);
    printf("after depth 0\n");
    return node.value;
  }

  int value;
  if(botMaximizing) {
    value = INT_MIN;
    for(int i = 0; i < (N*N)-numTurns; i++) {
      board[node.row*N+node.col] = 'O';
      value = max(value, alphabeta(children[i], depth-1, alpha, beta, false, board, numTurns+1));
      printf("bot max depth: %d, %d, %p\n", depth, (N*N) - numTurns, (void *)&children);
      free(children);
      printf("after bot max\n");
      alpha = max(alpha, value); 
    }
  }
  else {
    value = INT_MAX;
    for(int i = 0; i < (N*N)-numTurns; i++) {
      board[node.row*N+node.col] = 'X';
      value = min(value, alphabeta(children[i], depth-1, alpha, beta, true, board, numTurns+1));
      printf("bot min depth: %d, %d, %p\n", depth, (N*N) - numTurns, (void *)&children);
      free(children);
      printf("after bot min\n");
      beta = min(beta, value);
    }
  }
  printf("%d\n", depth);
  return value;
}

void updateCLI(char *board) {
  for(int outer = 0; outer < N; outer++) {
    for(int i =0 ; i< N; i++) {
      printf("%c |", board[outer*N+i]);
    }
    printf("\n");
    for(int i = 0; i < N; i++) {
      printf("- ");
    }
    printf("\n");
  }
}

int main() {
  char* board = (char *) calloc(N*N, sizeof(char));

  //let user go first. wait for input.
  //TODO: fill this in with user's input
  int row = 1;
  int col = 1; 
  int depth = 5; //number of turns taken (depth/2 game cycles)
  node_t root;
  root.value = calculateValue(board, row, col, 'X');
  root.row = row;
  root.col = col;
  printf("here\n");
  alphabeta(root, depth, INT_MIN, INT_MAX, true, board, 0);
  printf("here2\n");
  updateCLI(board);
  return 0;
}
