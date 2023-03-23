//Anu Dharshini Balraj(233004578),Sree Keerthi Chandar(333000085),Meghana Jaysing Amup(632000331)
// This file contains a piecewise linear branch predictor class.
// simple direct-mapped branch target buffer for indirect branch prediction.
// Reference:https://github.com/mohit-up/Piecewise_Linear_Predictor
#include <cmath>
#include <iostream>


class my_update_p : public branch_update {
public:
  unsigned int index;
};

class piecewise_perceptron : public branch_predictor {
private:
#define HISTORY_LENGTH 32
#define TABLE_BITS 10
#define TABLE_SIZE (1 << TABLE_BITS)
#define GA_BITS 10
#define GA_SIZE (1 << GA_BITS)
#define W_BITS 5

  my_update_p u;
  branch_info bi;
  bool GHR[HISTORY_LENGTH + 1];
  unsigned int GA[HISTORY_LENGTH + 1];
  unsigned int targets[TABLE_SIZE];
  int W[TABLE_SIZE][GA_SIZE][HISTORY_LENGTH + 1];
  int output;

int W_MAX, W_MIN, theta;
// int W_MAX = (1 << (W_BITS - 1));
// int W_MIN = (~(1 << (W_BITS - 1)) + )1;
// int theta = (int)(floor(2.14 * (HISTORY_LENGTH + 1)) + 20.58);

public:

  piecewise_perceptron(void) {

  //defining weight range
  W_MAX = 1 << (W_BITS - 1);
  W_MIN = ~(1 << (W_BITS - 1)) + 1;

  // Using optimim theta value from the paper Piecewise Linear Branch Prediction by Daniel A. Jimenez, 2005
  theta = (int)(floor(2.14 * (HISTORY_LENGTH + 1)) + 20.58);


    // std::cout << TABLE_SIZE << " " << GA_SIZE << "/n";
    // initialing history table
    for (int i = 0; i < TABLE_SIZE; i++) {
      for (int j = 0; j < GA_SIZE; j++) {
        for (int k = 0; k < (HISTORY_LENGTH + 1); k++) {
          W[i][j][k] = 0;
        }
      }
      targets[i] = 0;
    }
    //initializing global history resister and array of addresses
    for (int i = 0; i < HISTORY_LENGTH + 1; i++) {
      GA[i] = 0;
      GHR[i] = false;
    }
  }

  //predicts the next branch taken or not taken
  branch_update *predict(branch_info &b) {
    bi = b;
    //Hashing branch address
    int addr = b.address & (TABLE_SIZE - 1);

    if (b.br_flags & BR_CONDITIONAL) {
      output = W[addr][0][0];
      for (int i = 1; i < HISTORY_LENGTH + 1; i++) {
	//if branch was taken add the weight or otherwise substract the weight
        if (GHR[i])
          output += W[addr][GA[i]][i];
        else
          output -= W[addr][GA[i]][i];
      }
      //if output is less than zero then branch is not taken
      if (output < 0)
        u.direction_prediction(false);
      else
        u.direction_prediction(true);
    } else {
      u.direction_prediction(true);
    }
    //for indirect branches
    if (b.br_flags & BR_INDIRECT) {
      u.target_prediction(targets[addr]);
      // u.target_prediction (0);
    }
    return &u;
  }

  void update(branch_update *u, bool taken, unsigned int target) {
    //hash branch address
    int addr = bi.address & (TABLE_SIZE - 1);

    if (bi.br_flags & BR_CONDITIONAL) {
      //if wrong prediction update weights
      //weight should be less than theta to avoid overtaining
      if ((taken != u->direction_prediction()) || (abs(output) < theta)) {
        if (taken) {
	  //increment weight if branch was taken
          if (W[addr][0][0] < W_MAX) {
            W[addr][0][0]++;
          }
        } else {
	  //decrement weigh if branch was not taken
          if (W[addr][0][0] > W_MIN) {
            W[addr][0][0]--;
          }
        }
	
	//update weight history table
        for (int i = 1; i < HISTORY_LENGTH + 1; i++) {
          if (taken == GHR[i]) {
	    //increment the weight that contributed to this prediction
            if (W[addr][GA[i]][i] < W_MAX)
              W[addr][GA[i]][i]++;
            else
              W[addr][GA[i]][i] = W_MAX;
          } else {
	    //decrement the weight
            if ((W[addr][GA[i]][i] > W_MIN))
              W[addr][GA[i]][i]--;
            else
              W[addr][GA[i]][i] = W_MIN;
          }
        }
      }
      
      //Shift the current address into the global address array 
      //and current outcome to global history registor
      int i = HISTORY_LENGTH;
      while (i >= 2) {
        GA[i] = GA[i - 1];
        GHR[i] = GHR[i - 1];
        i--;
      }
      GA[1] = bi.address & (GA_SIZE - 1);
      GHR[1] = taken;
    }

    if (bi.br_flags & BR_INDIRECT) {
      targets[addr] = target;
    }
  }
};
