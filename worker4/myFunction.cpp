/*
 * myFunction.cpp
 *
 *  Created on: 2022. 7. 14.
 *      Author: user
 */

#include "myFunction.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

int randint(int a, int b){
    int randInit = rand();
    return (int)(randInit % (b-a+1) + a);
}

double randDouble(double a, double b){
    double randInit = rand();
    return (randInit / RAND_MAX) * b + a;
}

std::string returnEntryString(std::string key, std::string value){
    std::string result = "";
    result = "\"" + key + "\":\"" + value + "\"";

    return result;
}

std::string returnAdjacencyString(std::vector<link*> adjacentList){
    std::string incidence1 = "[";
    std::string incidence2 = "[";
    std::string result = "";

    for(int i = 0; i<adjacentList.size(); i++){
        incidence1 += std::to_string(adjacentList[i]->getIncidence1()) + ",";
        incidence2 += std::to_string(adjacentList[i]->getIncidence2()) + ",";

        incidence2 += std::to_string(adjacentList[i]->getIncidence1()) + ",";
        incidence1 += std::to_string(adjacentList[i]->getIncidence2()) + ",";
    }

    incidence1 += "]";
    incidence2 += "]";

    result = "[" + incidence1 + "," + incidence2 + "]";

    return result;

}

std::vector<int> returnListToVector(std::string listString){
    std::vector<int> result;
    int i = 1;

    while(i < listString.size()){
        std::string temp = "";
        while(i < listString.size() && listString.at(i) != ','){
            temp += listString.at(i);
            i += 1;
        }
        try{
            result.push_back(std::stoi(temp));
        }
        catch(std::string temp) {
            throw temp;
        }

        i += 1;
    }

    return result;

}
