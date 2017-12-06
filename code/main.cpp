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
    printf("\n");
  }
}
void updateMetaCLI(board_t *meta_board) {
  printf("META: \n");
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      char temp = meta_board[i*N+j].status;
      if(temp == 0) {
        printf(" |");
      } else {
        printf("%c|", temp);
      }
    }
    printf("\n");
  }
  printf("\n");
  
  for(int c = 0; c < N*N; c++) {
    printf(" -");
  }
  printf("\n");
  int nonBlank = 0;
  for(int n = 0; n < N; n++) {
    //TODO show if the meta_board square is a win or tie 
    for(int r = 0; r < N; r++) {
      for(int n1 = 0; n1 < N; n1++) {
        for(int c = 0; c < N; c++) {
            char temp = meta_board[n*N+n1].board[r*N+c];
          if(temp == 0) {
            printf(" |");
          } else {
            printf("%c|", temp);
            nonBlank++;
          }
        }
      }
      if(r < N-1) printf("\n");
    }
    printf("\n");
  }
  for(int c = 0; c < N*N; c++) {
    printf(" -");
  }
  printf("\n");
  printf("num blank = %d\n", nonBlank);
  /*for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      printf("r = %d, c = %d\n", r, c);
      updateCLI(meta_board[r*N+c].board);
      printf("\n");
    }
  } */
}

//for mini board
bool isWinner(int row, int col, char* board, char player) {
  bool rowWin = false;
  bool colWin = false; 
  bool diagWin = true; 

  for (int i = 0; i < N; i++) { 
    rowWin = true; 
    for (int j = 0; j < N; j++) { 
      if (board[i*N + j] != player) rowWin = false; 
    }
    if (rowWin) {
      return true;
    } 
  }

  for (int i = 0; i < N; i++) { 
    colWin = true; 
    for (int j = 0; j<N; j++) { 
      if(board[i + j*N] != player) colWin = false;
    }
    if (colWin) {
      return true; 
    }
  }

  for (int i = 0; i < N; i++) { 
    if (board[i * N + i] != player)
      diagWin = false; 
  }
  if (diagWin) {
    return true; 
  }

  diagWin = true; 
  for (int i =0; i < N; i++) { 
    if (board[i * N + (N-(i+1))] != player)
        diagWin = false;
  }
  if (diagWin) {
    return true;
  } 

  return false;
}


int* calculateSmallBoardScore(char* board, int row, int col, 
  char player, char opp, int* oldScore) {

  int center = 1 * N + 1; 
  int* res = (int *) calloc(2, sizeof(int)); 
  
  int oldPlayerScore;
  int oldOppScore; 
  if (player == 'O') { 
    oldPlayerScore = oldScore[0];
    oldOppScore = oldScore[1];
  } else {
    oldPlayerScore = oldScore[1];
    oldOppScore = oldScore[0]; 
  }

  int myScore = oldPlayerScore; 
  int hisScore = oldOppScore; 

  //add 3 for the center of any board
  if (row * N + col == center) {
    myScore += 3;
  }

  int opWon = 0;  
  int iWon = 1; 
  int fail = 0;

  //add 2 for getting a sequence of two tiles
  //subtract 2 from the opponent for blocking their sequence of two tiles

  bool diagOne = ((row*N+col) % 4 == 0); 
  bool diagTwo = (((row*N+col) % 2 == 0) && ((row*N+col) > 0) && ((row*N+col) < N*N-1)); 


  for (int i = N*row; i < (N * (row + 1)); i++) { 
    if (i == row*N + col) 
      continue; 
    if (board[i] == opp) opWon += 1; 
    if (board[i] == player) iWon += 1; 
  }

  if (iWon == 2 && opWon == 0) myScore += 2; 
  if (opWon == 2) hisScore -= 2; 

  opWon = 0;  
  iWon = 1; 
  for (int i = col; i < N*N; i += N) {
    if (i == row*N + col) 
      continue; 
    if (board[i] == opp) opWon += 1; 
    if (board[i] == player) iWon += 1;
  }

  if (iWon == 2 && opWon == 0) myScore += 2; 
  if (opWon == 2) hisScore -= 2; 

  if (diagOne) { 
    opWon = 0;  
    iWon = 1; 
    for (int i = 0; i < N*N; i += 4) {
      if (i == row*N+col) 
        continue; 
      if (board[i] == opp) opWon += 1; 
      if (board[i] == player) iWon += 1;
    }
    if (iWon == 2 && opWon == 0) myScore += 2; 
    if (opWon == 2) hisScore -= 2; 
  }

  if (diagTwo) { 
    opWon = 0;  
    iWon = 1; 
    for (int i = 2; i < N*N-1; i += 2) {
      if (i == row*N+col) 
        continue; 
      if (board[i] == opp) opWon += 1; 
      if (board[i] == player) iWon += 1;
    }
    if (iWon == 2 && opWon == 0) myScore += 2; 
    if (opWon == 2) hisScore -= 2; 
  }


  if (isWinner(row, col, board, player)) 
    //add 5 for any small board win
    myScore += 5;

  if (player == 'O') { 
    res[0] = myScore;
    res[1] = hisScore; 
  } else {
    res[0] = hisScore;
    res[1] = myScore;
  }

  return res;

}

//for mini board
bool boardComplete(char* local_board) { 
  for (int i = 0; i < N*N; i++) { 
    if ((int)local_board[i] == 0)
      return false;
  }

  return true; 
}

//returns which board the opponent needs to make a move in next 
//if that board is complete, returns -1 
int makeMove(board_t* meta_board, int local_board_idx, int row, int col, char player) { 
  int next_board_idx = row*N+col; 
  if((int) (meta_board[local_board_idx].board[next_board_idx]) != 0) {
    printf("dumb nuts local = %d r = %d, c = %d, before = %d, new = %d\n", local_board_idx, row, col, 
      (int) (meta_board[local_board_idx].board[next_board_idx]), (int) player);
  } 
  meta_board[local_board_idx].board[next_board_idx] = player; 
  meta_board[local_board_idx].numFilled++;
  if (isWinner(row, col, meta_board[local_board_idx].board, player)) {
    meta_board[local_board_idx].status = player; 
  } else if (boardComplete(meta_board[local_board_idx].board)) { //tie
    meta_board[local_board_idx].status = 1; 
  }
  if ((int)meta_board[next_board_idx].status == 0) 
    return next_board_idx; 

  return -1; 
}

/**
 * alpha - worst case for maximizing player
 * beta - best case for minimizing player
 * botMaximizing - true if its the bot's turn
 * keep track of how many X's and O's have already been placed on the board
 */
node_t alphabeta(node_t node, int depth, int alpha, int beta, bool botMaximizing, 
  int num_of_threads, board_t *meta_board, int metaIndex) {

  // int nextIndex = (node.row * N) + node.col;
  int numChildren = 0;
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      // if((int)meta_board[nextIndex].board[r*N+c] == 0) {
      if((int)meta_board[metaIndex].board[r*N+c] == 0) {
        numChildren++;
      }
    }
  }
  
  if(depth == 0 || numChildren == 0) {
    // printf("row = %d, col = %d -- ", node.row, node.col);
    return node;
  }
  //generate tree
  node_t *children = (node_t *) malloc(numChildren * sizeof(node_t));
  int childIndex = 0;

  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      int index = i*N + j;
      // if((int)meta_board[nextIndex].board[index] == 0) {
      if((int)meta_board[metaIndex].board[index] == 0) {
        int* scoreArr = (int *) malloc(2*sizeof(int));
        scoreArr[0] = alpha;
        scoreArr[1] = beta; 
        // int* val = calculateSmallBoardScore(meta_board[nextIndex].board, i, j, 
        //   botMaximizing ? 'O' : 'X', botMaximizing ? 'X' : 'O', scoreArr);
        int* val = calculateSmallBoardScore(meta_board[metaIndex].board, i, j, 
          botMaximizing ? 'O' : 'X', botMaximizing ? 'X' : 'O', scoreArr);
        free(scoreArr);
        node_t temp = node_t();
        temp.value = botMaximizing ? val[0] - val[1] : val[1] - val[0]; 
        temp.row = i;
        temp.col = j;
        temp.metaIdx = metaIndex; 
        children[childIndex] = temp;
        childIndex++;
        free(val);
      }
    }
  }

  node_t result = node_t();
  result.row = children[0].row;
  result.col = children[0].col;
  if(botMaximizing) {
    result.value = INT_MIN;
    int i;
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      // meta_board[nextIndex].board[children[i].row*N+children[i].col] = 'O';
      meta_board[metaIndex].board[children[i].row*N+children[i].col] = 'O';
      // node_t ab = alphabeta(children[i], depth-1, alpha, beta, false, num_of_threads, 
      //   meta_board, nextIndex);
      node_t ab = node_t();
      if(isWinner(children[i].row, children[i].col, meta_board[metaIndex].board, 'O')) {
        ab.value = INT_MAX;
        ab.row = children[i].row;
        ab.col = children[i].col;
        ab.metaIdx = metaIndex; 
      } 
      else {
        ab = alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, num_of_threads, 
          meta_board, children[i].row*N+children[i].col);
      }
      // meta_board[nextIndex].board[children[i].row*N+children[i].col] = 0;
      meta_board[metaIndex].board[children[i].row*N+children[i].col] = 0;
      if(ab.value > result.value) {
        result.value = ab.value;
        result.row = children[i].row;
        result.col = children[i].col;
        result.metaIdx = ab.metaIdx; //metaIndex 
      }
      alpha = max(alpha, result.value);
      if(beta <= alpha) break;
    }
  }
  else {
    result.value = INT_MAX;
    int i;
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      // meta_board[nextIndex].board[children[i].row*N+children[i].col] = 'X';
      meta_board[metaIndex].board[children[i].row*N+children[i].col] = 'X';
      // node_t ab = alphabeta(children[i], depth-1, alpha, beta, true, num_of_threads, 
      //   meta_board, nextIndex);
      node_t ab = node_t();
      if(isWinner(children[i].row, children[i].col, meta_board[metaIndex].board, 'X')) {
        ab.value = INT_MIN;
        ab.row = children[i].row;
        ab.col = children[i].col;
        ab.metaIdx = metaIndex;
      } 
      else {
        ab = alphabeta(children[i], depth-1, alpha, beta, !botMaximizing, num_of_threads, 
          meta_board, children[i].row*N+children[i].col);
      }
      // meta_board[nextIndex].board[children[i].row*N+children[i].col] = 0;
      meta_board[metaIndex].board[children[i].row*N+children[i].col] = 0;
      if(ab.value < result.value) {
        result.value = ab.value;
        result.row = children[i].row;
        result.col = children[i].col;
        result.metaIdx = ab.metaIdx;
      }
      beta = min(beta, result.value);
      if(beta <= alpha) break;
    }
  }
  free(children);
  return result;
}

node_t metaAlphabeta(node_t *nodePtr, int depth, int alpha, int beta, bool botMaximizing, 
  char *abBoard, int num_of_threads) {
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
        res.metaIdx = node.row*N + node.col;
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
        res.metaIdx = node.row*N + node.col;
        return res;
      } 
    }
  } 
  
  int numChildren = 0;
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      if((int)board[r*N+c] == 0) {
        numChildren++;
      } 
    }
  } 
  if(depth == 0 || numChildren == 0) {
    return *nodePtr;
  }

  //generate tree
  node_t *children = (node_t *) malloc(numChildren * sizeof(node_t));
  int childIndex = 0;
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      int index = i*N + j;
      if((int)board[index] == 0) {
        int* scoreArr = (int *) malloc(2*sizeof(int));
        scoreArr[0] = alpha;
        scoreArr[1] = beta; 
        int* val = calculateSmallBoardScore(board, i, j, 
          botMaximizing ? 'O' : 'X', botMaximizing ? 'X' : 'O', scoreArr);
        free(scoreArr);
        node_t temp = node_t();
        temp.value = botMaximizing ? val[0] - val[1] : val[1] - val[0]; 
        temp.row = i;
        temp.col = j;
        children[childIndex] = temp;
        temp.metaIdx = i*N + j;
        childIndex++;
        free(val);
      }
    }
  }
  node_t result = node_t();
  if(botMaximizing) {
    result.value = INT_MIN;
    int i;
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      node_t ab = metaAlphabeta(&(children[i]), depth-1, alpha, beta, false, board, num_of_threads);
      if(ab.value > result.value) {
        result.value = ab.value;
        result.row = children[i].row;
        result.col = children[i].row;
        result.metaIdx = ab.row*N+ab.col;
      }
      alpha = max(alpha, result.value);
    }
  }
  else {
    result.value = INT_MAX;
    int i;
    // #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
    for(i = 0; i < numChildren; i++) {
      node_t ab = metaAlphabeta(&(children[i]), depth-1, alpha, beta, true, board, num_of_threads);
      if(ab.value < result.value) {
        result.value = ab.value;
        result.row = children[i].row;
        result.col = children[i].col;
        result.metaIdx = ab.row*N+ab.col;
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
    if (rowWin) {
      // printf("row i = %d\n", i);
      return true;
    } 
  }

  for (int i = 0; i < N; i++) { 
    colWin = true; 
    for (int j = 0; j<N; j++) { 
      if(meta[i + j*N].status != player)
        colWin = false;
    }
    if (colWin) {
      // printf("col i = %d\n", i);
      return true; }
  }

  for (int i = 0; i < N; i++) { 
    if (meta[i * N + i].status != player)
      diagWin = false; 
  }
  if (diagWin) {
    // printf("diag 1 \n");
    return true; 
  }

  diagWin = true; 
  for (int i =0; i < N; i++) { 
    if (meta[i * N + (N-(i+1))].status != player)
        diagWin = false;
  }
  if (diagWin) {
    // printf("diag 2 \n");
    return true;
  } 

  return false;
}

int* updatePlayerHeuristic(board_t* meta_board, int small_idx, int row, int col, char player, char opp, int* oldScore) {
  int center = 1 * N + 1; 
  int* res = (int *) calloc(2, sizeof(int)); 
  
  int oldPlayerScore;
  int oldOppScore; 
  if (player == 'O') { 
    oldPlayerScore = oldScore[0];
    oldOppScore = oldScore[1];
  } else {
    oldPlayerScore = oldScore[1];
    oldOppScore = oldScore[0]; 
  }

  int myScore = oldPlayerScore; 
  int hisScore = oldOppScore; 

  int metaRow = small_idx / N; 
  int metaCol = small_idx % N; 
  bool metaDiag1 = (small_idx % 4 == 0); 
  bool metaDiag2 = (small_idx % 2 == 0 && small_idx < N*N-1 && small_idx > 0); 

  //add 3 for any move in the center board
  if (small_idx == center) {
    myScore += 3; 
  }
  int opWon = 0;  
  int iWon = 1; 
  int fail = 0;

  if (isWinner(row, col, meta_board[small_idx].board, player)) {

    if (isMetaWinner(meta_board, player)) {
      res[0] = INT_MAX;
      res[1] = hisScore; 
      return res; 
    }

    //add 5 for any small board win
    myScore += 5;
    if (small_idx == center)
      //add 10 for winning center board
      myScore += 10; 
    if (small_idx != center && small_idx % 2 == 0) 
      //add 3 for winning corner board
      myScore += 3; 

    //add 4 for getting a sequence of 2 unblocked winning boards 
    //subtract 4 from the opponent for blocking their sequence of winning boards 

    opWon = 0;  
    iWon = 1; 
    fail = 0; 

    for (int i = N*metaRow; i < (N * (metaRow + 1)); i++) { 
      if (i == small_idx) 
        continue; 
      char status = meta_board[i].status; 
      if (status == player) iWon += 1; 
      if (status == opp) opWon += 1; 
      if (status == 1) fail += 1; 
    }

    if (iWon == 2 && fail == 0 && opWon == 0) myScore += 4; 
    if (opWon == 2) hisScore -= 4; 

    opWon = 0;  
    iWon = 1; 
    fail = 0; 
    for (int i = metaCol; i < N*N; i += N) {
      if (i == small_idx) 
        continue; 
      char status = meta_board[i].status; 
      if (status == player) iWon += 1; 
      if (status == opp) opWon += 1; 
      if (status == 1) fail += 1; 
    }

    if (iWon == 2 && fail == 0 && opWon == 0) myScore += 4; 
    if (opWon == 2) hisScore -= 4; 

    if (metaDiag1) { 
      opWon = 0;  
      iWon = 1; 
      fail = 0; 
      for (int i = 0; i < N*N; i += 4) {
        if (i == small_idx) 
          continue; 
        char status = meta_board[i].status; 
        if (status == player) iWon += 1; 
        if (status == opp) opWon += 1; 
        if (status == 1) fail += 1; 
      }
      if (iWon == 2 && fail == 0 && opWon == 0) myScore += 4; 
      if (opWon == 2) hisScore -= 4; 
    }

    if (metaDiag2) { 
      opWon = 0;  
      iWon = 1; 
      fail = 0; 
      for (int i = 2; i < N*N-1; i += 2) {
        if (i == small_idx) 
          continue; 
        char status = meta_board[i].status; 
        if (status == player) iWon += 1; 
        if (status == opp) opWon += 1; 
        if (status == 1) fail += 1; 
      }
      if (iWon == 2 && fail == 0 && opWon == 0) myScore += 4; 
      if (opWon == 2) hisScore -= 4; 
    }
  }

  if (player == 'O') { 
    res[0] = myScore;
    res[1] = hisScore; 
  } else {
    res[0] = hisScore;
    res[1] = myScore;
  }

  return res;
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
    meta_board[i].numFilled = 0; 
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

    int nextIsTBD = false; //if next mini board is not yet decided because the calculated one is already completed
    int depth = 2;

    //TODO start with random row and column 
    node_t root = node_t();
    root.row = 1;
    root.col = 1;
    root.metaIdx = 0;
    // root.value = calculateValue(board, 1,1, false);
    int* scoreArr = (int *) malloc(2*sizeof(int));
    scoreArr[0] = 0;
    scoreArr[1] = 0; 
    int* val = calculateSmallBoardScore(meta_board[0].board, 1, 1, 'O', 'X', scoreArr);
    root.value = val[0] - val[1]; 
    free(scoreArr);
    free(val);
    makeMove(meta_board, 0, root.row, root.col, 'O');
    // updateMetaCLI(meta_board);
    bool firstMove = true;

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

      //TODO change this to check if it is the first turn or not instead of hardcoded coordinates
      if(!firstMove) {
        int nextIndex0;
        if(nextIsTBD) {
          //TODO meta_board -> char* board
          char *tempBoard = (char *) calloc(N*N, sizeof(char));
          for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
              tempBoard[i*N+j] = meta_board[i*N+j].status;
            }
          }
          node_t metaRes = metaAlphabeta(NULL, 1, INT_MIN, INT_MAX, true, tempBoard,num_of_threads);
          free(tempBoard);
          nextIndex0 = metaRes.row*N+metaRes.col;
        } else {
          nextIndex0 = root.row*N+root.col;
        }
        
        node_t res0 = alphabeta(root, depth, INT_MIN, INT_MAX, true, num_of_threads, meta_board, nextIndex0);
        int mm0 = makeMove(meta_board, nextIndex0, res0.row, res0.col, 'O');
        // updateMetaCLI(meta_board);

        if(mm0 != res0.row*N+res0.col ) {
          nextIsTBD = true;
        } else {
          nextIsTBD = false;
        }
        if(isMetaWinner(meta_board, 'O')) {
          winner = 'O';
          printf("O Won! numTurns = %d\n", numTurns);
          updateMetaCLI(meta_board);
          break; 
        }
        numTurns++;
        if(numTurns == N*N*N*N) {
          printf("Tie! %d\n", numTurns);
          updateMetaCLI(meta_board);
          break;
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
        node_t metaRes = metaAlphabeta(NULL, 1, INT_MIN, INT_MAX, false, tempBoard, num_of_threads);
        free(tempBoard);
        nextIndex = metaRes.row * N + metaRes.col;
      }
      else {
        nextIndex = root.row*N+root.col;
      }
      node_t res = alphabeta(root, depth, INT_MIN, INT_MAX, false, num_of_threads, meta_board, nextIndex);
      //printf("X meta index = %d\n", res.metaIdx);
      int mm = makeMove(meta_board, nextIndex, res.row, res.col, 'X');
      updateMetaCLI(meta_board);

      if(mm != res.row*N+res.col) {
        nextIsTBD = true;
      } else {
        nextIsTBD = false;
      }
      if(isMetaWinner(meta_board, 'X')) {
        winner = 'X';
        printf("X Won! numturns = %d\n", numTurns);
        updateMetaCLI(meta_board);
        break;
      }

      numTurns++;
      if(numTurns == N*N*N*N) {
        printf("Tie! %d\n", numTurns);
        updateMetaCLI(meta_board);
        break;
      }

      root.row = res.row;
      root.col = res.col;
      root.value = res.value;
      if(firstMove) firstMove = false;
    }
  }

  //TODO write output to file
  /*char output_filename[BUFSIZE];
  sprintf(output_filename, "file_outputs/output_%d.txt", num_of_threads);
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
