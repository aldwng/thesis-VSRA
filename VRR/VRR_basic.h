#ifndef VRR_BASIC_H
#define VRR_BASIC_H

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <time.h>
#include <iomanip>
#include <cstring>

using namespace std;

const double INF = 1e7; // infinity great
const double RINF = 1e-7; // infinity small
const double DIVZERO = 0.001; // zero denominator

const int QOSNUM = 10; // the number of apps requiring low latency
const int DATANUM = 0; // the number of apps requiring big bandwidth
const int APPNUM = QOSNUM + DATANUM;
const int DECNUM = QOSNUM + DATANUM; // the number of Decider objects

int N; // the number of nodes in a graph
int M; // the number of edges in a graph

const int MAXREQ = 120; // the number of flow requests
const int MAXFLOW = 200; // max flow rate
const int MINFLOW = 5; // min flow rate
const int TESTNUM = 100; // rounds of test

const int BIGGER = 10; // link capacity enlarging factor
const int BGFLOW = 100; // max rate of initiative background flow 
const int LOSTFLOW = 10; // max rate of flow disappearing

const int RANGE = 99;
const int DATAVOTES = 1; // weight
const int MLUVOTES = 8; // weight

const int RMFACTOR = 200; // for reinforcement, alpha + beta

#endif
