#pragma once

#ifndef VBR_BASIC_H
#define VBR_BASIC_H

#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <fstream>

const int MAXVOTER = 30;
const int MAXCANDIDATE = 40;
const float FAKEVOTER = 1.0;
const int ITERATION = 15; // the number of iterations will be tested during the routine
const int METHODS = 6;
const int RANGE = 99;

const int DECREASEFACTOR = 0.01;
const int INCREASEFACTOR = 100;

const int CONSTBALLOT = 9; // mischief voters' true ballot for every candidate
const int COMPROMISING_NUM = (MAXCANDIDATE / 2); // the number of ballot-compromised candidates
const int BURYING_NUM = (MAXCANDIDATE / 2); // the number of ballot-buried candidates

const int MODIFY_FACTOR = 3;
const int PROMOTE_FACTOR = 0.4;
const int PROMOTE_ROUNDS = 3;

// const int ALLIANCES = (MAXCANDIDATE / 5);

using namespace std;

#endif
