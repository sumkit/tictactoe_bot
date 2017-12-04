struct node_t {
  // node_t *children;
  int value;
  int row;
  int col;
};

struct board_t { 
  char* board; 
  char status; 
  int numFilled;
};