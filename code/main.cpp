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
  /*for(int c = 0; c < N*N; c++) {
    printf(" -");
  }
  printf("\n");
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
  printf("\n");*/
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      printf("r = %d, c = %d\n", r, c);
      updateCLI(meta_board[r*N+c].board);
      printf("\n");
    }
  }
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

//for mini board
bool boardComplete(char* local_board) { 
  for (int i = 0; i < N*N; i++) { 
    if ((int)local_board[i] == 0)
      return false;
  }

  return true; 
}

//for mini board
bool isWinner(int row, int col, char* board, char player, int metaIndex, bool fromMakeMove) {
  bool rowWin = false;
  bool colWin = false; 
  bool diagWin = true; 

  for (int i = 0; i < N; i++) { 
    rowWin = true; 
    for (int j = 0; j < N; j++) { 
      if (board[i*N + j] != player) rowWin = false; 
    }
    if (rowWin) {
      /*if(fromMakeMove) {
        updateCLI(board);
        printf("row i = %d, meta index = %d\n", i, metaIndex);}*/
      return true;
    } 
  }

  for (int i = 0; i < N; i++) { 
    colWin = true; 
    for (int j = 0; j<N; j++) { 
      if(board[i + j*N] != player) colWin = false;
    }
    if (colWin) {
      // if(fromMakeMove) printf("col i = %d, meta index = %d\n", i, metaIndex);
      return true; 
    }
  }

  for (int i = 0; i < N; i++) { 
    if (board[i * N + i] != player)
      diagWin = false; 
  }
  if (diagWin) {
    // if(fromMakeMove) printf("diag 1, meta index = %d\n", metaIndex);
    return true; 
  }

  diagWin = true; 
  for (int i =0; i < N; i++) { 
    if (board[i * N + (N-(i+1))] != player)
        diagWin = false;
  }
  if (diagWin) {
    // if(fromMakeMove) printf("diag 2, meta index = %d\n", metaIndex);
    return true;
  } 

  return false;

  /*bool rowWin = true;
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
  return rowWin || colWin || fwdDiagWin || backDiagWin;*/
}

//returns which board the opponent needs to make a move in next 
//if that board is complete, returns -1 
int makeMove(board_t* meta_board, int local_board_idx, int row, int col, char player) { 
  int next_board_idx = row*N+col; 
  /*if(meta_board[local_board_idx].board[next_board_idx] != 0) {
    printf("dumb nuts local = %d r = %d, c = %d\n", local_board_idx, row, col);
    updateCLI(meta_board[local_board_idx].board);
  } */
  meta_board[local_board_idx].board[next_board_idx] = player; 
  meta_board[local_board_idx].numFilled++;
  // printf("player: %c, local_board_idx: %d, row: %d, col: %d\n", player, local_board_idx, row, col);
  /*if (isWinner(row, col, meta_board[row*N+col].board, player, local_board_idx, true)) {
    meta_board[local_board_idx].status = player; 
    printf("index = %d, p = %c\n", local_board_idx, player);
  } */

  if (boardComplete(meta_board[row*N+col].board)) //tie
    meta_board[local_board_idx].status = 1; 

  if ((int)meta_board[next_board_idx].status == 0) 
    return next_board_idx; 

  return -1; 
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
      if(isWinner(node.row, node.col, board, 'O', 0, false)) {
        node_t res = node_t();
        res.value = INT_MAX;
        res.row = node.row;
        res.col = node.col;
        return res;
      } 

    }
    else {
      board[node.row*N+node.col] = 'X';
      if(isWinner(node.row, node.col, board, 'X', 0, false)) {
        node_t res = node_t();
        res.value = INT_MIN;
        res.row = node.row;
        res.col = node.col;
        return res;
      } 
    }
  } 
  
  int numChildren = 0;
  for(int r = 0; r < N; r++) {
    for(int c = 0; c < N; c++) {
      if((int)board[r*N+c] == 0) numChildren++;
    }
  }
  if(numChildren != ((N*N)-numTurns)) 
    printf("numChildren = %d, numTurns = %d\n", numChildren, (N*N)-numTurns);
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

int* updatePlayerHeuristic(board_t* meta_board, int small_idx, int row, int col, 
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

  int metaRow = small_idx / N; 
  int metaCol = small_idx % N; 
  bool metaDiag1 = (small_idx % 4 == 0); 
  bool metaDiag2 = (small_idx % 2 == 0 && small_idx < N*N-1 && small_idx > 0); 

  //add 3 for any move in the center board
  if (small_idx == center) {
    myScore += 3; 
  }

  //add 3 for the center of any board
  if (row * N + col == center) {
    myScore += 3;
  }

  int opWon = 0;  
  int iWon = 1; 
  int fail = 0;

  //add 2 for getting a sequence of two tiles
  //subtract 2 from the opponent for blocking their sequence of two tiles

  char* board = meta_board[small_idx].board; 
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

  if (isWinner(row, col, board, player, 0, false)) {

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
    int numCompleteBoards = 0;

    int nextIsTBD = false; //if next mini board is not yet decided because the calculated one is already completed

    //TODO start with random row and column 
    node_t root = node_t();
    root.row = 1;
    root.col = 1;
    // root.value = calculateValue(board, 1,1, false);
    root.value = calculateSmallBoardScore(meta_board[0].board, 'O', 'X');
    makeMove(meta_board, 0, root.row, root.col, 'O');
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
          node_t metaRes = alphabeta(NULL, 1, INT_MIN, INT_MAX, true, tempBoard, numCompleteBoards, num_of_threads);
          free(tempBoard);
          nextIndex0 = metaRes.row*N+metaRes.col;
        } else {
          nextIndex0 = root.row*N+root.col;
        }
        node_t res0 = alphabeta(&root, 1, INT_MIN, INT_MAX, false, meta_board[nextIndex0].board, 
            meta_board[nextIndex0].numFilled, num_of_threads);
        int mm0 = makeMove(meta_board, nextIndex0, res0.row, res0.col, 'O');
        // printf("meta = %d, row = %d, col = %d\n", nextIndex0, res0.row, res0.col);
        // updateMetaCLI(meta_board);

        if(mm0 != res0.row*N+res0.col ) {
          nextIsTBD = true;
          numCompleteBoards++;
        } else {
          nextIsTBD = false;
        }
        if(isMetaWinner(meta_board, 'O')) {
          winner = 'O';
          printf("O Won! numTurns = %d\n", numTurns);
          updateMetaCLI(meta_board);
          break; 
        }
        if(isWinner(res0.row, res0.col, meta_board[nextIndex0].board, 'O', 0, false)) {
          // printf("O Won mini board\n");
        }
        numTurns++;
        if(numTurns == N*N*N*N) {
          printf("Tie!\n");
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
        node_t metaRes = alphabeta(NULL, 1, INT_MIN, INT_MAX, true, tempBoard, numCompleteBoards, num_of_threads);
        free(tempBoard);
        nextIndex = metaRes.row*N+metaRes.col;
      }
      else {
        nextIndex = root.row*N+root.col;
      }
      node_t res = alphabeta(&root, 1, INT_MIN, INT_MAX, true, meta_board[nextIndex].board, 
          meta_board[nextIndex].numFilled, num_of_threads);
      int mm = makeMove(meta_board, nextIndex, res.row, res.col, 'X');
      // printf("meta = %d, row = %d, col = %d\n", nextIndex, res.row, res.col);
      // updateMetaCLI(meta_board);

      if(mm != res.row*N+res.col) {
        nextIsTBD = true;
        numCompleteBoards++;
      } else {
        nextIsTBD = false;
      }
      if(isMetaWinner(meta_board, 'X')) {
        winner = 'X';
        printf("X Won! numturns = %d\n", numTurns);
        updateMetaCLI(meta_board);
        break;
      }
      if(isWinner(res.row, res.col, meta_board[nextIndex].board, 'X', 0, false)) {
        // printf("X Won mini board\n");
      }
      numTurns++;
      if(numTurns == N*N*N*N) {
        printf("Tie!\n");
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
