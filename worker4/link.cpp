/*
 * link.cpp
 *
 *  Created on: 2022. 7. 18.
 *      Author: user
 */


#include "link.h"


link::link(int i, int j, int capacity){
    this->incidence1 = i;
    this->incidence2 = j;
    this->linkCapacity = capacity;
}

double link::transferCompleteTime(int computeDemand, omnetpp::simtime_t currentTime){
    if(linkStartTime == -1){ // 링크로 보내는 데이터가 없으면
        //waitingQueue.push(computeDemand);
        waiting += computeDemand;
        linkStartTime = currentTime;
    }
    else{ // 링크로 보내는 데이터가 있었으면
        transfer(currentTime);
        // 밀린 데이터 다 보내고

        //waitingQueue.push(computeDemand);
        waiting += computeDemand;
        linkStartTime = currentTime;
    }

    return waiting / linkCapacity;
}

void link::transfer(omnetpp::simtime_t currentTime) {
    if(linkStartTime != -1){
        double changeTime = currentTime.dbl() - linkStartTime.dbl();
        double computed = changeTime * linkCapacity;

        if(waiting > computed){
            waiting = waiting - computed;
        }
        else{
            waiting = 0;
        }

        if(waiting == 0){ // 링크 다보냈으면 -1
            linkStartTime = -1;
        }
    }
}

int link::getIncidence1() const {
    return incidence1;
}

int link::getIncidence2() const {
    return incidence2;
}

int link::getLinkCapacity() const {
    return linkCapacity;
}

double link::getWaiting() const {
    return waiting;
}

void link::setWaiting(double waiting) {
    this->waiting = waiting;
}



