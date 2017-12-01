#include <iostream>
#include <climits>
#include <stdio.h>
#include <cstdlib>
#include <string.h>

#include "main.h"

const int N = 3; //dimensions of board

int min(int a, int b) {
  return (a < b) ? a : b;
}
int max(int a, int b) {
  return (a > b) ? a : b;
}
void updateCLI(char *board) {
  for(int c = 0; c < N; c++) {
    printf(" -");
  }
  printf("\n");
  for(int r = 0; r < N; r++) {
    printf("|");
    for(int c = 0; c < N; c++) {
      char temp = board[r*N+c];
      if(temp == 0) {
        printf(" |");
      } else {
        printf("%c|", temp);
      }
    }
    printf("\n");
    for(int c = 0; c < N; c++) {
      printf(" -");
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
node_t alphabeta(node_t node, int depth, int alpha, int beta, bool botMaximizing, char* abBoard, int numTurns) {
  char* board = (char *) malloc(N*N*sizeof(char));
  memcpy(board, abBoard, N*N);
  if(botMaximizing) {
    board[node.row*N+node.col] = 'O';
    if(isWinner(node.row, node.col, board, 'O')) {
      node_t res = node_t();
      res.value = INT_MAX;
      res.row = node.row;
      res.col = node.col;
      return res;
    } 
  }
  else {
    board[node.row*N+node.col] = 'X';
    if(isWinner(node.row, node.col, board, 'X')) {
      node_t res = node_t();
      res.value = INT_MIN;
      res.row = node.row;
      res.col = node.col;
      return res;
    } 
  }
  int numChildren = (N*N) - numTurns;
  if(depth == 0 || numChildren == 0) {
    return node;
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
  node_t result = node_t();
  if(botMaximizing) {
    result.value = INT_MIN;
    for(int i = 0; i < numChildren; i++) {
      node_t ab = alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, board, numTurns+1);
      if(ab.value > result.value) {
        result.value = ab.value;
        result.row = ab.row;
        result.col = ab.col;
      }
      alpha = max(alpha, result.value);
    }
  }
  else {
    result = node_t();
    result.value = INT_MAX;
    for(int i = 0; i < numChildren; i++) {
      node_t ab = alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, board, numTurns+1);
      if(ab.value < result.value) {
        result.value = ab.value;
        result.row = ab.row;
        result.col = ab.col;
      }
      beta = min(beta, result.value);
    }
  }
  free(children);
  return result;
}

node_t readInput() {
  printf("hello\n");
  int r, c;
  std::cout << "Row #: ";
  std::cin >> r;
  std::cout << "Col #: ";
  std::cin >> c;
  node_t in = node_t();
  in.value = 0;
  in.row = r;
  in.col = c;
  return in;
}

int main() {
  char* board = (char *) calloc(N*N, sizeof(char));

  //let user go first. wait for input.
  //TODO: fill this in with user's input
  node_t root = readInput();
  root.value = calculateValue(board, root.row, root.col, 'X');
  //depth = # of turns taken (depth/2 = # game cycles)
  node_t res = alphabeta(root, 2, INT_MIN, INT_MAX, true, board, 1);
  board[root.row*N+root.col] = 'O';
  board[res.row*N+res.col] = 'X';
  updateCLI(board);
  free(board);
  return 0;
}
