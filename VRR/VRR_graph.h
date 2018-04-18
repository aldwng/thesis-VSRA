#ifndef VRR_GRAPH_H
#define VRR_GRAPH_H

#include "VRR_basic.h"

class Edge
{
public:
	int id, src, dst;
	double cap;

	Edge(){ ; }

	Edge(int ID, int SRC, int DST, double BW)
	{
		id = ID;
		src = SRC;
		dst = DST;
		cap = BW;
	}

	~Edge(){ ; }
};

class Node
{
public:
	int id;
	int degree;

	Node(){ ; }

	Node(int ID)
	{
		id = ID;
		degree = 0;
	}

	~Node(){ ; }
};

class Request
{
public:
	int id, type, src, dst;
	double rate;

	Request(){ ; }

	Request(int ID, int TYPE, int SRC, int DST, double RATE)
	{
		id = ID;
		type = TYPE;
		src = SRC;
		dst = DST;
		rate = RATE;
	}

	~Request(){ ; }
};

class Graph{
public:
	int n, m; // n nodes, m edges
	vector<Edge*> incL; // incident list, the list of all edges
	vector<vector<Edge*> > adjL, adjM; // adjacency list, adjacency matrix
	vector<vector<double> > load;

	Graph(){ ; }

	Graph(string address) // initiate a topology graph from a .txt file
	{
		ifstream infile(address.c_str());
		infile >> n >> m;
		N = n;
		M = m;

		adjL.resize(n, vector<Edge*>(0));
		adjM.resize(n, vector<Edge*>(n, NULL));
		load.resize(n, vector<double>(n, 0.0));

		int id, src, dst;
		double bw;
		for (int i = 0; i < m; i++)
		{
			infile >> id >> src >> dst >> bw;
			Edge* edge = new Edge(id, src, dst, bw * BIGGER);
			incL.push_back(edge);
			adjL[src].push_back(edge);
			adjM[src][dst] = edge;
		}
	}

	~Graph(){;}
};

#endif
