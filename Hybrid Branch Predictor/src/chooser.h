//Anu Dharshini Balraj(233004578),Sree Keerthi Chandar(333000085),Meghana Jaysing Amup(632000331)
// Chooser for Hybrid brand prediction
// This file chooses best predictor between two branch predictors for next prediction
// We have used piecewise linear perceptron and tage predictors to create a hybrid predictor
// Reference:https://github.com/chinmayachaudhry/Branch-Predictor

#include "piecewise_perceptron.h"
#include "tage.h"
#include <cmath>
#include <iostream>


class chooser_update : public branch_update {
public:
  unsigned int C_index;
};

class chooser : public branch_predictor {
private:
#define C_HISTORY_LENGTH 16
#define C_TABLE_BITS 8
#define C_TABLE_SIZE (1 << C_TABLE_BITS)

  chooser_update u;
  branch_info bi, bi2;
  bool prediction;
  unsigned int targets[C_TABLE_SIZE];
  int chooser_hist[C_TABLE_SIZE];

  int output;

  piecewise_perceptron *p1;
  tage *p2;

public:
  chooser(void) {
    // std::cout << TABLE_SIZE << " " << GA_SIZE << "/n";
   
    // initiate chooser history table
    for (int i = 0; i < C_TABLE_SIZE; i++)
      chooser_hist[i] = 1;
    
    // create objects for both predictors
    p1 = new piecewise_perceptron();
    p2 = new tage();

  }

  branch_update *predict(branch_info &b) {
    bi = b;
    //Hash branch address
    int addr = b.address & (C_TABLE_SIZE - 1);

    if (b.br_flags & BR_CONDITIONAL) {
      // std::cout << "*****************************";
      // check chooser table to decide which predictor will be acurate
      if (chooser_hist[addr] >= 2) {
        //Select predictor 1
        branch_update *p1_u = (branch_update *)p1->predict(b);
        u.direction_prediction(p1_u->direction_prediction());
      } else {
	      //select predictor 2
        branch_update *p2_u = (branch_update *)p2->predict(b);
        u.direction_prediction(p2_u->direction_prediction());
      }

    } else {
      u.direction_prediction(true);
    }
    if (b.br_flags & BR_INDIRECT) {
      u.target_prediction(targets[addr]);
      // u.target_prediction (0);
    }
    return &u;
  }

  void update(branch_update *u, bool taken, unsigned int target) {
    int addr = bi.address & (C_TABLE_SIZE - 1);
    // std::cout << " " << p2->predict(bi)->direction_prediction() << " " << p1->predict(bi)->direction_prediction() << "\n";
    
    //Predictor 1 is correct and predictor 2 is incorrect
    if ((taken == p1->predict(bi)->direction_prediction())&&(taken != p2->predict(bi)->direction_prediction())) {
 
        if (chooser_hist[addr] < 3)
          chooser_hist[addr]++;
        else 
          chooser_hist[addr] = 3;
    //predictor 2 is correct and predictor 1 is incorrect
    } else if ((taken == p2->predict(bi)->direction_prediction())&& (taken!= p1->predict(bi)->direction_prediction())) {


        if (chooser_hist[addr] > 0)
          chooser_hist[addr]--;
        else
          chooser_hist[addr] = 0;

    }
    p1->update(u, taken, target);
    // std::cout << "##? " << (p2 == NULL) <<"/n";
    p2->update(u, taken, target);
  }
};
