/*
 * subtask.cpp
 *
 *  Created on: 2022. 7. 13.
 *      Author: user
 */

#ifndef JOB_BASE_H_
#define JOB_BASE_H_

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>

int randint(int a, int b);

class subtask_base{
protected:
    double compDemand;
    double outputLinkDemand;
    int order;
    int processingNode = -1; // 실행 되어야 할 노드(해당 노드에서 실행해야 하는지 확인용)
    int destination = -1; // subtask 결과 어디로 보내야 할 지 알려주는 값
    double remainComp;
    int index; // node에서 어떤 job을 process할지 판단할 때 필요함.
    bool isLast = false;
    double jobStartTime;
    double processStartTime;
    double processFinishTime;
    double totalComp;
    double generatedTime;



public:
    subtask_base(double compDemand, double outputLinkDemand, int order, double generatedTime);



    double getCompDemand() const {
        return compDemand;
    }

    int getIndex() const {
        return index;
    }

    int getOrder() const {
        return order;
    }

    double getOutputLinkDemand() const {
        return outputLinkDemand;
    }

    int getProcessingNode() const {
        return processingNode;
    }

    int getRemainComp() const {
        return remainComp;
    }

    void setProcessingNode(int processingNode = -1) {
        this->processingNode = processingNode;
    }

    void setIndex(int index) {
        this->index = index;
    }

    int getDestination() const {
        return destination;
    }

    void setDestination(int destination = -1) {
        this->destination = destination;
    }

    bool isIsLast() const {
        return isLast;
    }

    void setIsLast(bool isLast) {
        this->isLast = isLast;
    }

    double getProcessFinishTime() const {
        return processFinishTime;
    }

    void setProcessFinishTime(double processFinishTime) {
        this->processFinishTime = processFinishTime;
    }

    double getProcessStartTime() const {
        return processStartTime;
    }

    void setProcessStartTime(double processStartTime) {
        this->processStartTime = processStartTime;
    }

    double getTotalComp() const {
        return totalComp;
    }

    void setTotalComp(double totalComp) {
        this->totalComp = totalComp;
    }

    void setOutputLinkDemand(double outputLinkDemand) {
        this->outputLinkDemand = outputLinkDemand;
    }

    double getJobStartTime() const {
        return jobStartTime;
    }

    void setJobStartTime(double jobStartTime) {
        this->jobStartTime = jobStartTime;
    }

    double getGeneratedTime() const {
        return generatedTime;
    }
};

class job_base{
protected:
    int source;
    int destination;
    int model;
    double totalComp = 0;
    double generated_time;
    int* path; // 다음 노드로 가기위한 path
    std::vector<subtask_base*> subtasks;
    int compDelta = 0;
    int currentSubtaskIndex = 0;
    int index = -1;
    double inputSize;
    double jobStartTime;
    double processStartTime;
    double processFinishTime;
    double taskCompDemand[12] = {0.073, 0, 0.224, 0, 0.112, 0.145, 0.1, 0, 0, 0.038, 0.017, 0.004};
    double taskLinkDemand[12] = {0.276947, 0.0667419, 0.177979, 0.0412598, 0.0618896, 0.0618896, 0.0412598, 0.00878906, 0.00390625, 0.00390625, 0.000953674, 0.000009};

public:
    job_base(int source, int destination, int model, double inputSize, double generated_time, std::vector<int> cut_position);
    int getCompDelta() const;
    void setCompDelta(int compDelta = 0);
    int getCurrentSubtaskIndex() const;
    void setCurrentSubtaskIndex(int currentSubtaskIndex = 0);
    int* getPath() const;
    void setPath(int *path);
    int getIndex() const;
    void setIndex(int index = -1);
    int getSource() const;
    int getDestination() const;

    std::vector<subtask_base*> getSubtasks();

    double getInputSize() const {
        return inputSize;
    }

    ~job_base(){
        for(int i = 0; i<subtasks.size(); i++){
            delete subtasks[i];
        }

    }

    double getProcessFinishTime() const {
        return processFinishTime;
    }

    void setProcessFinishTime(double processFinishTime) {
        this->processFinishTime = processFinishTime;
    }

    double getProcessStartTime() const {
        return processStartTime;
    }

    void setProcessStartTime(double processStartTime) {
        this->processStartTime = processStartTime;
    }

    double getTotalComp() const {
        return totalComp;
    }

    double getJobStartTime() const {
        return jobStartTime;
    }

    void setJobStartTime(double jobStartTime) {
        this->jobStartTime = jobStartTime;
    }

    double getGeneratedTime() const {
        return generated_time;
    }
};

class output_base{
protected:
    int order;
    int index;
    double linkDemand;
    int destination;
    std::queue<int>* path;
    bool isLast = false;
    double jobStartTime;
    double processStartTime;
    double processFinishTime;
    double totalComp;
    double generatedTime;

public:
    output_base(int index, double linkDemand, int destination, double processStartTime, double totalComp, double generatedTime);
    int getDestination() const {
        return destination;
    }



    void setDestination(int destination) {
        this->destination = destination;
    }

    int getIndex() const {
        return index;
    }

    void setIndex(int index) {
        this->index = index;
    }

    double getLinkDemand() const {
        return linkDemand;
    }

    void setLinkDemand(double linkDemand) {
        this->linkDemand = linkDemand;
    }

    std::queue<int>* getPath(){
        return path;
    }

    void setPath(std::queue<int> *path) {
        this->path = path;
    }

    bool isIsLast() const {
        return isLast;
    }

    void setIsLast(bool isLast = false) {
        this->isLast = isLast;
    }

    ~output_base(){
        delete path;
    }

    double getProcessFinishTime() const {
        return processFinishTime;
    }

    void setProcessFinishTime(double processFinishTime) {
        this->processFinishTime = processFinishTime;
    }

    double getProcessStartTime() const {
        return processStartTime;
    }

    void setProcessStartTime(double processStartTime) {
        this->processStartTime = processStartTime;
    }

    double getTotalComp() const {
        return totalComp;
    }

    void setTotalComp(double totalComp) {
        this->totalComp = totalComp;
    }

    double getJobStartTime() const {
        return jobStartTime;
    }

    void setJobStartTime(double jobStartTime) {
        this->jobStartTime = jobStartTime;
    }

    double getGeneratedTime() const {
        return generatedTime;
    }
};


#endif /* JOB_BASE_H_ */
