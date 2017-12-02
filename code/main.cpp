#include <iostream>
#include <climits>
#include <stdio.h>
#include <cstdlib>
#include <omp.h>
#include <string.h>

#include "main.h"

#define BUFSIZE 1024

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
 * make sure AI blocks when player is about to win 
 * https://en.wikipedia.org/wiki/Tic-tac-toe 
 */


int calculateSmallBoardScore(char *board, char bot, char player) { 
  int center = 1 * N + 1; 
  int score = 0; 
  if (board[center] == bot) 
    score += 3;
  else if (board[center] == player)  
    score -= 3; 
  

  int diagScoreOne = 0;
  int diagScoreTwo = 0; 

  for (int i = 0; i < N; i++) { 
    int colScore = 0; 
    for (int col = 0; col < N; col++) { 
      if (board[i*N+col] == bot) 
        colScore += 1;
      else if (board[i*N+col] == player) 
        colScore -= 1; 
    }

    if (colScore >= 2 || colScore <= -2) 
      score += colScore; 

    int rowScore = 0; 
    for (int row = 0; row < N; row++) { 
      if (board[row*N + i] == bot) 
        rowScore += 1; 
      else if (board[row * N + i] == player) 
        rowScore -= 1; 
    }

    if (rowScore >= 2 || rowScore <= -2) 
      score += rowScore; 

    if (board[i*N+i] == bot) 
      diagScoreOne += 1; 
    else if (board[i*N+i] == player)
      diagScoreOne -= 1; 

    if (board[i*N+(N-(i+1))] == bot) 
      diagScoreTwo += 1;
    else if (board[i*N + (N-(i+1))] == player)
      diagScoreTwo -= 1; 

  }

  if (diagScoreOne >= 2 || diagScoreOne <= -2) 
      score += diagScoreOne; 


  if (diagScoreTwo >= 2 || diagScoreTwo <= -2) 
      score += diagScoreTwo; 

  return score; 

}


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
  bool rowWin = true;
  bool colWin = true;
  bool fwdDiagWin = row == col;
  bool backDiagWin = row == N-(col+1);
  for(int i = 0; i < N; i++) {
    if(colWin && board[i*N+col] != player) {
      //check col
      colWin = false;
    }
    if(rowWin && board[row*N+i] != player) {
      //check row
      rowWin = false;
    }
    if(fwdDiagWin && board[i*N+i] != player) {
      //check diagonal
      fwdDiagWin = false;
    }
    if(backDiagWin && board[i*N+(N-(i+1))] != player) {
      //other diagonal (0, 2) (1, 1)
      backDiagWin = false;
    }
  }
  return rowWin || colWin || fwdDiagWin || backDiagWin;
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
  node_t result = node_t();
  if(botMaximizing) {
    result.value = INT_MIN;
    int i;
    #pragma omp parallel for default(shared) private(i) num_threads(256)
    for(i = 0; i < numChildren; i++) {
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
    int i;
    #pragma omp parallel for default(shared) private(i) num_threads(256)
    for(i = 0; i < numChildren; i++) {
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

#ifdef RUN_MIC /* Use RUN_MIC to distinguish between the target of compilation */

  /* This pragma means we want the code in the following block be executed in 
   * Xeon Phi.
   */
#pragma offload target(mic) \
  // inout(board: length(N*N) INOUT) 
#endif
  {
    //depth = # of turns taken (depth/2 = # game cycles)
    char winner;
    int numTurns = 0;
    while(1) {
      //let user go first. wait for input.
      node_t root = readInput();
      root.value = calculateValue(board, root.row, root.col, 'X');
      board[root.row*N+root.col] = 'O';
      if(isWinner(root.row, root.col, board, 'O')) {
        winner = 'O';
        printf("O Won!\n");
        break;
      }
      numTurns++;
      if(numTurns = N*N) {
        printf("Tie!\n");
        break;
      }
      
      node_t res = alphabeta(root, 2, INT_MIN, INT_MAX, true, board, 1);
      board[res.row*N+res.col] = 'X';
      updateCLI(board);
      if(isWinner(res.row, res.col, board, 'X')) {
        winner = 'X';
        printf("X Won!\n");
        break;
      }
      numTurns++;
      if(numTurns == N*N) {
        printf("Tie!\n");
        break;
      }
    }
  }

  //write output to file
  char output_filename[BUFSIZE];
  sprintf(output_filename, "output_%d.txt", 256);
  FILE *output_file = fopen(output_filename, "w");
  if (!output_file) {
    printf("Error: couldn't output costs file");
    return -1;
  }
  for(int c = 0; c < N; c++) {
    fprintf(output_file, " -");
  }
  fprintf(output_file, "\n");
  for(int r = 0; r < N; r++) {
    fprintf(output_file, "|");
    for(int c = 0; c < N; c++) {
      char temp = board[r*N+c];
      if(temp == 0) {
        fprintf(output_file, " |");
      } else {
        fprintf(output_file, "%c|", temp);
      }
    }
    fprintf(output_file, "\n");
    for(int c = 0; c < N; c++) {
      fprintf(output_file, " -");
    }
    fprintf(output_file, "\n");
  }
  fclose(output_file);

  free(board);
  return 0;
}
