/*
 * iot.cpp
 *
 *  Created on: 2022. 7. 11.
 *      Author: user
 */

#include "iot.h"

int returnModuleNumber(std::string gateString){
    int i = 0;

    std::string moduleNumber = "";

    for(i = 0; i<gateString.length(); i++){
        if(gateString[i] == '['){
            i += 1;
            break;
        }

    }

    if(i == gateString.length())
        return -1;

    for(int j = i; j<gateString.length(); j++){
        if(gateString[j] == ']')
            break;
        moduleNumber += gateString[j];
    }


    return std::stoi(moduleNumber);
}

/**
 * Let's make it more interesting by using several (n) `tic' modules,
 * and connecting every module to every other. For now, let's keep it
 * simple what they do: module 0 generates a message, and the others
 * keep tossing it around in random directions until it arrives at
 * module 3.
 */



void iot::initialize()
{
    // job 생성 루프용 메세지
    makeJobLoop = new cMessage("makeJobLoop");
    scheduleAt(simTime() + 1.0, makeJobLoop);



    //dataplane 접근용
    for(cModule::SubmoduleIterator it(getParentModule()); !it.end(); it++){
        cModule *submodule = *it;
        if(strcmp(submodule->getFullName(), "dataplane") == 0){
            this->dataplaneModule = check_and_cast<dataplane *>(submodule);
            break;
        }
    }


    // parameter 가져오기용 부모 모듈 로드
    cModule* network = getParentModule();
    this->nodeNum = network->par("nodeNum").intValue();
    this->jobGenerateRate = network->par("jobGenerateRate").intValue();
    this->modelNum = network->par("modelNum").intValue();
    this->linkCapacity = network->par("linkCapacity").intValue();
    this->nodeCapacity = network->par("nodeCapacity").intValue();
    this->availableJobNum = network->par("availableJobNum").intValue(); // 노드가 처리할 수 있는 job의 양
    this->episode_length = network->par("episode_length").intValue();
    this->reset_network = network->par("reset_network").boolValue();
    this->low = network->par("low").doubleValue();
    this->high = network->par("high").doubleValue();
    //std::vector<std::string> timesStr = cStringTokenizer(par("times")).asVector();
    std::vector<std::string> tmp = cStringTokenizer(network->par("cut_array")).asVector();
    for(int i = 0; i <  tmp.size(); i++){
        this->cut_position.push_back(std::stoi(tmp[i]));
    }

    // 상수
    this->myIndex = getIndex();
    cMessage* ready = new cMessage("ready");
    ready->addPar("nodeIndex");
    ready->par("nodeIndex").setLongValue(this->myIndex);
    send(ready, "outDataplane");

    this->compQueue = new std::queue<subtask_base*>[availableJobNum];
    this->jobRemainState = new double[this->availableJobNum * this->modelNum];


    // 링크 정보 초기화
    for(cModule::GateIterator it(this); !it.end(); ++it){

        cGate *gate = *it;
        std::string gateString = gate->str();
        int moduleNumber = returnModuleNumber(gateString); // 인접 노드 번호

        //// EV << getIndex() << ":" << i << "=" << moduleNumber << " ";
        if(dataplaneModule->adjacentMatrix[myIndex][moduleNumber] == nullptr && myIndex != moduleNumber){ // 내 엣지가 결정되지 않았을 때, self loop가 아닐 때
            link* newLink = new link(myIndex, moduleNumber, linkCapacity);
            dataplaneModule->adjacentMatrix[myIndex][moduleNumber] = newLink;
            dataplaneModule->adjacentMatrix[moduleNumber][myIndex] = newLink; // 반대쪽도 지정해줌

            dataplaneModule->adjacentList.push_back(newLink);
        }
    }

    if(this->myIndex % 3 == 0){
        this->nodeCapacity = this->low;
        this->is_source = false;
    }
    else{
        this->nodeCapacity = this->high;
        this->is_source = true;
    }



}

void iot::makeJob(){
//    if(myIndex != 0)
//        return;
    int source = getIndex();
    int destination = source;
    int model = modelNum;
    //int model = modelNum;

    // 새로운 job이 만들어짐
    job_base* newJob = new job_base(source, destination, model, 1.236696, simTime().dbl(), this->cut_position);

    // 데이터 플레인에 새로운 job 추가
    this->dataplaneModule->pushNewJob(newJob);

    jobQueue.push(newJob);

    //compQueue[0].push(newJob);


    // EV << source << "  " << destination << "  " << model << "  ";
}

void iot::handleMessage(cMessage *msg){
    if(this->reset_network){
        if((((int)msg->getSendingTime().dbl() / this->episode_length) != ((int)simTime().dbl() / this->episode_length) and strcmp(msg->getFullName(), "reset") != 0)){
            if(strcmp(msg->getFullName(), "makeJobLoop") == 0){
                scheduleAt(simTime() + 1.0, makeJobLoop);
                return;
            }

            else{
                if(strcmp(msg->getFullName(), "output") == 0 and (int)msg->par("send_time").doubleValue() / this->episode_length == ((int)simTime().dbl() / this->episode_length)){
                }
                else{
                    delete msg;
                    msg = NULL;
                    return;
                }

            }
        }
    }


    if(strcmp(msg->getFullName(), "makeJobLoop") == 0){
        if(randint(1, 100) <= this->jobGenerateRate){
            if(!is_source) // task device면 무조건 job 생성
                this->makeJob();
        }
        // 다시 자신에게 보냄(루프)
        scheduleAt(simTime() + 1.0, makeJobLoop);
        //makeJob();

    }
    else if(strcmp(msg->getFullName(), "processFinishMsg") == 0){ // compute 끝났으므로 라우팅해주고 다시 job process하면 됨.

        delete msg;
        msg = NULL;
        // EV << " process finish \n";

        subtask_base* finishedSubtask = realCompQueue.front();
        realCompQueue.pop();
        isProcessing = false;
        processStartTime = -1;

        //output routing 해줘야함.
        output_base* output = new output_base(this->currentProcessing, finishedSubtask->getOutputLinkDemand(), finishedSubtask->getDestination(), finishedSubtask->getProcessStartTime(), finishedSubtask->getTotalComp(), finishedSubtask->getGeneratedTime());
        output->setJobStartTime(finishedSubtask->getJobStartTime());

        if(finishedSubtask->isIsLast()){ // 마지막 subtask면 output에 last flag on
            output->setIsLast(true);
            // EV << " last task \n";
        }


        routing(output);

        std::queue<int>* outputPath = output->getPath();
        int nextNode = outputPath->front();
        outputPath->pop();

        if(nextNode == this->myIndex){ // 내가 프로세싱 하는거면
            EV<<"abcdefghi";
            Output* outputMsg = new Output("output");
            outputMsg->addPar("send_time").setDoubleValue(simTime().dbl());
            outputMsg->setOutput(output);
//            outputMsg->addPar("isSec");
//            outputMsg->par("isSec").setBoolValue(true);
            scheduleAt(simTime(), outputMsg);
        }
        else{
            sendJob(output, nextNode);
        }


        this->currentProcessing = -1;
        this->currentProcessingOrder = -1;
        //프로세싱 해야되는 job남았으면 processJob()
        if(!isRealCompQueueEmpty())
            processJob();
    }

    else if(strcmp(msg->getFullName(), "schedulingCompleteMsg") == 0){ // source인 모듈이 받는 메세지, 시작 노드로 라우팅 해주면 됨

        EV << "good job";
        job_base* inputJob = jobQueue.front();

        jobQueue.pop();

        int startNode = msg->par("startNode");
        int index = msg->par("index");
        inputJob->setProcessStartTime(simTime().dbl());
        inputJob->setJobStartTime(inputJob->getGeneratedTime());

        std::vector<subtask_base*> subtasks = inputJob->getSubtasks();

        for(int i = 0; i<subtasks.size(); i++){
            subtasks[i]->setJobStartTime(simTime().dbl());
        }

        delete msg;
        msg = NULL;
        output_base* output = new output_base(index, inputJob->getInputSize(), startNode, simTime().dbl(), inputJob->getTotalComp(), inputJob->getGeneratedTime());

        routing(output);

        std::queue<int>* outputPath = output->getPath();
        int nextNode = outputPath->front();
        outputPath->pop();

        if(nextNode == this->myIndex){ // 내가 프로세싱 하는거면
            EV<<"abcdefghi";
            Output* outputMsg = new Output("output");
            outputMsg->addPar("send_time").setDoubleValue(simTime().dbl());
            outputMsg->setOutput(output);
            scheduleAt(simTime(), outputMsg);
        }
        else{
            sendJob(output, nextNode);
        }

        // 시작 노드로 라우팅
        // input data지만 output 클래스 이용하면 될 듯
        // // EV << simTime() << "im" << getFullName();
    }

    else if(strcmp(msg->getFullName(), "subtask") == 0){ // 내가 처리해야하는 subtask가 들어옴. 큐에 넣으면 됨.
        // EV << "asfksdfsaklfj";
        Subtask* subtaskMsg = check_and_cast<Subtask *>(msg);

        const subtask_base* subtask_cost = subtaskMsg->getSubtask();
        subtask_base* subtask = const_cast<subtask_base*>(subtask_cost);

        delete msg;
        msg = NULL;

        int subtaskIndex = subtask->getIndex();
        compQueue[subtaskIndex].push(subtask);

    }

    else if(strcmp(msg->getFullName(), "output") == 0){ // 내가 destination이면 다음 것 실행하면됨, 아니면 path대로 보내면 됨


        if(this->reset_network){
            if(((int)msg->par("send_time").doubleValue() / this->episode_length != (int)simTime().dbl() / this->episode_length)){
                return;
            }
        }



        Output* outputMsg = check_and_cast<Output *>(msg);

        output_base* output = const_cast<output_base*>(outputMsg->getOutput());

        delete msg;
        msg = NULL;

        if(output->getDestination() == this->myIndex){ // 내가 최종 목적지면
            // EV << "im dst " << "\n";
            int outputIndex = output->getIndex();

            if(output->isIsLast()){ // 최종 출력이면
                // EV << "dst arrived, remove job from jobQueue" << "\n";
                double latency = simTime().dbl() - output->getGeneratedTime();
                double totalComp = output->getTotalComp();

                this->jobLatency += latency;
                this->jobCompleteNum += 1;
                this->reward += totalComp / latency;
                // EV << "totalComp  " << totalComp << "  latency  " << latency;


                dataplaneModule->activatedJobQueue[outputIndex] = nullptr;
                delete output;
                output = NULL;

                cMessage* completeMsg = new cMessage("complete");

                totalComp = totalComp - (latency * 10);

                if(totalComp < 0)
                    totalComp = 0;


                completeMsg->addPar("totalComp").setDoubleValue(totalComp);

                send(completeMsg, "outDataplane");

                return;
            }

            subtask_base* subtaskToCompute = compQueue[outputIndex].front();
            compQueue[outputIndex].pop();
            realCompQueue.push(subtaskToCompute);
            delete output;
            output = NULL;
            if(!isProcessing) // 실행하는 job이 없으면 processJob 실행(안하면 시뮬레이션 멈춤), 있으면 그냥 냅둠(processJob이 job이 남아잇으면 자신을 다시 호출하기 때문)
                processJob();
        }
        else{ // 아니면(다음 노드로 라우팅)

            int nextNode = output->getPath()->front();
            // EV << "next node routing : " << nextNode << "\n";
            output->getPath()->pop();
            sendJob(output, nextNode);
        }
    }

    else if(strcmp(msg->getFullName(), "info") == 0){
        computeJobRemain();
        State* stateMsg = new State("info");

        state_base* state = new state_base(jobRemainState, jobRemain, jobProgress, this->currentProcessing, myIndex, this->currentProcessingOrder, this->jobLatency, this->jobCompleteNum, this->reward);
        // nodeState, 잔여량, 진행량, job index, node index
        this->jobLatency = 0;
        this->reward = 0;
        this->jobCompleteNum = 0;

        stateMsg->setState(state);

        delete msg;
        msg = NULL;

        // EV << jobRemain;

        send(stateMsg, "outDataplane");

    }
    else if(strcmp(msg->getFullName(), "reward") == 0){
        delete msg;
        msg = NULL;
        cMessage* reward = new cMessage("reward");
        //reward->addPar("reward").setDoubleValue(this->jobProgress);
        reward->addPar("latency").setBoolValue(this->jobLatency);
        reward->addPar("num").setLongValue(this->jobCompleteNum);
        reward->addPar("index").setLongValue(myIndex);

        send(reward, "outDataplane");
    }

    else if(strcmp(msg->getFullName(), "reset") == 0){
        if(!this->reset_network)
            return;

        delete msg;
        msg = NULL;
        reset_iot();
    }
}

void iot::reset_iot(){
    for(int i = 0; i<this->availableJobNum; i++){
        while(this->compQueue[i].size() > 0){
            this->compQueue[i].pop();
        }
    }

    //this->compQueue = new std::queue<subtask_base*>[this->availableJobNum];
    //this->jobRemainState = new double[this->availableJobNum * this->modelNum];

    this->jobRemain = 0;
    this->jobProgress = 0;
    this->jobLatency = 0;
    this->reward = 0;
    this->jobCompleteNum = 0;

    // std::queue<job_base*> jobQueue;
    // std::queue<subtask_base*> realCompQueue;
    // std::queue<double *> processedJobQueue; // returnJobProgress에서만 관리하는 큐

    while(this->jobQueue.size() > 0){
        this->jobQueue.pop();
    }

    while(this->realCompQueue.size() > 0){
        this->realCompQueue.pop();
    }

    while(this->processedJobQueue.size() > 0){
        this->processedJobQueue.pop();
    }

    this->currentProcessing = -1;
    this->currentProcessingOrder = -1;


    this->sendStartTime;
    this->processStartTime = -1;
    this->latestComputeJobTime = -1;

    this->isProcessing = false;

    episode_start_time = simTime();




}

// compQueue에서 pop한 job 진행시키는 함수(job/compPower로 나눈 시간 후에 scheduleAt 호출)
void iot::processJob(){
    isProcessing = true;
    processStartTime = simTime();

    subtask_base* subtask = realCompQueue.front();


    int index = subtask->getIndex();
    int order = subtask->getOrder();
    subtask->setProcessStartTime(processStartTime.dbl());

    this->currentProcessing = index;
    this->currentProcessingOrder = order;

    cMessage* processFinishMsg = new cMessage("processFinishMsg");
    double subtaskCompDemand = subtask->getCompDemand();

    // EV << "this->currentProcessing : " << this->currentProcessing << "\n";
    // EV << "subtaskCompDemand : " << subtaskCompDemand << "\n";
    // EV << "this->nodeCapacity : " << this->nodeCapacity << "\n";
    // EV << "subtask->isIsLast() : " << subtask->isIsLast() << "\n";


    double finishTime = (double)subtaskCompDemand/(double)(this->nodeCapacity);
    // job 시작 시간, job 종료 시간, job 컴퓨팅 요구량
    double* subtaskInfo = new double[6]{processStartTime.dbl(), processStartTime.dbl() + finishTime, (double)subtaskCompDemand, (double)subtask->getIndex(), (double)subtask->getOrder(), (double)subtask->getCompDemand()};
    processedJobQueue.push(subtaskInfo);

    // EV << finishTime;

    scheduleAt(simTime() + finishTime, processFinishMsg);
}

void iot::computeJobRemain(){
    if(std::isnan(1/(this->latestComputeJobTime - simTime())))
        return;
    simtime_t currentTime = simTime();
    this->latestComputeJobTime = simTime();

    int s = processedJobQueue.size();
    double* jobRemain = new double[this->availableJobNum * this->modelNum]{0};
    double jobProgress = 0;

    for(int i = 0; i<s; i++){ // 어찌됐든 프로세싱을 시작한 job들에 대해
        double *subtaskInfo = processedJobQueue.front();
        double timeChangeRatio = (currentTime.dbl() - subtaskInfo[0])/(subtaskInfo[1] - subtaskInfo[0]);
        if(currentTime.dbl() > subtaskInfo[1] || std::isnan(timeChangeRatio)){ // 지금이 job 종료 시간 이후면 == 이미 끝난 job이라면 또는 분모가 0에 가깝다면(거의 종료 시간임)
            jobProgress += subtaskInfo[2] / subtaskInfo[5]; // job 컴퓨팅 요구량 그대로 증가
            processedJobQueue.pop(); // job 삭제
        }
        else{ // 지금이 job 종료 시간 이전이면 == 아직 컴퓨팅 하고있는 job이라면
            double timeChangeRatio = (currentTime.dbl() - subtaskInfo[0])/(subtaskInfo[1] - subtaskInfo[0]); // (현재시간 - 시작시간)/(종료시간 - 시작시간) == 시간 변화율
            double thisJobProgress = timeChangeRatio * subtaskInfo[2]; // 시간 변화율 * 남은 compute 요구량 == compute 변화량
            // EV << myIndex << " : " <<thisJobProgress << "\n";
            double thisComputeRemain = subtaskInfo[2] - thisJobProgress;

            int index = (int)subtaskInfo[3];
            int order = (int)subtaskInfo[4];

            jobProgress += thisJobProgress / subtaskInfo[5];

            subtaskInfo[0] = currentTime.dbl(); // 시작 시간을 지금 시간으로 바꿈
            subtaskInfo[2] = thisComputeRemain; // 남은 컴퓨팅량을 새롭게 조정

            jobRemain[this->modelNum * index + order] = thisComputeRemain / 100.0;
        }
    }

    // 대기하고있는 모든 job 더함
    for(int i = 0; i < availableJobNum; i++){
        int s = compQueue[i].size();
        for(int j = 0; j < s; j++){
            subtask_base* temp = compQueue[i].front();
            int index = temp->getIndex();
            int order = temp->getOrder();
            double compDemand = temp->getCompDemand();

            jobRemain[this->modelNum * index + order] = compDemand / 100.0;

            compQueue[i].pop();
            compQueue[i].push(temp);
        }
    }

    this->jobRemainState = jobRemain;
    this->jobRemain = returnJobRemain();
    this->jobProgress = jobProgress;
}

// 엣지로 subtask 보내는 함수
void iot::sendJob(output_base* output, int nodeName){
    sendStartTime = simTime();
    int gateNum = 0;
    // EV << "nextNode : " << nodeName << "\n";
    for(cModule::GateIterator it(this); !it.end(); ++it){
        cGate *gate = *it;
        // EV << gate->str() << " ";
    }
    for(cModule::GateIterator it(this); !it.end(); ++it){

        cGate *gate = *it;
        std::string gateString = gate->str();
        int moduleNumber = returnModuleNumber(gateString);



        if(moduleNumber == nodeName)
            break;

        gateNum += 1;
    }

    // EV << "gateNum : " << gateNum << "\n";
    Output* outputMsg = new Output("output");
    outputMsg->setOutput(output);

    // 라우팅 전 link 정리 다 해두기
    std::vector<link*> adjacentList = dataplaneModule->adjacentList;
    link*** adjacentMatrix = dataplaneModule->adjacentMatrix;

    for(int i = 0; i<adjacentList.size(); i++){
        adjacentList[i]->transfer(simTime());
    }

    link* l = dataplaneModule->adjacentMatrix[myIndex][nodeName];
    double transferTime = l->transferCompleteTime(output->getLinkDemand(), sendStartTime);

    // EV << output->getIndex() << "transfer time : " << transferTime << "\n";

    outputMsg->addPar("send_time").setDoubleValue(simTime().dbl());

    sendDelayed(outputMsg, transferTime, "out", gateNum);
}

void iot::routing(output_base* output){ // 다익스트라, path만 짜줌, send안해줌
    // 라우팅 전 link 정리 다 해두기

    // EV << "routing" << endl;

    std::vector<link*> adjacentList = dataplaneModule->adjacentList;
    link*** adjacentMatrix = dataplaneModule->adjacentMatrix;

    for(int i = 0; i<adjacentList.size(); i++){
        adjacentList[i]->transfer(simTime());
    }
    // 됨

    int start = myIndex;
    int destination = output->getDestination();

    if(start == destination){ // 자기 자신에게 보내는거면 여기서 리턴. 안그러면 아래에서 index 접근 에러남.
        std::queue<int>* path = new std::queue<int>;
        path->push(start);
        output->setPath(path);
        return;
    }

    double* cost = new double[nodeNum];
    for(int i = 0; i<nodeNum; i++){
        cost[i] = 10000000;
    }

    // 됨

    for(int i = 0; i<nodeNum; i++){
        link* l = adjacentMatrix[start][i];
        if(l != nullptr){
            cost[i] = l->getWaiting();
        }
    }


    //됨

    bool* visited = new bool[nodeNum];
    int* pre = new int[nodeNum]; // 해당 노드에 가기 전 노드

    for(int i = 0; i<nodeNum; i++){
        visited[i] = false;
        pre[i] = -1;
    }

    //됨
//

    // // EV << isVisitedAll(visited);
    // 됨

    visited[start] = true;
    cost[start] = 0;

    while(!isVisitedAll(visited)){
        // 가장 작은 cost를 가지는 노드 찾음
        int minIndex = -1;
        double minCost = 100000000;
        for(int i = 0; i<nodeNum; i++){ // 방문하지 않은 노드 중에서 cost가장 작은 노드 찾는 과정
            if(cost[i] < minCost && !visited[i]){
                minCost = cost[i];
                minIndex = i;
            }
        }

        visited[minIndex] = true;



        for(int j = 0; j<nodeNum; j++){
            if(!visited[j] && adjacentMatrix[minIndex][j] != nullptr){ // 방문하지 않았고, cost가장 작은 노드와 이어져있는 노드라면
                double transferTime = ((double)adjacentMatrix[minIndex][j]->getWaiting() + (double)output->getLinkDemand())/(double)adjacentMatrix[minIndex][j]->getLinkCapacity();
                if(cost[minIndex] + transferTime <= cost[j]){
                    cost[j] = cost[minIndex] + adjacentMatrix[minIndex][j]->getWaiting();
                    pre[j] = minIndex;
                }
            }
        }
    }

    int cur = destination;
    std::stack<int> reversePath;
    while(cur != -1){
        reversePath.push(cur);
        cur = pre[cur];
    }

//    reversePath.push(start);

    int s = reversePath.size();
    std::queue<int>* path = new std::queue<int>;
    for(int i = 0; i<s; i++){
        path->push(reversePath.top());
        reversePath.pop();
    }

    output->setPath(path);
}

bool iot::isVisitedAll(bool* visited){
    for(int i = 0; i<nodeNum;i++){
        if(!visited[i])
            return false;
    }
    return true;
}

double iot::returnJobRemain(){
    int s = this->availableJobNum * this->modelNum;
    double result = 0;
    for(int i = 0; i<s; i++){
        result += this->jobRemainState[i];
    }
    return result;

}

bool iot::isRealCompQueueEmpty(){
    if(realCompQueue.empty())
        return true;
    else
        return false;
}

void iot::refreshDisplay() const{
    char buf[100];

    sprintf(buf, "job remain : %.3f, job progress : %.3f", this->jobRemain, this->jobProgress);
    getDisplayString().setTagArg("t", 0, buf);
}

iot::~iot()
{
    cancelAndDelete(makeJobLoop);
}


Define_Module(iot);
