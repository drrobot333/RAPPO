/*
 * iot.h
 *
 *  Created on: 2022. 7. 20.
 *      Author: user
 */

#ifndef IOT_H_
#define IOT_H_

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <queue>
#include <vector>
#include <deque>
#include <stack>
#include <cmath>

#include "myFunction.h"
#include "job_m.h"
#include "subtask_m.h"
#include "output_m.h"
#include "state_m.h"
#include "job_base.h"
#include "state_base.h"
#include "link.h"
#include "dataplane.h"

using namespace omnetpp;

class iot : public cSimpleModule
{
private:
    cMessage *makeJobLoop = nullptr;


    int myIndex;
    double progressChange = 0;

    //파라미터
    int nodeNum;
    int jobGenerateRate;
    int modelNum;
    int linkCapacity;
    double nodeCapacity;
    int availableJobNum;
    bool reset_network;
    bool is_source;
    double low; // node capacity
    double high; // source node capacity
    std::vector<int> cut_position;
    //cValueArray cut_position;
    //

    double* jobRemainState;
    double jobRemain = 0;
    double jobProgress = 0;
    double jobLatency = 0;
    double reward = 0;
    int jobCompleteNum = 0;
    int episode_length;

    dataplane* dataplaneModule;
    std::queue<job_base*> jobQueue;

    std::queue<subtask_base*>* compQueue;

    std::queue<subtask_base*> realCompQueue;

    std::queue<double *> processedJobQueue; // returnJobProgress에서만 관리하는 큐


    int currentProcessing = -1;
    int currentProcessingOrder = -1;


    simtime_t sendStartTime;
    simtime_t processStartTime = -1;
    simtime_t latestComputeJobTime = -1;
    simtime_t episode_start_time = -1;

    bool isProcessing = false;


protected:
    virtual void initialize() override;
    virtual void makeJob();
    virtual void handleMessage(cMessage *msg) override;
    virtual void sendJob(output_base* output, int nodeName);
    virtual void processJob();
    virtual bool isRealCompQueueEmpty();
    virtual void routing(output_base* output);
    virtual bool isVisitedAll(bool* visited);
    virtual void refreshDisplay() const override;
    virtual double returnJobRemain();
    virtual void reset_iot();
    virtual ~iot();
public:
    virtual void computeJobRemain();
};



#endif /* IOT_H_ */
