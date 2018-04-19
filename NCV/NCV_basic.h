#pragma once
#ifndef NCV_BASIC_H
#define NCV_BASIC_H

#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <map>
#include <iterator>

using namespace std;

const int SERVICE_TYPE = 25; // at most, 26 kinds of type, which is easily represented in lowercase letter by 'char'
const int MAX_TRAFFIC_NUM = 4000;
const int UNIT_TRAFFIC_NUM = 100;
const int MAX_TRAFFIC_RATE = 10; // max consumption of bandwidth each traffic
const int MIN_TRAFFIC_RATE = 1; 

const int MAX_CHAIN_LENGTH = 10; // max length of service chain
const int MIN_CHAIN_LENGTH = 1; // min length of service chain
const int SIMPLEFATTREE_LEVEL = 4;
const int HOST_VOL = 20; // the max number of vms running in a host
const double LINK_BASE_CAPACITY = 1000; // the minimal capacity of links in the tree topo
const double VM_RCS = 50.0;

const int MAX_NEWVM_CONSTRAINT = 3;
const int MIN_NEWVM_CONSTRAINT = 2;
const int TEST_ROUND = 5;
const int NUM_ENTITY = 5;

const int METHOD_RANGE = 99;

const int JVP_STRETCH_N = 600;
const double JVP_STRETCH_ALPHA = 0.1;

#endif
