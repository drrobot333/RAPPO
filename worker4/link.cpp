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
    if(linkStartTime == -1){ // ��ũ�� ������ �����Ͱ� ������
        //waitingQueue.push(computeDemand);
        waiting += computeDemand;
        linkStartTime = currentTime;
    }
    else{ // ��ũ�� ������ �����Ͱ� �־�����
        transfer(currentTime);
        // �и� ������ �� ������

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

        if(waiting == 0){ // ��ũ �ٺ������� -1
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



