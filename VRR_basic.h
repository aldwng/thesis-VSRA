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

/*
**********App-Net Voting**********

���Ǵ��ڱ�������
û�п�������ʧ�����
V17.1�Ļ����ϣ�ȥ����·�ɾ��߹����ж�request����С�Ŀ���
V17.2�Ļ����ϣ������������ʧ�Ļ��ƣ��������������߳���
����app��net���۵ķ���evaluate()�Ƿ���Ҫ�Ľ�
ÿһ��TESTNUM�����³�ʼ��app��net��decider�������µ�������Ϣ����TESTNUM�ڣ�n��requests,��Щrequests����ͬ����������Ϣ��ǰһ��requests�Ĳ����Ӱ������˵ĸ���
V17.3�Ļ����ϣ������µ�����ģ��
	APP: ��������� (�������м��ٿ���)
�Ƚ���app���ͶƱ

ͶƱ������
	BasicSum
	RangeVoting
	Cumulative
	SchulzeMethod
	lessVariance

Bug:
Data�����۽���޷�����evaltable[j][j]��Զ����߷֡����о��ǲ�ͬdataʵ���·�ɷ�����һ��.���̫����, double������int��ʾ, ����ɷ�ʴ���

�д��޸ģ�
	data��Ϊ�������Լ�������������Լ�������te·��, ��Ϊ�����Լ��ĸ������ܸ��غ���С��·��.

�����¼��
Q: Happiness: 0.895125 Maxbandwidth: 454.498 Latency: 0.200967
D: Happiness: 0.795262 Maxbandwidth: 428.849 Latency: 0.299775
V: Happiness: 0.898524 Maxbandwidth: 440.639 Latency: 0.232641

Q: Happiness: 0.876911 Maxbandwidth: 417.529 Latency: 0.405284
D: Happiness: 0.81943 Maxbandwidth: 407.103 Latency: 0.48591 latency���߿������. maxBandwidth��С��Ϊʲô, ���׼ȷ����data������?
V: Happiness: 0.883704 Maxbandwidth: 447.66 Latency: 0.403029

V: Happiness: 0.888129 Maxbandwidth: 425.975 Latency: 0.304937
Q: Happiness: 0.886639 Maxbandwidth: 429.374 Latency: 0.295495
D: Happiness: 0.804871 Maxbandwidth: 403.715 Latency: 0.399652

Q: Happiness: 0.902662 Maxbandwidth: 426.466 Latency: 0.294225
D: Happiness: 0.819325 Maxbandwidth: 397.579 Latency: 0.410125
D: Happiness: 0.889635 Maxbandwidth: 409.189 Latency: 0.380733
D: Happiness: 0.899392 Maxbandwidth: 400.342 Latency: 0.403634
V: Happiness: 0.945955 Maxbandwidth: 429.03 Latency: 0.307653
Q: Happiness: 0.938044 Maxbandwidth: 426.253 Latency: 0.304124
D: Happiness: 0.901506 Maxbandwidth: 401.594 Latency: 0.401752
data�ĳ���ÿ�μ�����������·�����ȴû�еõ�������, Ϊë!!!????????????????????????
Q: Happiness: 0.975434 Maxbandwidth: 585.14 Latency: 0.121525
D: Happiness: 0.910503 Maxbandwidth: 638.54 Latency: 0.258431
V: Happiness: 0.969765 Maxbandwidth: 608.89 Latency: 0.185785

V: Happiness: 0.967373 Maxbandwidth: 595.66 Latency: 0.219049
Q: Happiness: 0.952448 Maxbandwidth: 586.185 Latency: 0.201783
D: Happiness: 0.859728 Maxbandwidth: 631.04 Latency: 0.480867

V: Happiness: 0.965065 Maxbandwidth: 628.49 Latency: 0.261247
Q: Happiness: 0.979783 Maxbandwidth: 602.825 Latency: 0.12818
D: Happiness: 0.950749 Maxbandwidth: 617.82 Latency: 0.377748

V: Happiness: 0.97544 Maxbandwidth: 318.543 Latency: 0.354395
Q: Happiness: 0.965655 Maxbandwidth: 298.689 Latency: 0.45581
D: Happiness: 0.894418 Maxbandwidth: 307.015 Latency: 0.561096 qos��data����ͶƱ�ܸо�������qos��netͶƱ��ʲô���
D: Happiness: 0.930208 Maxbandwidth: 349.403 Latency: 0.392742 �޸�dataΪ����·����·��ʽ��Ľ��, ������data��Ҫ������ٶ��Լ��Ŀ���������·��

D: Happiness: 0.94381 Maxbandwidth: 339.637 Latency: 0.411227
D: Happiness: 0.910077 Maxbandwidth: 334.76 Latency: 0.468744
D: Happiness: 0.93519 Maxbandwidth: 330.627 Latency: 0.437659

D1: Happiness: 0.924229 Maxbandwidth: 330.815 Latency: 0.442439
D2: Happiness: 0.913722 Maxbandwidth: 324.958 Latency: 0.474889

D1: Happiness: 0.9247 Maxbandwidth: 335.648 Latency: 0.414219
D2: Happiness: 0.929029 Maxbandwidth: 322.611 Latency: 0.430575

D1: Happiness: 0.936172 Maxbandwidth: 332.282 Latency: 0.399488
D2: Happiness: 0.92817 Maxbandwidth: 334.358 Latency: 0.444957
D2: Happiness: 0.91766 Maxbandwidth: 326.904 Latency: 0.42726
D1: Happiness: 0.942589 Maxbandwidth: 330.679 Latency: 0.407218
D2: Happiness: 0.923731 Maxbandwidth: 332.455 Latency: 0.405932 0.75 0.25
D2: Happiness: 0.937409 Maxbandwidth: 326.02 Latency: 0.365457 0.9 0.1

V: Happiness: 0.994341 Maxbandwidth: 434.617 Latency: 0.298863
D: Happiness: 0.993875 Maxbandwidth: 432.518 Latency: 0.264899

V: Happiness: 0.988589 Maxbandwidth: 316.527 Latency: 0.452936
D: Happiness: 0.987938 Maxbandwidth: 311.363 Latency: 0.492913

V: Happiness: 0.985642 Maxbandwidth: 237.732 Latency: 0.608016
D: Happiness: 0.984745 Maxbandwidth: 230.716 Latency: 0.654709

V: Happiness: 0.966081 Maxbandwidth: 181.391 Latency: 1.04412
D: Happiness: 0.962558 Maxbandwidth: 171.731 Latency: 1.07444

V: Happiness: 0.972619 Maxbandwidth: 321.105 Latency: 0.898531
D: Happiness: 0.970846 Maxbandwidth: 325.712 Latency: 0.898191

V: Happiness: 0.961279 Maxbandwidth: 208.856 Latency: 1.30536
D: Happiness: 0.956833 Maxbandwidth: 209.458 Latency: 1.31723 dataӦ��֮��ͶƱ�޹���

V: Happiness: 0.956833 Maxbandwidth: 209.458 Latency: 1.31723
D: Happiness: 0.957349 Maxbandwidth: 264.827 Latency: 1.44911
V: Happiness: 0.963704 Maxbandwidth: 254.215 Latency: 1.45765
D: Happiness: 0.959215 Maxbandwidth: 243.458 Latency: 1.46
V: Happiness: 0.967524 Maxbandwidth: 258.222 Latency: 1.36358
D: Happiness: 0.96357 Maxbandwidth: 275.244 Latency: 1.40563
���Դ�ͼ
V: Happiness: 0.962839 Maxbandwidth: 505.057 Latency: 1.20299
D: Happiness: 0.957044 Maxbandwidth: 506.496 Latency: 1.22717 ���̫С�� dataӦ����ͶƱ������Ч��
data vs qos ��ͼ, �����е�Ч����
V: Happiness: 0.733474 Maxbandwidth: 369.863 Latency: 1.0472
Q: Happiness: 0.715538 Maxbandwidth: 372.567 Latency: 1.02576
D: Happiness: 0.657798 Maxbandwidth: 416.002 Latency: 1.13081

V: Happiness: 0.733005 Maxbandwidth: 372.707 Latency: 1.14848
Q: Happiness: 0.717813 Maxbandwidth: 366.744 Latency: 1.15341
D: Happiness: 0.660092 Maxbandwidth: 418.223 Latency: 1.18413

dataȨ�ر�Ϊ2
V: Happiness: 0.728251 Maxbandwidth: 388.006 Latency: 1.16747
Q: Happiness: 0.714577 Maxbandwidth: 360.363 Latency: 1.15896
D: Happiness: 0.655178 Maxbandwidth: 405.254 Latency: 1.22048

V: Happiness: 0.685406 Maxbandwidth: 204.66 Latency: 0.726811
Q: Happiness: 0.669256 Maxbandwidth: 198.303 Latency: 0.721944
D: Happiness: 0.611833 Maxbandwidth: 213.107 Latency: 0.787513
(d1, d2, d3�������޶�ԭĿ�ڵ�)
TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 100, MAXFLOW = 150, TESTNUM = 100, BGFLOW = 50, DATAVOTES = 2 (d1)
V: Happiness: 0.692154 Maxbandwidth: 415.741 Latency: 0.906603
Q: Happiness: 0.676414 Maxbandwidth: 388.207 Latency: 0.895633
D: Happiness: 0.612343 Maxbandwidth: 418.199 Latency: 0.996352

TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 250, TESTNUM = 1000, BGFLOW = 300, DATAVOTES = 2 (d2)
V: Happiness: 0.773632 Maxbandwidth: 171.186 Latency: 1.36827
Q: Happiness: 0.760415 Maxbandwidth: 164.692 Latency: 1.34745
D: Happiness: 0.702761 Maxbandwidth: 162.701 Latency: 1.41123

TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 250, TESTNUM = 1000, BGFLOW = 200, DATAVOTES = 2 (d3)
V: Happiness: 0.728002 Maxbandwidth: 385.823 Latency: 1.15597
Q: Happiness: 0.709255 Maxbandwidth: 352.604 Latency: 1.18643
D: Happiness: 0.656064 Maxbandwidth: 393.226 Latency: 1.20456
TESTNUM = 2000
V: Happiness: 0.729544 Maxbandwidth: 386.122 Latency: 1.15702
Q: Happiness: 0.720425 Maxbandwidth: 372.486 Latency: 1.11325
D: Happiness: 0.654659 Maxbandwidth: 413.799 Latency: 1.17608

TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 100, MAXFLOW = 30, TESTNUM = 100, BGFLOW = 50, DATAVOTES = 2
V: Happiness: 0.850565 Maxbandwidth: 348.842 Latency: 0.437951
Q: Happiness: 0.865563 Maxbandwidth: 351.187 Latency: 0.429191
TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 50, MAXFLOW = 50, TESTNUM = 100, BGFLOW = 50, DATAVOTES = 2
V: Happiness: 0.839245 Maxbandwidth: 474.749 Latency: 0.293241
Q: Happiness: 0.840698 Maxbandwidth: 463.363 Latency: 0.295427
D: Happiness: 0.79574 Maxbandwidth: 477.519 Latency: 0.325227
TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 50, MAXFLOW = 50, TESTNUM = 300, BGFLOW = 50, DATAVOTES = 2, (d4)����ԭĿ��㡣V��Ч����������ΪQ�����з��������۲���D�����з��������۲���öർ��
V: Happiness: 0.840495 Maxbandwidth: 469.184 Latency: 0.294954
Q: Happiness: 0.848465 Maxbandwidth: 465.946 Latency: 0.294115
D: Happiness: 0.789069 Maxbandwidth: 472.021 Latency: 0.337047

TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 200, TESTNUM = 2000, BGFLOW = 200, DATAVOTES = 2 (d5)
V: Happiness: 0.866276 Maxbandwidth: 218.744 Latency: 1.24485
Q: Happiness: 0.860028 Maxbandwidth: 214.485 Latency: 1.28248
D: Happiness: 0.825315 Maxbandwidth: 223.534 Latency: 1.31411
DATAVOTES = 3
Happiness: 0.861351 Maxbandwidth: 219.522 Latency: 1.29433
����Ϊqos, data, mlu���в���
TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 200, TESTNUM = 2000, BGFLOW = 200, DATAVOTES = 2, MLUVOTES = 8
V: Happiness: 0.868512 Maxbandwidth: 228.517 Latency: 1.261 Mlu: 0.988938
Q: Happiness: 0.861917 Maxbandwidth: 218.142 Latency: 1.28706 Mlu: 0.986573
D: Happiness: 0.824265 Maxbandwidth: 229.78 Latency: 1.25803 Mlu: 0.987109
M: Happiness: 0.840504 Maxbandwidth: 224.458 Latency: 1.30177 Mlu: 0.987154
TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 200, TESTNUM = 2000, BGFLOW = 200, DATAVOTES = 2, MLUVOTES = 8
V: Happiness: 0.865979 Maxbandwidth: 263.166 Latency: 0.921623 Mlu: 0.984039
Q: Happiness: 0.872749 Maxbandwidth: 265.613 Latency: 0.841729 Mlu: 0.986889
D: Happiness: 0.853043 Maxbandwidth: 258.439 Latency: 0.890482 Mlu: 0.985894
M: Happiness: 0.848621 Maxbandwidth: 270.882 Latency: 0.937509 Mlu: 0.982428

V: Happiness: 0.870741 Maxbandwidth: 265.388 Latency: 0.914538 Mlu: 0.981135
Q: Happiness: 0.869322 Maxbandwidth: 249.039 Latency: 0.903431 Mlu: 0.986547
D: Happiness: 0.84177 Maxbandwidth: 246.469 Latency: 0.993804 Mlu: 0.984939
M: Happiness: 0.848027 Maxbandwidth: 275.777 Latency: 0.92094 Mlu: 0.986251

TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 50, MAXFLOW = 50, TESTNUM = 200, BGFLOW = 50, DATAVOTES = 2, MLUVOTES = 8
V: Happiness: 0.853631 Maxbandwidth: 475.193 Latency: 0.296971 Mlu: 0.570945
Q: Happiness: 0.848486 Maxbandwidth: 466.627 Latency: 0.301425 Mlu: 0.610081
D: Happiness: 0.800567 Maxbandwidth: 473.107 Latency: 0.326964 Mlu: 0.578052
M: Happiness: 0.810848 Maxbandwidth: 466.159 Latency: 0.358556 Mlu: 0.573117
V: Happiness: 0.853183 Maxbandwidth: 471.51 Latency: 0.29213 Mlu: 0.573721 (d6)
Q: Happiness: 0.851862 Maxbandwidth: 468.053 Latency: 0.292768 Mlu: 0.604495
D: Happiness: 0.801041 Maxbandwidth: 468.434 Latency: 0.334051 Mlu: 0.581319
M: Happiness: 0.810058 Maxbandwidth: 479.647 Latency: 0.341459 Mlu: 0.560505
TOPO = Abilene, QOSNUM = 6, DATANUM = 6, MAXREQ = 1, MAXFLOW = 200, TESTNUM = 1000, BGFLOW = 100, DATAVOTES = 2, MLUVOTES = 8 (d7) single flow
V: Happiness: 0.865453 Maxbandwidth: 650.708 Latency: 0.743391 Mlu: 0.562406
Q: Happiness: 0.859673 Maxbandwidth: 626.183 Latency: 0.735107 Mlu: 0.560032
D: Happiness: 0.829522 Maxbandwidth: 674.386 Latency: 0.901424 Mlu: 0.557697
M: Happiness: 0.846249 Maxbandwidth: 648.135 Latency: 0.783936 Mlu: 0.560835

TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 5, MAXFLOW = 200, TESTNUM = 2000, BGFLOW = 100, DATAVOTES = 2, MLUVOTES = 8 (d8)
V: Happiness: 0.741143 Maxbandwidth: 690.591 Latency: 0.729625 Mlu: 0.613249
Q: Happiness: 0.731377 Maxbandwidth: 625.53 Latency: 0.668227 Mlu: 0.632743
D: Happiness: 0.683448 Maxbandwidth: 694.247 Latency: 0.816444 Mlu: 0.605311
M: Happiness: 0.676156 Maxbandwidth: 696.793 Latency: 0.840841 Mlu: 0.613233 (����֮���mlu, �����ڼ���·�ɵ�ʱ��ѡ����mlu·���ڲ�������֮��δ��ʹ��ȫ����mlu��С, ʵ���Ϲؼ�������·��ʱ����req�Ĵ�С�������)
TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 1, MAXFLOW = 200, TESTNUM = 8000, BGFLOW = 100, DATAVOTES = 2, MLUVOTES = 8
V: Happiness: 0.759012 Maxbandwidth: 725.028 Latency: 0.691691 Mlu: 0.609007
Q: Happiness: 0.756929 Maxbandwidth: 664.362 Latency: 0.600273 Mlu: 0.614288
D: Happiness: 0.691022 Maxbandwidth: 728.566 Latency: 0.782636 Mlu: 0.610817 (Ϊʲômaxbandwidth��mluС, ��Ϊ���ﻹ����һ��data���໥ͶƱ�Ĺ���)
M: Happiness: 0.693717 Maxbandwidth: 733.116 Latency: 0.793319 Mlu: 0.614766

TOPO = ATTBig, QOSNUM = 6, DATANUM = 6, MAXREQ = 100, MAXFLOW = 100, TESTNUM = 100, BGFLOW = 100, DATAVOTES = 2, MLUVOTES = 8 (d9)
V: Happiness: 0.694165 Maxbandwidth: 396.528 Latency: 0.733144 Mlu: 0.781375 (��ʵ��, �Աȷ�����Q��D������֤ÿ�εĽ���������ŵ�, ��mlu����·�ֲ�֪��req�Ĵ�С, ����δ�صõ���������)
Q: Happiness: 0.673092 Maxbandwidth: 349.617 Latency: 0.714403 Mlu: 0.848046 
D: Happiness: 0.638222 Maxbandwidth: 387.091 Latency: 0.802585 Mlu: 0.791552
M: Happiness: 0.648944 Maxbandwidth: 385.705 Latency: 0.824385 Mlu: 0.783565

2017.11 - post
scenario: voting among apps
TOPO = ATTBig, QOSNUM = 10, MAXREQ = 100, MAXFLOW = 100, MINFLOW = 10, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10 (11a)
V: Happiness: 0.449037 Maxbandwidth: 240.606 Latency: 1.22769 Mlu: 0.91386
V: Happiness: 0.448989 Maxbandwidth: 241.403 Latency: 1.22985 Mlu: 0.911811
Q: Happiness: 0.393552 Maxbandwidth: 250.177 Latency: 1.19808 Mlu: 0.901373
Q: Happiness: 0.391594 Maxbandwidth: 245.477 Latency: 1.21164 Mlu: 0.90285
RLV: Happiness: 0.818805 Maxbandwidth: 251.337 Latency: 1.22202 Mlu: 0.896175
RLV: Happiness: 0.811594 Maxbandwidth: 249.184 Latency: 1.22622 Mlu: 0.898981

MAXREQ = 80, ��ʱ����������·��Դȫ����ռ�á�MAXREQ̫��û�������С�˲��Խ���öԱ�Ч����
TOPO = ATTBig, QOSNUM = 10, MAXREQ = 80, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11a)
Q: Happiness: 0.392682 Maxbandwidth: 309.304 Latency: 1.12913 Mlu: 0.878751
V: Happiness: 0.443723 Maxbandwidth: 290.656 Latency: 1.1667 Mlu: 0.895968
RLV1: Happiness: 0.608263 Maxbandwidth: 308.619 Latency: 1.15007 Mlu: 0.874295
RLV2: Happiness: 0.734019 Maxbandwidth: 308.426 Latency: 1.16193 Mlu: 0.873161
���Կ���RL2�õ��ý������ȸ��ߡ�

TOPO = ATTBig, QOSNUM = 10, MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11b)
Q: Happiness: 0.385354 Maxbandwidth: 205.043 Latency: 1.26654 Mlu: 0.91797
V: Happiness: 0.44016 Maxbandwidth: 199.005 Latency: 1.29694 Mlu: 0.93071
RL1V: Happiness: 0.623513 Maxbandwidth: 211.575 Latency: 1.27975 Mlu: 0.913374
RL2V: Happiness: 0.709353 Maxbandwidth: 206.216 Latency: 1.28377 Mlu: 0.91503
�ڽϴ��ͼ��ATTBig��App-QoS֮�����ͶƱ����Ȼ���һ�������ƣ���ȡRLV����������Ը��ߣ�������һ�֣�����ʱָ���϶����������ơ�

TOPO = ATTBig, QOSNUM = 10, DECNUM = 11, MLUVOTES = 8, MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11c)
Q: Happiness: 0.421915 Maxbandwidth: 203.807 Latency: 1.27462 Mlu: 0.919407
M: Happiness: 0.331457 Maxbandwidth: 220.75 Latency: 1.37259 Mlu: 0.890173
V: Happiness: 0.480904 Maxbandwidth: 214.999 Latency: 1.30021 Mlu: 0.907086
RL1V: Happiness: 0.626703 Maxbandwidth: 226.862 Latency: 1.27562 Mlu: 0.894204
RL2V: Happiness: 0.70293 Maxbandwidth: 216.379 Latency: 1.28953 Mlu: 0.90316
MLU��Ȩ������Ҳ���Բ�ȡ���Ƶ�RL˼·���иı䡣��ȡ����Ч������ʱ��ע��ȡ������Ч�����ݣ�reqnumber65�Ժ��mlu��һ��0.999

TOPO = ATTBig, QOSNUM = 10, DECNUM = 11, MLUVOTES = 8, MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11d)
Q: Happiness: 0.421915 Maxbandwidth: 203.807 Latency: 1.27462 Mlu: 0.919407
M: Happiness: 0.331457 Maxbandwidth: 220.75 Latency: 1.37259 Mlu: 0.890173
V: Happiness: 0.480904 Maxbandwidth: 214.999 Latency: 1.30021 Mlu: 0.907086
WRL1V: Happiness: 0.464025 Maxbandwidth: 225.63 Latency: 1.30095 Mlu: 0.896269
WRL2V: Happiness: 0.475272 Maxbandwidth: 220.201 Latency: 1.29275 Mlu: 0.901814
WRL1+RL1V: Happiness: 0.622202 Maxbandwidth: 218.507 Latency: 1.30734 Mlu: 0.900765
WRL2+RL2V: Happiness: 0.693377 Maxbandwidth: 223.076 Latency: 1.30915 Mlu: 0.898153
WRL2+RL1V: Happiness: 0.627052 Maxbandwidth: 221.723 Latency: 1.29996 Mlu: 0.897365
WRL2+RL2V: Happiness: 0.700135 Maxbandwidth: 218.225 Latency: 1.3006 Mlu: 0.896135

TOPO = ATTBig, DATANUM = 10, DECNUM = 10, MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11e)
D: Happiness: 0.941276 Maxbandwidth: 217.562 Latency: 1.36854 Mlu: 0.897641 (alpha is 200 * 0.75)
V: Happiness: 0.949906 Maxbandwidth: 217.988 Latency: 1.35267 Mlu: 0.895304
RL1V: Happiness: 0.957661 Maxbandwidth: 220.205 Latency: 1.36274 Mlu: 0.894459
RL2V: Happiness: 0.963564 Maxbandwidth: 222.655 Latency: 1.35876 Mlu: 0.893978
dataapp�����RLͶƱ������ȸ��ߣ�����maxbandwidthҲ����

TOPO = ATTBig, DATANUM = 10, DECNUM = 11, MLUVOTES = 8(init), MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11f)
D: Happiness: 0.946607 Maxbandwidth: 223.128 Latency: 1.36949 Mlu: 0.897053 (alpha is 200 * 0.75)
M: Happiness: 0.946337 Maxbandwidth: 220.528 Latency: 1.37383 Mlu: 0.88932
V: Happiness: 0.954355 Maxbandwidth: 219.151 Latency: 1.35906 Mlu: 0.895179
RL1V: Happiness: 0.960952 Maxbandwidth: 221.539 Latency: 1.36902 Mlu: 0.889897
RL2V: Happiness: 0.965834 Maxbandwidth: 219.892 Latency: 1.3718 Mlu: 0.896359
WRL1V: Happiness: 0.954096 Maxbandwidth: 214.926 Latency: 1.36934 Mlu: 0.895485
WRL2V: Happiness: 0.955137 Maxbandwidth: 220.913 Latency: 1.35346 Mlu: 0.891687
WRL1+RL1V: Happiness: 0.960953 Maxbandwidth: 219.316 Latency: 1.37706 Mlu: 0.893363
WRL1+RL2V: Happiness: 0.966761 Maxbandwidth: 217.077 Latency: 1.37107 Mlu: 0.897807
WRL2+RL1V: Happiness: 0.961669 Maxbandwidth: 216.92 Latency: 1.36282 Mlu: 0.894551
WRL2+RL2V: Happiness: 0.966814 Maxbandwidth: 222.398 Latency: 1.37172 Mlu: 0.88969

TOPO = ATTBig, QOSNUM = 10, DATANUM = 10, DECNUM = 20, DATAVOTES = 1, MAXREQ = 120, MAXFLOW = 150, MINFLOW = 5, TESTNUM = 200, BGFLOW = 20, LOSTFLOW = 10, BIGGER = 10 (11g) (Voter�࣬bgflow���ý�С����ԭʼͼ�еı�����̫��, maxflowҲ��С)
Q: Happiness: 0.700644 Maxbandwidth: 364.324 Latency: 0.937386 Mlu: 0.826275(alpha is 200)
D: Happiness: 0.639115 Maxbandwidth: 384.44 Latency: 1.04315 Mlu: 0.768237(alpha is 200)
V: Happiness: 0.716634 Maxbandwidth: 378.678 Latency: 0.935105 Mlu: 0.810699
RL1V: Happiness: 0.784004 Maxbandwidth: 382.175 Latency: 0.945723 Mlu: 0.805847
RL2V: Happiness: 0.818596 Maxbandwidth: 389.359 Latency: 0.918992 Mlu: 0.799351

RM1�����о�
TOPO = ATTBig, QOSNUM = 10, MAXREQ = 120, MAXFLOW = 200, MINFLOW = 5, TESTNUM = 200, BGFLOW = 100, LOSTFLOW = 10, BIGGER = 10 (11h)
*/