/*
 * state.cpp
 *
 *  Created on: 2022. 7. 20.
 *      Author: user
 */




#include "state_base.h"

state_base::state_base(double* state, double remain, double progress, int currentProcessing, int num, int order, double latency, int completeNum, double reward){
        this->state = state;
        this->remain = remain;
        this->progress = progress;
        this->currentProcessing = currentProcessing;
        this->num = num;
        this->order = order;
        this->latency = latency;
        this->completeNum = completeNum;
        this->reward = reward;
    }
