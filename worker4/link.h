/*
 * link.h
 *
 *  Created on: 2022. 7. 18.
 *      Author: user
 */

#ifndef LINK_H_
#define LINK_H_

#include <omnetpp.h>
#include <queue>

class link{
protected:
    int incidence1;
    int incidence2;
    int linkCapacity;
    double waiting = 0;
    std::queue<int> waitingQueue;
    omnetpp::simtime_t linkStartTime = -1;

public:
    link(int i, int j, int capacity);

    virtual double transferCompleteTime(int computeDemand, omnetpp::simtime_t currentTime);
    virtual void transfer(omnetpp::simtime_t currentTime);
    virtual int getIncidence1() const;
    virtual int getIncidence2() const;
    virtual int getLinkCapacity() const;
    virtual double getWaiting() const;
    virtual void setWaiting(double waiting);


    void setLinkStartTime(const omnetpp::simtime_t &linkStartTime = -1) {
        this->linkStartTime = linkStartTime;
    }

    void setLinkCapacity(int linkCapacity) {
        this->linkCapacity = linkCapacity;
    }
};


#endif /* LINK_H_ */
