#include <iostream>
#include <climits>
#include <stdio.h>
#include <cstdlib>

#include "main.h"

const int N = 3; //dimensions of board

int min(int a, int b) {
  return (a < b) ? a : b;
}
int max(int a, int b) {
  return (a > b) ? a : b;
}
void updateCLI(char *board) {
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      printf("%c |", board[r*N+c]);
    }
    printf("\n");
  }
  printf("\n");
}

/**
 * Calculate the heuristic of playing a letter on a square for this player
 * player either equals 'X' (user) or 'O' (bot)
 */
int calculateValue(char *board, int row, int col, char player) {
  int rowSquare = 1;
  int colSquare = 1;
  int diagSquare = 1;
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
    //check diagonal
    if(board[i*N+i] == player) {
      value += (diagSquare*diagSquare);
      diagSquare++;
    }
    if(board[i*N+i] == 0) {
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
  if(botMaximizing) {
    board[node.row*N+node.col] = 'O';
    if(isWinner(node.row, node.col, board, 'O')) return INT_MAX;
  }
  else {
    board[node.row*N+node.col] = 'X';
    if(isWinner(node.row, node.col, board, 'X')) return INT_MIN;
  }
  printf("num turns = %d:\n", numTurns);
  updateCLI(board);
  int numChildren = (N*N) - numTurns;
  if(depth == 0 || numChildren == 0) {
    return node.value;
  }
  //generate tree
  node_t *children = (node_t *) malloc(numChildren * sizeof(node_t));
  int childIndex = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      int index = i*N + j;
      if(board[index] == 0) {
        int val = calculateValue(board, i, j, botMaximizing ? 'O' : 'X');
        node_t temp = node_t();
        temp.value = val;
        temp.row = i;
        temp.col = j;
        children[childIndex] = temp;
        childIndex++;
      }
    }
  }
  if(childIndex != numChildren) { 
    // printf("child = %d, n*n = %d, num turns = %d\n", childIndex, numChildren, numTurns);
  }
  int value;
  if(botMaximizing) {
    value = INT_MIN;
    for(int i = 0; i < numChildren; i++) {
      value = max(value, alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, board, numTurns+1));
      alpha = max(alpha, value); 
    }
  }
  else {
    value = INT_MAX;
    for(int i = 0; i < numChildren; i++) {
      value = min(value, alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, board, numTurns+1));
      beta = min(beta, value);
    }
  }
  free(children);
  return value;
}

int main() {
  char* board = (char *) calloc(N*N, sizeof(char));

  //let user go first. wait for input.
  //TODO: fill this in with user's input
  int row = 1;
  int col = 1; 
  node_t root = node_t();
  root.value = calculateValue(board, row, col, 'X');
  root.row = row;
  root.col = col;
  char* abBoard = (char *) calloc(N*N, sizeof(char));
  //depth = # of turns taken (depth/2 = # game cycles)
  alphabeta(root, 2, INT_MIN, INT_MAX, true, abBoard, 1);
  // updateCLI(board);
  free(board);
  return 0;
}
