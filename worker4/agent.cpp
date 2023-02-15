/*
 * agent.cpp
 *
 *  Created on: 2022. 7. 11.
 *      Author: user
 */


#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "myFunction.h"

using namespace omnetpp;

/**
 * Let's make it more interesting by using several (n) `tic' modules,
 * and connecting every module to every other. For now, let's keep it
 * simple what they do: module 0 generates a message, and the others
 * keep tossing it around in random directions until it arrives at
 * module 3.
 */
class agent : public cSimpleModule
{
private:
public:
    virtual void initialize() override;
};



Define_Module(agent);


void agent::initialize(){
}
