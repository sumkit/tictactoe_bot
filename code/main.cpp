#include <iostream>
#include <climits>

#include "main.h"

const int N = 3; //dimensions of board

/**
 * alpha - worst case for maximizing player
 * beta - best case for minimizing player
 * botMaximizing - true if its the user's turn (wants to calculate best case scenario for bot)
 */
void alphabeta(node_t *node, int depth, int alpha, int beta, bool botMaximizing) {
  if(depth == 0 || node->children == NULL)
    return node->value;
  if(botMaximizing) {}
  else {}
}

void runCLI() {
  for(int outer = 0; outer < N; outer++) {
    for(int i =0 ; i< N; i++) {
      printf(" |"); 
    }
    printf("\n");
    for(int i = 0; i < N; i++) {
      printf("- ");
    }
    printf("\n");
  }
}

int main() {
  int depth = 5; //number of turns taken (depth/2 game cycles)
  alphabeta(root, depth, INT_MIN, INT_MAX, true); //let user go first
  return 0;
}
