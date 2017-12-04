#include <iostream>
#include <climits>
#include <stdio.h>
#include <cstdlib>
#include <omp.h>
#include <string.h>

#include "main.h"

#define BUFSIZE 1024

static int _argc;
static const char **_argv;

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
    // printf("\n");
  }
}
void updateMetaCLI(board_t *meta_board) {
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      updateCLI(meta_board[r*N+c].board);
    }
    printf("\n");
  }
  printf("abc\n");
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
node_t alphabeta(node_t *nodePtr, int depth, int alpha, int beta, bool botMaximizing, 
  char* abBoard, int numTurns, int num_of_threads) {
  char* board = (char *) malloc(N*N*sizeof(char));
  memcpy(board, abBoard, N*N);

  if(nodePtr != NULL) {
    node_t node = *nodePtr;
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
  }
  
  int numChildren = (N*N) - numTurns;
  if(depth == 0 || numChildren == 0) {
    return *nodePtr;
  }
  //generate tree
  node_t *children = (node_t *) malloc(numChildren * sizeof(node_t));
  int childIndex = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      int index = i*N + j;
      if(board[index] == 0) {
        int val = calculateSmallBoardScore(board, botMaximizing ? 'O' : 'X', botMaximizing ? 'X' : 'O');
        // int val = calculateValue(board, i, j, botMaximizing ? 'O' : 'X');
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
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      node_t ab = alphabeta(&(children[i]), depth-1, alpha, beta, !botMaximizing, 
        board, numTurns+1, num_of_threads);
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
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      node_t ab = alphabeta(&(children[i]), depth-1, alpha, beta, !botMaximizing, 
        board, numTurns+1, num_of_threads);
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

/* Starter code function, do not touch */
int get_option_int(const char *option_name, int default_value)
{
  for (int i = _argc - 2; i >= 0; i -= 2)
    if (strcmp(_argv[i], option_name) == 0)
      return atoi(_argv[i + 1]);
  return default_value;
}


bool boardComplete(char* local_board) { 
  for (int i = 0; i < N*N; i++) { 
    if (local_board[i] == 0)
      return false;
  }

  return true; 
}

//returns which board the opponent needs to make a move in next 
//if that board is complete, returns -1 
int makeMove(board_t* meta_board, int local_board_idx, int row, int col, char player) { 
  char* local_board = meta_board[local_board_idx].board; 
  local_board[row*N+col] = player; 
  if (isWinner(row, col, local_board, player))
    meta_board[local_board_idx].status = player; 

  if (boardComplete(local_board)) //tie
    meta_board[local_board_idx].status = 1; 

  int next_board_idx = row*N+col; 

  if (meta_board[next_board_idx].status == 0) 
    return next_board_idx; 

  return -1; 
}

bool isMetaWinner(board_t* meta, char player) { 
  bool rowWin = false;
  bool colWin = false; 
  bool diagWin = true; 

  for (int i = 0; i < N; i++) { 
    rowWin = true; 
    for (int j = 0; j < N; j++) { 
      if (meta[i*N + j].status != player)
        rowWin = false; 
    }
    if (rowWin) return true; 

  }

  for (int i = 0; i < N; i++) { 
    colWin = true; 
    for (int j = 0; j<N; j++) { 
      if(meta[i + j*N].status != player)
        colWin = false;
    }
    if (colWin) return true; 
  }

  for (int i = 0; i < N; i++) { 
    if (meta[i * N + i].status != player)
      diagWin = false; 
  }
  if (diagWin) return true; 

  diagWin = true; 
  for (int i =0; i < N; i++) { 
    if (meta[i * N + (N-(i+1))].status != player)
        diagWin = false;
  }
  if (diagWin) return true;

  return false;

}



int main(int argc, const char *argv[]) {
  _argc = argc - 1;
  _argv = argv + 1;


  // 3x3 meta board with pointers to 9 constituent local boards 
  // meta board holds a status for each board: "O" if O won, X if X won, 0 if in progress, 1 if tied. 
  board_t* meta_board = (board_t*) calloc(N*N, sizeof(board_t)); 
  for (int i = 0; i < N*N; i++) { 
    char* local_board = (char *) calloc(N*N, sizeof(char)); 
    meta_board[i].board = local_board; 
    meta_board[i].status = 0; 
  }

  //TODO: take this out 
  // char* board = meta_board[0].board; 

  int num_of_threads = get_option_int("-n", 1);

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
    int numTurns = 1;

    int nextIsTBD; //if next mini board is not yet decided because the calculated one is already completed

    //TODO start with random row and column 
    node_t root = node_t();
    root.row = 1;
    root.col = 1;
    // root.value = calculateValue(board, 1,1, false);
    root.value = calculateSmallBoardScore(meta_board[0].board, 'O', 'X');
    makeMove(meta_board, root.row*N+root.col, root.row, root.col, 'O');
    // board[4] = 'O';
    while(1) {
      //let user go first. wait for input.
      /*node_t root = readInput();
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
      }*/
      if(numTurns % 10000 == 0) updateMetaCLI(meta_board);
      if(!(root.row != 1 && root.col != 1)) {
        int nextIndex0;
        if(nextIsTBD) {
          //TODO meta_board -> char* board
          char *tempBoard = (char *) calloc(N*N, sizeof(char));
          for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
              tempBoard[i*N+j] = meta_board[i*N+j].status;
            }
          }
          node_t metaRes = alphabeta(&root, 2, INT_MIN, INT_MAX, true, tempBoard, 1, num_of_threads);
          free(tempBoard);
          nextIndex0 = metaRes.row*N+metaRes.col;
        } else {
          nextIndex0 = root.row*N+root.col;
        }
        int mm0 = makeMove(meta_board, nextIndex0, root.row, root.col, 'X');
        node_t res0 = alphabeta(&root, 2, INT_MIN, INT_MAX, false, meta_board[nextIndex0].board, 
            1, num_of_threads);
        if(mm0 != nextIndex0 ) {
          nextIsTBD = true;
        } else {
          nextIsTBD = false;
        }
        if(isMetaWinner(meta_board, 'O')) {
          winner = 'O';
          updateMetaCLI(meta_board);
          printf("O Won!\n");
          break; 
        }
        if(isWinner(res0.row, res0.col, meta_board[nextIndex0].board, 'O')) {
          // winner = 'O';
          printf("O Won mini board\n");
          // break;
        }
        numTurns++;
        if(numTurns == N*N) {
          // printf("Tie!\n");
          // break;
        }
        root.row = res0.row;
        root.col = res0.col;
        root.value = res0.value;
      }
      
      int nextIndex;
      if(nextIsTBD) {
        char *tempBoard = (char *) calloc(N*N, sizeof(char));
        for(int i = 0; i < N; i++) {
          for(int j = 0; j < N; j++) {
            tempBoard[i*N+j] = meta_board[i*N+j].status;
          }
        }
        node_t metaRes = alphabeta(&root, 2, INT_MIN, INT_MAX, true, tempBoard, 1, num_of_threads);
        free(tempBoard);
        nextIndex = metaRes.row*N+metaRes.col;
      }
      else {
        nextIndex = root.row*N+root.col;
      }
      node_t res = alphabeta(&root, 2, INT_MIN, INT_MAX, true, meta_board[nextIndex].board, 
          1, num_of_threads);
      int mm = makeMove(meta_board, root.row*N+root.col, root.row, root.col, 'X');

      if(mm != nextIndex) {
        nextIsTBD = true;
      } else {
        nextIsTBD = false;
      }
      if(isMetaWinner(meta_board, 'X')) {
        winner = 'X';
        updateMetaCLI(meta_board);
        printf("X Won!\n");
        break;
      }
      if(isWinner(res.row, res.col, meta_board[nextIndex].board, 'X')) {
        // winner = 'X';
        // printf("X Won mini board\n");
        // break;
      }
      numTurns++;
      if(numTurns == N*N) {
        // printf("Tie on mini board\n");
        // break;
      }
      root.row = res.row;
      root.col = res.col;
      root.value = res.value;
    }
  }

  //TODO write output to file
  /*char output_filename[BUFSIZE];
  sprintf(output_filename, "output_%d.txt", num_of_threads);
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
  fclose(output_file); */

  free(meta_board);
  return 0;
}
