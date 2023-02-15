/*
 * queue.cpp
 *
 *  Created on: 2022. 7. 14.
 *      Author: user
 */


#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class queue : public cSimpleModule
{
protected:
    virtual void initialize() override;
    cQueue queue;
};

void queue::initialize()
    {
        EV << "my name is queue";
    }

Define_Module(queue);
