/*
 * myFunction.h
 *
 *  Created on: 2022. 7. 14.
 *      Author: user
 */

#ifndef MYFUNCTION_H_
#define MYFUNCTION_H_

#include <iostream>
#include "link.h"

int randint(int a, int b);
double randDouble(double a, double b);
std::string returnEntryString(std::string key, std::string value);
std::string returnAdjacencyString(std::vector<link*> adjacentList);
std::vector<int> returnListToVector(std::string);


#endif /* MYFUNCTION_H_ */
