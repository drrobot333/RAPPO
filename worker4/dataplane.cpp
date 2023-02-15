/*
 * dataplane.cpp
 *
 *  Created on: 2022. 7. 14.
 *      Author: user
 */

#include "dataplane.h"
#define BUF_SIZE 200000



void dataplane::initialize(){
    LPTSTR lpvMessage = TEXT("Default message from client.");
    TCHAR  chBuf[BUF_SIZE];
    BOOL   fSuccess = false;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\worker4");

    this->latency = 0;
    this->completeNum = 0;
    this->reward = 0;

    srand(100);

    test = new cMessage("test");
    sendStateMsg = new cMessage("sendState");
    checkTimeMsg = new cMessage("checkTime");

    cModule* network = getParentModule();
    this->nodeNum = network->par("nodeNum").intValue();
    this->jobWaitingQueueLength = network->par("jobWaitingQueueLength").intValue();
    this->availableJobNum = network->par("availableJobNum").intValue();
    this->modelNum = network->par("modelNum").intValue();
    this->episode_length = network->par("episode_length").intValue();
    this->reset_network = network->par("reset_network").boolValue();
    this->job_generate_rate = network->par("jobGenerateRate").intValue();
    this->low = network->par("low").doubleValue();
    this->high = network->par("high").doubleValue();


    adjacentMatrix = new link**[nodeNum];
    for(int i = 0; i<this->nodeNum; i++){
        adjacentMatrix[i] = new link*[nodeNum];
    }

    for(int i = 0; i<this->nodeNum; i++){
        for(int j = 0; j<this->nodeNum; j++){
            adjacentMatrix[i][j] = nullptr;
        }
    }

    activatedJobQueue = new job_base*[this->availableJobNum]{nullptr};
    // activatedJobQueue = new job_base*[this->availableJobNum]{nullptr}; -> nullptr로 초기화 해줘야함. 안그러면 job activate 안함.

    nodeState = new double*[this->nodeNum];
    jobRemain = new double[this->nodeNum];
    nodeProcessingState = new int*[this->nodeNum];

    this->isActioned = false;


    scheduleAt(simTime() + this->time_step_interval, test);
    scheduleAt(simTime() + this->episode_length, checkTimeMsg);

    this->hPipe = CreateFile(
        lpszPipename,   // pipe name
        GENERIC_READ |  // read and write access
        GENERIC_WRITE,
        0,              // no sharing
        NULL,           // default security attributes
        OPEN_EXISTING,  // opens existing pipe
        0,              // default attributes
        NULL
    );

}

DWORD dataplane::sendPythonMessage(std::string msg){
    DWORD cbWritten = 0;
    LPTSTR szMsg = (LPTSTR)msg.c_str();
    DWORD size = msg.size();

    bool fSuccess = WriteFile(
        hPipe,                  // pipe handle
        szMsg,                  // message
        size,                   // message length
        &cbWritten,             // bytes written
        NULL);

    return cbWritten;
}

std::string dataplane::getPythonMessage(){
    TCHAR pchRequest[BUF_SIZE];
    DWORD cbBytesRead = 0;

    bool fSuccess = ReadFile(
        hPipe,        // handle to pipe
        pchRequest,    // buffer to receive data
        BUF_SIZE*sizeof(TCHAR), // size of buffer
        &cbBytesRead, // number of bytes read
        NULL);        // not overlapped I/O

    std::string response((char *)pchRequest, cbBytesRead);

    return response;
}



int dataplane::activateNewJob(){
    // EV << "jobWaitingQueue.size() : " << jobWaitingQueue.size() << "\n";
    if(jobWaitingQueue.size() <= 0)
        return 0;

    this->activatedJobNum = 0;
    for(int i = 0; i < availableJobNum; i++){
        if(activatedJobQueue[i] != nullptr){
                this->activatedJobNum += 1;
        }
    }

    for(int i = 0; i < availableJobNum; i++){
        if(activatedJobQueue[i] == nullptr){
            job_base* newJob = jobWaitingQueue.front();


            assignIndex(newJob, i);
            int startNode = scheduleJob(newJob);

            if(startNode == -1){
                return -1; // 그 timestep에서 스케쥴링 안하는 것 이므로 리턴
            }

            activatedJobQueue[i] = newJob; // void 액션인지 확인하고 할당해야함.

            // 소스 모듈에게 스케쥴링 됐으니 스케쥴링 대로 전송하라고 보냄
            int sourceModule = newJob->getSource();
            // EV << sourceModule;
            // 해당되는 모듈로 보낸 상태
            cMessage* schedulingCompleteMsg = new cMessage("schedulingCompleteMsg");
            schedulingCompleteMsg->addPar("startNode");
            schedulingCompleteMsg->par("startNode").setLongValue(startNode);

            schedulingCompleteMsg->addPar("index");
            schedulingCompleteMsg->par("index").setLongValue(i);

            send(schedulingCompleteMsg, "out", sourceModule);
            // // EV << newJob->getSource() << "scheduling complete";


            jobWaitingQueue.pop();
            break;
        }
    }

    return 0;
}

void dataplane::resetNetwork(){
    this->latency = 0;
    this->completeNum = 0;
    this->reward = 0;
    this->jobProgress = 0;
    this->totalCompleteNum = 0;
    this->episodeReward = 0;
    this->activatedJobNum = 0;
    this->isWaiting = false;
    this->isActioned = false;
    this->episode_num = this->episode_num + 1;

    latestSendTime = simTime();

    for(int i = 0; i<this->nodeNum; i++){
        for(int j = 0; j<this->nodeNum; j++){
            if(adjacentMatrix[i][j] != nullptr){
                adjacentMatrix[i][j]->setWaiting(0);
                adjacentMatrix[i][j]->setLinkStartTime(-1);
            }
        }
    }

    for(int i = 0; i<this->availableJobNum; i++){
        activatedJobQueue[i] = nullptr;
    }

    for(int i = 0 ; i<nodeNum ; i++){
        if(nodeState[i] != NULL and nodeProcessingState[i] != NULL){
            delete[] nodeState[i];
            delete[] nodeProcessingState[i];
        }
        jobRemain[i] = 0.0;

    }

    int job_waiting_queue_size = this->jobWaitingQueue.size();
    int backlog_size = this->backlog.size();

    while(this->jobWaitingQueue.size() > 0){
        this->jobWaitingQueue.pop();
    }

    while(this->backlog.size() > 0){
            this->backlog.pop();
    }

    for (int i = 0; i < gateSize("out"); i++) {
        cMessage* info = new cMessage("reset");
        send(info, "out", i);
    }

    this->previous_state = "";
    this->current_state = "";



    cancelEvent(test);
    scheduleAt(simTime() + 0.5, test);

}

std::string dataplane::getStateMsg(job_base* newJob, bool isAction){
    double** jobWaiting = new double*[this->jobWaitingQueueLength];
    for(int i = 0; i<this->jobWaitingQueueLength; i++){
        jobWaiting[i] = new double[this->nodeNum + this->modelNum]{0};
    }

    int s = jobWaitingQueue.size();
    for(int i = 0 ; i<s; i++){
        job_base* temp = jobWaitingQueue.front();
        int source = temp->getSource();
        int destination = temp->getDestination();
        std::vector<subtask_base*> subtasks = temp->getSubtasks();

        jobWaiting[i][source] = -1;
        jobWaiting[i][destination] = 1;

        for(int j = 0; j<subtasks.size(); j++){
            double compDemand = (subtasks[j]->getCompDemand());
            jobWaiting[i][this->nodeNum + j] = compDemand;
        }
        jobWaitingQueue.pop();
        jobWaitingQueue.push(temp);

    }

    std::string nodeStateMsg = "[";
    std::string nodeProcessingMsg = "[";
    std::string linkWaitingMsg = "[";
    std::string jobWaitingMsg = "[";
    std::string activatedJobListMsg = "[";
    std::string jobRemainMsg = "[";
    std::string networkStateMsg = "";


    for(int i = 0; i<nodeNum; i++){
        nodeStateMsg += "[";
        for(int j = 0; j<(this->availableJobNum * this->modelNum); j++){
            nodeStateMsg += std::to_string(nodeState[i][j]);
            nodeStateMsg += ", ";
        }
        nodeStateMsg += "],";
    }
    nodeStateMsg += "]";

    for(int i = 0; i<nodeNum; i++){
        nodeProcessingMsg += "[";
        for(int j = 0; j<(this->availableJobNum * this->modelNum); j++){
            nodeProcessingMsg += std::to_string(nodeProcessingState[i][j]);
            nodeProcessingMsg += ", ";
        }
        nodeProcessingMsg += "],";
    }
    nodeProcessingMsg += "]";

    for(int i = 0 ; i<adjacentList.size(); i++){
        linkWaitingMsg += "[" + std::to_string(adjacentList[i]->getWaiting()/100) + "]";
        linkWaitingMsg += ", ";
        linkWaitingMsg += "[" + std::to_string(adjacentList[i]->getWaiting()/100) + "]";
        linkWaitingMsg += ", ";
    }
    linkWaitingMsg += "]";



    for(int i = 0; i < this->jobWaitingQueueLength; i++){
        jobWaitingMsg += "[";
        for(int j = 0; j<this->nodeNum + this->modelNum; j++){
            if(j<this->nodeNum)
                jobWaitingMsg += std::to_string(jobWaiting[i][j]);
            else
                jobWaitingMsg += std::to_string((double)jobWaiting[i][j]/ 100);
            jobWaitingMsg += ", ";
        }
        jobWaitingMsg += "],";
    }
    jobWaitingMsg += "]";

    for(int i = 0; i<this->availableJobNum; i++){
        if(this->activatedJobQueue[i] != nullptr){
            activatedJobListMsg += "1,";
        }
        else{
            activatedJobListMsg += "0,";
        }
    }
    activatedJobListMsg += "]";

    for(int i = 0; i<this->nodeNum; i++){
        jobRemainMsg += std::to_string(this->jobRemain[i]) + ", ";
        }
    jobRemainMsg += "]";

    //nodeStateMsg = "c" + nodeStateMsg + "!";

    std::string nodeStateEntry = returnEntryString(std::string("nodeState"), nodeStateMsg); // 노드 job waiting
    std::string nodeProcessingEntry = returnEntryString(std::string("nodeProcessing"), nodeProcessingMsg); // 노드 job processing
    std::string linkWaitingEntry = returnEntryString(std::string("linkWaiting"), linkWaitingMsg); // 링크 waiting
    std::string jobWaitingEntry = returnEntryString(std::string("jobWaiting"), jobWaitingMsg); // jobWaitingQueue
    std::string activatedJobListEntry = returnEntryString(std::string("activatedJobList"), activatedJobListMsg); // activatedJobQueue
    std::string jobRemainEntry = returnEntryString(std::string("jobRemain"), jobRemainMsg);

    std::string jobIndexEntry = returnEntryString(std::string("jobIndex"), std::to_string(-1));
    if(newJob != nullptr)
        jobIndexEntry = returnEntryString(std::string("jobIndex"), std::to_string(newJob->getIndex()));

    std::string isActionEntry = returnEntryString(std::string("isAction"), std::to_string(isAction));
    std::string rewardEntry = returnEntryString(std::string("reward"), std::to_string(this->reward));

    std::string averageLatencyEntry;
    if(this->completeNum > 0)
        averageLatencyEntry = returnEntryString(std::string("averageLatency"), std::to_string(this->latency / (double)this->completeNum));
    else
        averageLatencyEntry = returnEntryString(std::string("averageLatency"), std::to_string(-1));

    std::string completeJobNumEntry = returnEntryString(std::string("completeJobNum"), std::to_string(this->completeNum));
    this->completeNum = 0; // 해당 time step에서 complete한 job의 개수이므로 state 보내고 초기화함.

    std::string sojournTimeEntry = returnEntryString(std::string("sojournTime"), std::to_string((simTime() - this->latestSendTime).dbl()));
    this->latestSendTime = simTime();

    networkStateMsg = "{" + nodeStateEntry + ", " + nodeProcessingEntry + ", " + linkWaitingEntry + ", " + jobWaitingEntry + ", " + activatedJobListEntry + ", " + jobIndexEntry + ", " + isActionEntry + ", " + rewardEntry + ", " + averageLatencyEntry + ", " + completeJobNumEntry + ", " + sojournTimeEntry + ", " + jobRemainEntry + "}";

    return networkStateMsg;

}

void dataplane::sendState(){ // info 다 들어오고 실행하는 구문임.

    sendPythonMessage("action"); // 파이썬에 state 보낼거라는 예고

    if(getPythonMessage() == "ok"){
        sendPythonMessage(this->previous_state);
        sendPythonMessage(this->current_state);
    }

    if(getPythonMessage() == "ok"){
        return;
    }
}

// agent가 python과 통신하는 부분
int dataplane::scheduleJob(job_base* newJob){
    // this->nodeState는 완성된 상태
    // this->nodeState 파이썬으로 전송하고 답 올때까지 기다려야함.
    // while(pBuf == "p") ~~~

    // send state
    this->current_state = getStateMsg(newJob, true);
    sendState();


    std::string response = getPythonMessage();

    sendPythonMessage("ok"); // offloading 받았다는 답장

    if(response == "void"){
        EV << "Received void action. Finish schedule." << endl;

        return -1;
    }

    EV << "Receive offloading : " << response << endl;

    std::vector<int> offloading;

    offloading = returnListToVector(response);

    // python에서 받아왔다고 가정
    std::vector<subtask_base*> subtasks = newJob->getSubtasks();

    // EV << "subtasks.size : " << subtasks.size() << "\n";

    for(int i = 0; i<subtasks.size(); i++){

        //int processingNode = randint(0, nodeNum-1);
        int processingNode = offloading[i];
        //int processingNode = 0;
        subtasks[i]->setProcessingNode(processingNode);

        if(i != 0){
            // 두번째 i는 i가 아니라 python에서 받아온 값으로 대체되어야 함
            subtasks[i-1]->setDestination(processingNode);
        }

        if(i == subtasks.size()-1){
            subtasks[i]->setDestination(newJob->getDestination());
        }


        subtasks[i]->setIndex(newJob->getIndex());


        Subtask* subtaskMsg = new Subtask("subtask");
        subtaskMsg->setSubtask(subtasks[i]);

        // i가 아니라 python에서 받아온 값으로 대체되어야 함
        send(subtaskMsg, "out", processingNode);
        //send(new cMessage("hello"), "out", i);
    }

    // 0이 아니라 python에서 받아온 값중 가장 첫 번째 있는 값으로 대체해야함
    return subtasks[0]->getProcessingNode();
}

void dataplane::pushNewJob(job_base* newJob){
    moveJob();
    if(jobWaitingQueue.size() < jobWaitingQueueLength)
        jobWaitingQueue.push(newJob);

    else{
        if(this->backlog.size() < 1000){
            backlog.push(newJob);
        }
    }


}

void dataplane::moveJob(){
    while(backlog.size() > 0 && jobWaitingQueue.size() < jobWaitingQueueLength){
        jobWaitingQueue.push(backlog.front());
        backlog.pop();
    }
}

int dataplane::getActivatedJobNum(){
    int result = 0;
    for(int i = 0; i < availableJobNum; i++){
        if(activatedJobQueue[i] != nullptr){
            result += 1;
        }
    }

    return result;
}

void dataplane::assignIndex(job_base* newJob, int index){
    newJob->setIndex(index);
    return;
}

void dataplane::handleMessage(cMessage *msg){
    EV << msg->getFullName() << endl;
    if(this->reset_network){ // 에피소드 이전 이벤트인지 확인 -> 에피소드 이전 이벤트면 버리고, 아니면 보내기
        if((((int)msg->getSendingTime().dbl() / this->episode_length) != ((int)simTime().dbl() / this->episode_length)) and strcmp(msg->getFullName(), "checkTime") != 0){
            if(strcmp(msg->getFullName(), "test") != 0){
                if(strcmp(msg->getFullName(), "complete") == 0 or strcmp(msg->getFullName(), "info") == 0){
                    delete msg;
                    msg = NULL;
                }
                return;
            }

        }
    }


    if(strcmp(msg->getFullName(), "test") == 0){
        nodeState = new double*[this->nodeNum];
        scheduleAt(simTime() + this->time_step_interval, test);
        getIotState();
    }

    else if(strcmp(msg->getFullName(), "complete") == 0){
        this->episodeReward += msg->par("totalComp").doubleValue();
        this->reward += 1;
        this->totalCompleteNum += 1;
        this->state_finish_num += 1;

        delete msg;
        msg = NULL;

    }

    else if(strcmp(msg->getFullName(), "sendState") == 0){
        getIotState();

        if (sendStateMsg->isScheduled()){
            cancelEvent(sendStateMsg);
            scheduleAt(simTime() + 0.1, sendStateMsg);
        }
        else{
            scheduleAt(simTime() + 0.1, sendStateMsg);
        }


    }
    else if(strcmp(msg->getFullName(), "checkTime") == 0){ // 한 에피소드 끝, 초기화 해야함

        srand(this->episode_num);
        for(int i = 0; i<adjacentList.size(); i++){ // 링크 되돌려둠.
            this->adjacentList[i]->setLinkCapacity(randint(1, 5));
        }

//        int disadvantage_index = randint(0, adjacentList.size()/2 - 1); // 한 에피소드 마다 랜덤으로 링크 안좋게 만듬
//
//        adjacentList[disadvantage_index * 2]->setLinkCapacity(1);
//        adjacentList[disadvantage_index * 2 + 1]->setLinkCapacity(1);



//        if(((int)simTime().dbl() / 100 > 20 and (int)simTime().dbl() / 100 < 50) or ((int)simTime().dbl() / 100 > 70 and (int)simTime().dbl() / 100 < 100)){
//            adjacentList[0]->setLinkCapacity(1);
//            adjacentList[1]->setLinkCapacity(1);
//        }
//        else{
//            adjacentList[0]->setLinkCapacity(5);
//            adjacentList[1]->setLinkCapacity(5);
//        }



        sendPythonMessage("episode_finish");
        if(getPythonMessage() == "ok"){
            std::string rewardEntry = returnEntryString(std::string("reward"), std::to_string(this->episodeReward));
            this->episodeReward = 0;

            std::string averageLatencyEntry = returnEntryString(std::string("averageLatency"), std::to_string(this->latency / (double)this->totalCompleteNum));
            std::string completeNumEntry = returnEntryString(std::string("completNum"), std::to_string(this->totalCompleteNum));
            this->totalCompleteNum = 0;

            std::string sojournTimeEntry = returnEntryString(std::string("sojournTime"), std::to_string((simTime() - this->latestSendTime).dbl()));
            std::string currentTimeEntry = returnEntryString(std::string("currentTime"), std::to_string(simTime().dbl()));

            std::string networkStateMsg = "{" + rewardEntry + ", " + sojournTimeEntry + ", " + completeNumEntry + ", " + currentTimeEntry + ", " + averageLatencyEntry + "}";
            this->latency = 0;
            this->completeNum = 0;

            sendPythonMessage(networkStateMsg);
            if(getPythonMessage() == "ok"){

                if(this->reset_network){
                    resetNetwork();
                }

                scheduleAt(simTime() + this->episode_length, checkTimeMsg);
            }

        }

    }

    else if(strcmp(msg->getFullName(), "info") == 0){
        State* stateMsg = check_and_cast<State*>(msg);
        state_base* state = const_cast<state_base*>(stateMsg->getState());

        double jobRemain = state->getRemain();
        double jobProgress = state->getProgress();
        double* jobRemainState = state->getState();
        int index = state->getNum(); // nodeIndex
        int currentProcessing = state->getCurrentProcessing(); // jobIndex
        int currentProcessingOrder = state->getOrder();
        double jobLatency = state->getLatency();
        int jobCompleteNum = state->getCompleteNum(); // 특정 iot에서 해당 time step에 complete한 job 개수
        double reward = state->getReward();

        // EV << "iot" << index << "[" << jobRemain << ", " << jobProgress << "]\n";
        //this->reward += jobProgress;
        this->completeNum += jobCompleteNum; // 모든 iot에서 수집한 해당 time step에 complete한 job 개수
        // this->episodeReward += jobCompleteNum;
        // this->totalCompleteNum += jobCompleteNum;
        this->latency += jobLatency;
        this->jobProgress += jobProgress;
        // this->reward += reward;
        // this->reward = -this->latency;
        this->jobRemain[index] = jobRemain;

        nodeState[index] = jobRemainState;

        // nodeProcessingState
        int* tempProcessingState = new int[this->availableJobNum * this->modelNum]{0};
        if(currentProcessingOrder != -1 && currentProcessing != -1){
            tempProcessingState[this->modelNum * currentProcessing + currentProcessingOrder] = 1;
        }
        nodeProcessingState[index] = tempProcessingState;
        // nodeProcessingState

        if(index == this->nodeNum - 1){ // state가 다 오면 job activate
            this->previous_state = this->current_state;
            if(jobWaitingQueue.size() > 0 && getActivatedJobNum() < availableJobNum){
                while(jobWaitingQueue.size() > 0 && getActivatedJobNum() < availableJobNum){
                    int flag = activateNewJob();
                    moveJob();
                    // EV << "scheduled one\n";
                }
                this->isActioned = true;
                // EV << this->isActioned;

                sendPythonMessage("stop"); // stop 보냄
                std::string response = getPythonMessage();
                if(response == "ok"){
                    EV << "Sent stop message." << endl;
                }
                else{
                    throw std::invalid_argument("response error");
                }

                if(jobWaitingQueue.size() == 0){
                    if (!sendStateMsg->isScheduled()){
                        scheduleAt(simTime() + 0.1, sendStateMsg);
                    }
                }

                this->reward = 0;
                this->jobProgress = 0;
            }

            for(int i = 0 ; i<nodeNum ; i++){
                delete[] nodeState[i];
                delete[] nodeProcessingState[i];

                nodeState[i] = NULL;
                nodeProcessingState[i] = NULL;
            }
        }

        delete msg;
    }
    else if(strcmp(msg->getFullName(), "ready") == 0){
        // EV << this->availableJobNum * (this->nodeNum + this->modelNum);

        if((int)(msg->par("nodeIndex")) == this->nodeNum - 1){ // 모든 노드 초기화 완료, python에 정보 보내기
            std::string adjacencyString = returnAdjacencyString(this->adjacentList);
            std::string initialize_msg = "";

            std::string modelNumString = returnEntryString(std::string("modelNum"), std::to_string(this->modelNum));
            std::string availableJobNumString = returnEntryString(std::string("availableJobNum"), std::to_string(this->availableJobNum)); // 네트워크에서 돌아갈 수 있는 job 개수(node feature에 영향을 미치는 수)
            std::string nodeNumString = returnEntryString(std::string("nodeNum"), std::to_string(this->nodeNum));
            std::string jobWaitingQueueLengthString = returnEntryString(std::string("jobWaitingQueueLength"), std::to_string(this->jobWaitingQueueLength));
            std::string adjacencyListString = returnEntryString(std::string("adjacencyList"), adjacencyString);
            std::string episode_length_string = returnEntryString(std::string("episode_length"), std::to_string(this->episode_length));
            std::string job_generate_rate_string = returnEntryString(std::string("job_generate_rate"), std::to_string(this->job_generate_rate));
            std::string node_capacity_string = returnEntryString(std::string("node_capacity"), std::to_string(this->low) + ", " +std::to_string(this->high));

            initialize_msg = "{" + modelNumString + ", " + availableJobNumString + ", " + nodeNumString + ", " + jobWaitingQueueLengthString + ", " +  adjacencyListString + ", " + episode_length_string + ", " + job_generate_rate_string + ", " + node_capacity_string + "}";

            sendPythonMessage(initialize_msg);

            std::string response = getPythonMessage();
            if(response == "init")
                EV << "Initialize complete." << endl;
                return;
        }

    }
}

void dataplane::getIotState(){
    for (int i = 0; i < gateSize("out"); i++) {
        cMessage* info = new cMessage("info");
        send(info, "out", i);
    }
}

void dataplane::refreshDisplay() const{
    char buf[100];

    sprintf(buf, "jobWaitingQueue length : %d, backlog length : %d, activated Job Num : %d", jobWaitingQueue.size(), backlog.size(), this->activatedJobNum);
    getDisplayString().setTagArg("t", 0, buf);
}
