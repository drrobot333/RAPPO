/*
 * state.h
 *
 *  Created on: 2022. 7. 20.
 *      Author: user
 */

#ifndef STATE_H_
#define STATE_H_

class state_base{
public:
    double* state;
    double remain;
    double progress;
    double latency;
    double reward;
    int completeNum;
    int currentProcessing;
    int num;
    int order;

    state_base(double* state, double remain, double progress, int currentProcessing, int num, int order, double latency, int completeNum, double reward);

    int getCurrentProcessing() const {
        return currentProcessing;
    }

    void setCurrentProcessing(int currentProcessing) {
        this->currentProcessing = currentProcessing;
    }

    int getNum() const {
        return num;
    }

    void setNum(int num) {
        this->num = num;
    }

    double getProgress() const {
        return progress;
    }

    void setProgress(double progress) {
        this->progress = progress;
    }

    double getRemain() const {
        return remain;
    }

    void setRemain(double remain) {
        this->remain = remain;
    }

    double* getState() const {
        return state;
    }

    void setState(double *state) {
        this->state = state;
    }

    int getOrder() const {
        return order;
    }

    void setOrder(int order) {
        this->order = order;
    }

    int getCompleteNum() const {
        return completeNum;
    }

    double getLatency() const {
        return latency;
    }

    double getReward() const {
        return reward;
    }
};



#endif /* STATE_H_ */
