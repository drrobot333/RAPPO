/*
 * dataplane.h
 *
 *  Created on: 2022. 7. 13.
 *      Author: user
 */

#ifndef DATAPLANE_H_
#define DATAPLANE_H_

#include <vector>
#include <queue>
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <typeinfo>

#include "myFunction.h"
#include "job_base.h"
#include "state_base.h"
#include "subtask_m.h"
#include "state_m.h"
#include "link.h"

#include <omnetpp.h>


using namespace omnetpp;

class dataplane : public cSimpleModule{
public:
    // 상수
    int availableJobNum;
    int jobWaitingQueueLength;
    int nodeNum;
    int modelNum;

    std::vector<cModule*> gateVector;

    cMessage* test = nullptr;
    cMessage* sendStateMsg = nullptr;
    cMessage* checkTimeMsg = nullptr;

    link*** adjacentMatrix;
    std::vector<link*> adjacentList;

    int episode_length;
    int episode_num = 0;
    bool reset_network;

    double time_step_interval = 0.5;
    //


    job_base** activatedJobQueue; // cpp 에서 선언됨
    std::queue<job_base *> jobWaitingQueue;
    std::queue<job_base *> backlog;

    double** nodeState;
    double* jobRemain;
    int** nodeProcessingState;

    HANDLE hPipe; // 파이프 사용하기 위한 핸들러

    bool isActioned;
    bool isWaiting = false;

    double latency;
    double reward;
    double jobProgress = 0;
    int completeNum;
    int activatedJobNum = 0;
    int totalCompleteNum = 0;
    int state_finish_num = 0;
    double episodeReward = 0;
    int job_generate_rate;
    double low; // node capacity
    double high; // source node capacity

    simtime_t latestSendTime = 0;
    simtime_t episode_start_time = 0;

    std::string current_state;
    std::string previous_state;




public:
    virtual void initialize() override;
    virtual int activateNewJob();
    virtual void pushNewJob(job_base* newJob);
    virtual void moveJob();
    virtual void assignIndex(job_base* newJob, int index);
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;
    virtual int getActivatedJobNum();
    virtual int scheduleJob(job_base* newJob);
    virtual void sendState();
    virtual DWORD sendPythonMessage(std::string msg);
    virtual std::string getPythonMessage();
    virtual void resetNetwork();
    virtual std::string getStateMsg(job_base* newJob, bool isAction);
    void getIotState();

};

Define_Module(dataplane);



#endif /* DATAPLANE_H_ */
