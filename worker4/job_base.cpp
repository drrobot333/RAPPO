/*
 * subtask.cpp
 *
 *  Created on: 2022. 7. 13.
 *      Author: user
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "myFunction.h"
#include "job_base.h"



subtask_base::subtask_base(double compDemand, double outputLinkDemand, int order, double generatedTime){
    this->compDemand = compDemand;
    this->outputLinkDemand = outputLinkDemand;
    this->order = order;
    this->generatedTime = generatedTime;
}

job_base::job_base(int source, int destination, int model, double inputSize, double generated_time, std::vector<int> cut_position){
    this->source = source;
    this->destination = destination;
    this->inputSize = inputSize;
    this->generated_time = generated_time;
    model = cut_position.size();

    for(int i = 0; i < model; i++){
        double compDemand = 0;
        int startComp = i > 0 ? cut_position[i-1] : 0;
        for(int j = startComp; j<cut_position[i];j++){
            compDemand += this->taskCompDemand[j];
        }

        double outputLinkDemand = this->taskLinkDemand[cut_position[i] - 1];
//        int startLink = i > 0 ? cut_position[i-1] : 0;
//        for(int j = 0; j<cut_position[i];j++){
//            outputLinkDemand += this->taskLinkDemand[j];
//        }

        subtask_base* sub = new subtask_base(compDemand, outputLinkDemand, i, generated_time);
        if(i == model-1){
            sub->setIsLast(true);
        }

        this->totalComp += (compDemand + outputLinkDemand);
        subtasks.push_back(sub);
    }
    for(int i = 0; i < model; i++){
        subtasks[i]->setTotalComp(this->totalComp);
    }
}

int job_base::getCompDelta() const {
    return compDelta;
}

void job_base::setCompDelta(int compDelta) {
    this->compDelta = compDelta;
}

int job_base::getCurrentSubtaskIndex() const {
    return currentSubtaskIndex;
}

void job_base::setCurrentSubtaskIndex(int currentSubtaskIndex) {
    this->currentSubtaskIndex = currentSubtaskIndex;
}

int* job_base::getPath() const {
    return path;
}

void job_base::setPath(int *path) {
    this->path = path;
}

int job_base::getIndex() const {
    return index;
}

void job_base::setIndex(int index) {
    this->index = index;
}

int job_base::getSource() const {
    return source;
}

int job_base::getDestination() const {
    return destination;
}

std::vector<subtask_base*> job_base::getSubtasks() {
    return subtasks;
}

output_base::output_base(int index, double linkDemand, int destination, double processStartTime, double totalComp, double generatedTime){
    this->index = index;
    this->linkDemand = linkDemand;
    this->destination = destination;
    this->processStartTime = processStartTime;
    this->totalComp = totalComp;
    this->generatedTime = generatedTime;
}
