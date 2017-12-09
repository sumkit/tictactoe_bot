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
void updateMetaCLI(board_t *meta_board, bool file, FILE *output_file) {
  if(file) fprintf(output_file, "META: \n");
  else printf("META: \n");
  for(int i = 0; i < N; i++) {
    for(int j = 0; j < N; j++) {
      char temp = meta_board[i*N+j].status;
      if(temp == 0) {
        if(file) fprintf(output_file, " |");
        else printf(" |");
      } else {
        if(file) fprintf(output_file, "%c|", temp);
        else printf("%c|", temp);
      }
    }
    if(file) fprintf(output_file, "\n");
    else printf("\n");
  }
  printf("\n");
  
  for(int c = 0; c < N*N; c++) {
    if(file) fprintf(output_file, " -");
    else printf(" -");
  }
  if(file) fprintf(output_file, "\n");
  else printf(" -");
  int nonBlank = 0;
  for(int n = 0; n < N; n++) {
    //TODO show if the meta_board square is a win or tie 
    for(int r = 0; r < N; r++) {
      for(int n1 = 0; n1 < N; n1++) {
        for(int c = 0; c < N; c++) {
            char temp = meta_board[n*N+n1].board[r*N+c];
          if(temp == 0) {
            if(file) fprintf(output_file, " |");
            else printf(" |");
          } else {
            if(file) fprintf(output_file, "%c|", temp);
            else printf("%c|", temp);
            nonBlank++;
          }
        }
      }
      if(r < N-1) {
        if(file) fprintf(output_file, "\n");
        else printf("\n");
      }
    }
    printf("\n");
  }
  for(int c = 0; c < N*N; c++) {
    printf(" -");
  }
  if(file) fprintf(output_file, "\n");
  else printf("\n");
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

  int center = (N/2) * N + (N/2); 
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

  //add 2 for getting a sequence of N-1 tiles
  //subtract 2 from the opponent for blocking their sequence of N-1 tiles

  bool diagOne = (row==col);
  bool diagTwo = (row == (N-1)-col);  


  for (int i = N*row; i < (N * (row + 1)); i++) { 
    if (i == row*N + col) 
      continue; 
    if (board[i] == opp) opWon += 1; 
    if (board[i] == player) iWon += 1; 
  }

  if (iWon == N-1 && opWon == 0) myScore += 2; 
  if (opWon == N-1) hisScore -= 2; 

  opWon = 0;  
  iWon = 1; 
  for (int i = col; i < N*N; i += N) {
    if (i == row*N + col) 
      continue; 
    if (board[i] == opp) opWon += 1; 
    if (board[i] == player) iWon += 1;
  }

  if (iWon == N-1 && opWon == 0) myScore += 2; 
  if (opWon == N-1) hisScore -= 2; 

  if (diagOne) { 
    opWon = 0;
    iWon = 0; 
    for (int i = 0; i < N; i++) { 
      int idx = i * N + i;
      if (board[idx] == opp) opWon += 1; 
      if (board[idx] == player) iWon += 1;
    } 
  }
  if (iWon == N-1 && opWon == 0) myScore += 2; 
  if (opWon == N-1) hisScore -= 2; 


  if (diagTwo) { 
    opWon = 0;  
    iWon = 0; 
    for (int i = 0; i < N; i ++) {
      int idx = i * N + (N-1-i); 
      if (board[idx] == opp) opWon += 1; 
      if (board[idx] == player) iWon += 1;
    }
    if (iWon == N-1 && opWon == 0) myScore += 2; 
    if (opWon == N-1) hisScore -= 2; 
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
  int numChildren = 0;
  int firstChildRow, firstChildCol;
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      if((int)meta_board[metaIndex].board[r*N+c] == 0) {
        if(numChildren == 0) {
          firstChildRow = r;
          firstChildCol = c;
        }
        numChildren++;
      }
    }
  } 
  
  if(depth == 0 || numChildren == 0) {
    return node;
  }
  //generate tree
  node_t result = node_t();
  result.row = firstChildRow;
  result.col = firstChildCol;
  int i;
  #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
  for(i = 0; i < N; i++) {
    int j;
    #pragma omp parallel for default(shared) private(j) num_threads(num_of_threads)
    for(j = 0; j < N; j++) {
      int index = i*N + j;
      if((int)meta_board[metaIndex].board[index] == 0) {
        int* scoreArr = (int *) malloc(2*sizeof(int));
        scoreArr[0] = alpha;
        scoreArr[1] = beta; 
        int* val = calculateSmallBoardScore(meta_board[metaIndex].board, i, j, 
          botMaximizing ? 'O' : 'X', botMaximizing ? 'X' : 'O', scoreArr);
        free(scoreArr);
        node_t temp = node_t();
        temp.value = botMaximizing ? val[0] - val[1] : val[1] - val[0]; 
        temp.row = i;
        temp.col = j;
        temp.metaIdx = metaIndex; 
        free(val);

        if(botMaximizing) {
          result.value = INT_MIN;
          meta_board[metaIndex].board[index] = 'O';
          node_t ab = node_t();
          if(isWinner(i, j, meta_board[metaIndex].board, 'O')) {
            ab.value = INT_MAX;
            ab.row = i;
            ab.col = j;
            ab.metaIdx = metaIndex; 
          } else {
            ab = alphabeta(temp, depth-1, alpha, beta, !botMaximizing, num_of_threads, 
              meta_board, index);
          }
          meta_board[metaIndex].board[index] = 0; 
          if(ab.value > result.value) {
            result.value = ab.value;
            result.row = i;
            result.col = j;
            result.metaIdx = ab.metaIdx;  
          }
          alpha = max(alpha, result.value);
        } else {
          result.value = INT_MAX;
          meta_board[metaIndex].board[index] = 'X';
          node_t ab = node_t();
          if(isWinner(i, j, meta_board[metaIndex].board, 'X')) {
            ab.value = INT_MIN;
            ab.row = i;
            ab.col = j;
            ab.metaIdx = metaIndex;
          } 
          else {
            ab = alphabeta(temp, depth-1, alpha, beta, !botMaximizing, num_of_threads, 
              meta_board, index);
          }
          meta_board[metaIndex].board[index] = 0;
          if(ab.value < result.value) {
            result.value = ab.value;
            result.row = i;
            result.col = j;
            result.metaIdx = ab.metaIdx;
          }
          beta = min(beta, result.value);
        }
      }
    }
  }

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
  
  int numChildren = 0;
  int firstChildRow, firstChildCol;
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      if((int)board[r*N+c] == 0) {
        if(numChildren == 0) {
          firstChildRow = r;
          firstChildCol = c;
        }
        numChildren++;
      } 
    }
  } 
  if(depth == 0 || numChildren == 0) {
    return *nodePtr;
  }

  //generate tree
  node_t result = node_t();
  result.row = firstChildRow;
  result.col = firstChildCol;
  int i;
  #pragma omp parallel for default(shared) private(i) num_threads(num_of_threads)
  for(i = 0; i < N; i++) {
    int j;
    #pragma omp parallel for default(shared) private(j) num_threads(num_of_threads)
    for(j = 0; j < N; j++) {
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
        free(val);

        if(botMaximizing) {
          result.value = INT_MIN;
          node_t ab = metaAlphabeta(&temp, depth-1, alpha, beta, false, board, num_of_threads);
          if(ab.value > result.value) {
            result.value = ab.value;
            result.row = i;
            result.col = j;
          }
          alpha = max(alpha, result.value);
        }
        else {
          result.value = INT_MAX;
          node_t ab = metaAlphabeta(&temp, depth-1, alpha, beta, true, board, num_of_threads);
          if(ab.value < result.value) {
            result.value = ab.value;
            result.row = i;
            result.col = j;
          }
          beta = min(beta, result.value);
        }
      }
    }
  }
  
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

int* updatePlayerHeuristic(board_t* meta_board, char* small_board, int small_idx, char player, char opp, int* oldScore) {
  int center = N/2 * N + N/2; 
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
  bool metaDiag1 = (metaRow == metaCol); 
  bool metaDiag2 = (metaRow == (N-1)-metaCol); 

  //add 3 for any move in the center board
  if (small_idx == center) {
    myScore += 3; 
  }
  int opWon = 0;  
  int iWon = 1; 
  int fail = 0;

  if (isWinner(0, 0, small_board, player)) {

    if (isMetaWinner(meta_board, player)) {
      if (player == '0') { 
        res[0] = INT_MAX;
        res[1] = hisScore; 
      } else {
        res[0] = hisScore;
        res[1] = INT_MAX; 
      }
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

    for (int i = 0; i < N; i++) {
      int idx = i * N + metaCol;  
      if (idx == small_idx) 
        continue; 
      char status = meta_board[idx].status; 
      if (status == player) iWon += 1; 
      if (status == opp) opWon += 1; 
      if (status == 1) fail += 1; 
    }

    if (iWon == N-1 && fail == 0 && opWon == 0) myScore += 4; 
    if (opWon == N-1) hisScore -= 4; 

    opWon = 0;  
    iWon = 1; 
    fail = 0; 
    for (int i = metaCol; i < N; i++) {
      int idx = metaRow * N + i; 
      if (idx == small_idx) 
        continue; 
      char status = meta_board[idx].status; 
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
      for (int i = 0; i < N; i++) {
        int idx = i * N + i; 
        if (idx == small_idx) 
          continue; 
        char status = meta_board[idx].status; 
        if (status == player) iWon += 1; 
        if (status == opp) opWon += 1; 
        if (status == 1) fail += 1; 
      }
      if (iWon == N-1 && fail == 0 && opWon == 0) myScore += 4; 
      if (opWon == N-1) hisScore -= 4; 
    }

    if (metaDiag2) { 
      opWon = 0;  
      iWon = 1; 
      fail = 0; 
      for (int i = 0; i < N; i++) {
        int idx = i * N + ((N-1)-i); 
        if (idx == small_idx) 
          continue; 
        char status = meta_board[idx].status; 
        if (status == player) iWon += 1; 
        if (status == opp) opWon += 1; 
        if (status == 1) fail += 1; 
      }
      if (iWon == N-1 && fail == 0 && opWon == 0) myScore += 4; 
      if (opWon == N-1) hisScore -= 4; 
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

node_t metaMove(board_t* meta_board, bool botMaximizing, int num_of_threads) { 
  node_t res =  node_t(); 
  int maxScore = 0; 
  char* board; 
  int* init = (int*) calloc(2, sizeof(int));
  init[0] = 0;
  init[1] = 0;
  char player = botMaximizing ? 'O' : 'X'; 
  char opp = botMaximizing ? 'X' : 'O'; 
  for (int metaIdx = 0; metaIdx < N*N; metaIdx++) { 
    board = meta_board[metaIdx].board; 
    char status = meta_board[metaIdx].status;
    if ((int) status == 0) { 
      node_t temp = node_t(); 
      temp = alphabeta(temp, 2, INT_MAX, INT_MIN, botMaximizing, 
        num_of_threads, meta_board, metaIdx);  

      char* boardcpy = (char *) calloc(N*N, sizeof(char)); 
      memcpy(board, boardcpy, N*N); 
      boardcpy[temp.row*N + temp.col] = player; 

      int* curVal = updatePlayerHeuristic(meta_board, boardcpy, metaIdx, player, opp, init);
      int curScore;
      int updatedScore; 

      if (botMaximizing)
        curScore = curVal[0] - curVal[1];
      else
        curScore = curVal[1] - curVal[0];  
      
      int resScore = temp.value + curScore; 
      if (resScore > maxScore) { 
        maxScore = resScore; 
        res.value = resScore; 
        res.row = temp.row; 
        res.col = temp.col; 
        res.metaIdx = metaIdx; 
      }
    free(curVal); 
    free(boardcpy); 
    }
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
  {
    printf("in ifdef\n");
  }

  /* This pragma means we want the code in the following block be executed in 
   * Xeon Phi.
   */
#pragma offload target(mic) \
  in(meta_board: length(N*N))
  {
    printf("in pragma\n");
  } 
#endif
  {
    char winner;
    int numTurns = 1;

    int nextIsTBD = false; //if next mini board is not yet decided because the calculated one is already completed
    //depth = # of turns taken (depth/2 = # game cycles)
    int depth = 4;

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
    bool firstMove = true;

    while(1) {
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
          node_t meta = metaMove(meta_board, true, num_of_threads); 
          nextIndex0 = makeMove(meta_board, meta.metaIdx, meta.row, meta.col, 'O'); 
          //node_t metaRes = metaAlphabeta(NULL, 1, INT_MIN, INT_MAX, true, tempBoard, num_of_threads);
          free(tempBoard);
          //nextIndex0 = metaRes.row*N+metaRes.col;
        } else {
          nextIndex0 = root.row*N+root.col;
        }
        
        node_t res0 = alphabeta(root, depth, INT_MIN, INT_MAX, true, num_of_threads, meta_board, nextIndex0);
        int mm0 = makeMove(meta_board, nextIndex0, res0.row, res0.col, 'O');

        if(mm0 != res0.row*N+res0.col ) {
          nextIsTBD = true;
        } else {
          nextIsTBD = false;
        }
        numTurns++;
        if(isMetaWinner(meta_board, 'O')) {
          winner = 'O';
          printf("O Won! numTurns = %d\n", numTurns);
          updateMetaCLI(meta_board, false, NULL);
          break; 
        }
        
        if(numTurns == N*N*N*N) {
          printf("Tie! %d\n", numTurns);
          updateMetaCLI(meta_board, false, NULL);
          break;
        }
        root.row = res0.row;
        root.col = res0.col;
        root.value = res0.value;
        root.metaIdx = res0.metaIdx;
      }
      
      int nextIndex;

      if(nextIsTBD) {
        char *tempBoard = (char *) calloc(N*N, sizeof(char));
        for(int i = 0; i < N; i++) {
          for(int j = 0; j < N; j++) {
            tempBoard[i*N+j] = meta_board[i*N+j].status;
          }
        }

        node_t meta = metaMove(meta_board, false, num_of_threads); 
        nextIndex = makeMove(meta_board, meta.metaIdx, meta.row, meta.col, 'X'); 
        //node_t metaRes = metaAlphabeta(NULL, 1, INT_MIN, INT_MAX, false, tempBoard, num_of_threads);
        free(tempBoard);
        //nextIndex = metaRes.row * N + metaRes.col;
      }
      else {
        nextIndex = root.row*N+root.col;
      }
      node_t res = alphabeta(root, depth, INT_MIN, INT_MAX, false, num_of_threads, meta_board, nextIndex);
      int mm = makeMove(meta_board, nextIndex, res.row, res.col, 'X');

      if(mm != res.row*N+res.col) {
        nextIsTBD = true;
      } else {
        nextIsTBD = false;
      }
      numTurns++;
      if(isMetaWinner(meta_board, 'X')) {
        winner = 'X';
        printf("X Won! numturns = %d\n", numTurns);
        updateMetaCLI(meta_board, false, NULL);
        break;
      }

      if(numTurns == N*N*N*N) {
        printf("Tie! %d\n", numTurns);
        updateMetaCLI(meta_board, false, NULL);
        break;
      }

      root.row = res.row;
      root.col = res.col;
      root.value = res.value;
      root.metaIdx = res.metaIdx;
      if(firstMove) firstMove = false;
    }
  }

  //TODO write output to file

  //const char *filename = basename("file_outputs/");
  char output_filename[BUFSIZE];
  sprintf(output_filename, "file_outputs/output_%d.txt", num_of_threads);
  FILE *output_file = fopen(output_filename, "w");
  if (!output_file) {
    printf("Error: couldn't output costs file");
    return -1;
  }
  /*for(int c = 0; c < N; c++) {
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
  }*/
  fclose(output_file); 

  free(meta_board);
  return 0;
}
