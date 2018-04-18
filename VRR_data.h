#include "VRR_graph.h"
#include "VRR_voting.h"

#ifndef VRR_DATA_H
#define VRR_DATA_H

class Data : public Voter 
{
public:
	set<int> node_set;
	vector<double> distance;
	vector<int> parent;
	Graph* graph;

	Data(int ID, Graph* GRAPH) : Voter(ID, GRAPH)
	{
		graph = GRAPH;
		parent.resize(GRAPH->n);
		distance.resize(GRAPH->n);

		//alpha = (int)(RLFACTOR * 0.75);
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
			double capacity = graph->adjM[SRC][dst]->cap;
			if (capacity <= ld) continue;
			double bw = (capacity - ld) > distance[SRC] ? distance[SRC] : (capacity - ld);
			if (bw > distance[dst])
			{
				distance[dst] = bw;
				parent[dst] = SRC;
			}
		}
	}

	int find_max()
	{
		double max_bw = -1.0;
		int node = -1;
		for (set<int>::iterator it = node_set.begin(); it != node_set.end(); it++)
		{
			if (max_bw < distance[*it])
			{
				max_bw = distance[*it];
				node = *it;
			}
		}
		return node;
	}

	void dijkstra(Request* REQ)
	{
		bool found = false;
		node_set.clear();
		for (int i = 0; i < graph->n; i++)
		{
			parent[i] = -1;
			distance[i] = -1.0;
			node_set.insert(i);
		}
		distance[REQ->src] = INF;
		update(REQ->src);
		node_set.erase(REQ->src);
		while (!node_set.empty())
		{
			int next = find_max();
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
			cout << "VRR Data dijkstra(): no path found" << "\n";
			return;
		}
	}

	void update_sp(int SRC)
	{
		for (int i = 0; i < graph->adjL[SRC].size(); i++)
		{
			int dst = graph->adjL[SRC][i]->dst;
			double mine_ld = adj_load[SRC][dst];
			double ld = graph->load[SRC][dst];
			double capacity = graph->adjL[SRC][i]->cap;
			if (mine_ld >= capacity)
			{
				mine_ld = capacity * 0.999;
			}
			if (ld >= capacity)
			{
				ld = capacity * 0.999;
			}
			double bw = ((double)alpha / (double)RMFACTOR) * mine_ld + ((double)(RMFACTOR - alpha) / (double)RMFACTOR) * mine_ld;
			bw = bw > distance[SRC] ? distance[SRC] : bw;
			if (distance[dst] < bw) 
			{
				distance[dst] = bw;
				parent[dst] = SRC;
			}
		}
	}

	int find_max_sp()
	{
		double max_bw = -1;
		int node = -1;
		for (set<int>::iterator it = node_set.begin(); it != node_set.end(); it++)
		{
			if (max_bw < distance[*it])
			{
				max_bw = distance[*it];
				node = *it;
			}
		}
		return node;
	}

	void dijkstra_sp(Request* REQ)
	{
		bool found = false;
		node_set.clear();
		for (int i = 0; i < graph->n; i++)
		{
			parent[i] = -1;
			distance[i] = -1.0;
			node_set.insert(i);
		}
		distance[REQ->src] = INF;
		update_sp(REQ->src);
		node_set.erase(REQ->src);
		while (!node_set.empty())
		{
			int next = find_max_sp();
			if (next == REQ->dst)
			{
				found = true;
				record_path(REQ);
				return;
			}
			update_sp(next);
			node_set.erase(next);
		}
		if (!found) 
		{
			cout << "VRR Data dijkstra_sp(): no path found" << "\n";
			return;
		}
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

	void propose(Request* REQ)
	{
		if (REQ->type == id) dijkstra(REQ);
		else dijkstra_sp(REQ);
	}

	void evaluate(Request* REQ, vector<Voter*>& VOTERS)
	{
		if (REQ->type == id) // calculate the bandwidth of the path
		{
			for (int i = 0; i < VOTERS.size(); i++)
			{
				double bw = INF;
				for (int j = 0; j < VOTERS[i]->path_record.size() - 1; j++)
				{
					int from = VOTERS[i]->path_record[j];
					int to = VOTERS[i]->path_record[j + 1];
					if (graph->adjM[from][to]->cap - graph->load[from][to] <= 0.1) bw = 0.1;
					else 
					{
						bw = (graph->adjM[from][to]->cap - graph->load[from][to]) < bw ? (graph->adjM[from][to]->cap - graph->load[from][to]) : bw; 
					}
				}
				evaluate_list[i] = 10 / bw; // the best candidate has the lowest score
			}
		}
		else // calculate the mlu based on adj_load
		{
			for (int i = 0; i < VOTERS.size(); i++)
			{
				double bw = INF;
				for (int j = 0; j < VOTERS[i]->path_record.size() - 1; j++)
				{
					int from = VOTERS[i]->path_record[j];
					int to = VOTERS[i]->path_record[j + 1];
					double bw_flag = graph->adjM[from][to]->cap - adj_load[from][to] * 0.75 - graph->load[from][to] * 0.25;
					if (bw_flag < 0) bw_flag = 1.0;
					bw = bw_flag > bw ? bw : bw_flag;
				}
				evaluate_list[i] = 10 / bw; // the best candidate has the lowest score;
			}
		}
	}

	~Data() { ; }
};

#endif