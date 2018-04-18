#include "VRR_voting.h"
#include "VRR_graph.h"

#ifndef VRR_MLU_H
#define VRR_MLU_H

class Mlu: public Voter
{
public:
	// MLU: inorder to minimize the max link utilization of a route
	set<int> node_set;
	vector<int> parent;
	vector<double> distance;
	Graph *graph;

	Mlu(int ID, Graph* GRAPH): Voter(ID, GRAPH)
	{
		graph = GRAPH;
		parent.resize(graph->n);
		distance.resize(graph->n);

		accepted = 0;
		aborted = 0;
	}

	void init()
	{
		path_record.clear();
		evaluate_list.clear();
		evaluate_list.resize(DECNUM);
	}

	void update(int SRC)
	{
		for (int i = 0; i < graph->adjL[SRC].size(); i++)
		{
			int dst = graph->adjL[SRC][i]->dst;
			double ld = graph->load[SRC][dst];
			double capacity = graph->adjL[SRC][i]->cap;
			double te = 0.0;
			if (ld > capacity)
				te = 1.0;
			else te = (ld / capacity) > distance[SRC] ? (ld / capacity) : distance[SRC];
			if (te < distance[dst])
			{
				distance[dst] = te;
				parent[dst] = SRC;
			}
		}
	}

	int find_min()
	{
		double mlu = 1.0; // mlu does not exceed 1
		int node = -1;
		for (set<int>::iterator it = node_set.begin(); it != node_set.end(); it++)
		{
			if (mlu > distance[*it])
			{
				mlu = distance[*it];
				node = *it;
			}
		}
		return node;
	}

	void record_path(Request* REQ)
	{
		int node = REQ->dst;
		do
		{
			path_record.insert(path_record.begin(), node);
			node = parent[node];
		} while (node != -1);
	}

	void dijkstra(Request* REQ)
	{
		bool found = false;
		node_set.clear();
		for (int i = 0; i < graph->n; i++)
		{
			distance[i] = 1.1;
			parent[i] = -1;
			node_set.insert(i);
		}

		distance[REQ->src] = 0.0;
		update(REQ->src);
		node_set.erase(REQ->src);

		while (!node_set.empty())
		{
			int next = find_min();
			if (next == REQ->dst)
			{
				found = true;
				record_path(REQ);
				return;
			}
			update(next);
			node_set.erase(next);
		}
		if (!found)
		{
            cout << "VRR Mlu dijkstra(): no path found" << "\n";
			return;
		}
	}

	void propose(Request* REQ)
	{
		dijkstra(REQ);
	}
	
	void evaluate(Request* REQ, vector<Voter*>& VOTERS)
	{
		for (int i = 0; i < VOTERS.size(); i++)
		{
			double te = 0.0;
			for (int j = 0; j < VOTERS[i]->path_record.size() - 1; j++)
			{
				int from = VOTERS[i]->path_record[j];
				int to = VOTERS[i]->path_record[j + 1];
				if (te < graph->load[from][to] / graph->adjM[from][to]->cap)
				{
					te = graph->load[from][to] / graph->adjM[from][to]->cap;
				}
			}
			if (te > 1.0) te = 1.0;
			evaluate_list[i] = te; 
			// the best candidate has the lowest socre
		}
	}

	~Mlu() { ; }
};
#endif
