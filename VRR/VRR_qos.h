#include "VRR_voting.h"
#include "VRR_graph.h"

#ifndef VRR_QOS_H
#define VRR_QOS_H

class Qos: public Voter
{
public:
	set<int> node_set; // unchecked node set
	vector<int> parent; // parent node
	vector<double> distance; // distance record
	vector<double> latency;
	Graph* graph;
	// used for routing algorithm

	Qos(int ID, Graph* GRAPH) : Voter(ID, GRAPH)
	{
		graph = GRAPH;
		parent.resize(GRAPH->n);
		distance.resize(GRAPH->n);
		latency.resize(GRAPH->n);

		aborted = 0;
		accepted = 0;
	}

	void update(int SRC)
	{
		for (int i = 0; i < graph->adjL[SRC].size(); i++)
		{
			int dst = graph->adjL[SRC][i]->dst;
			double ld = graph->load[SRC][dst];
			double capacity = graph->adjL[SRC][i]->cap;

			// 1/(c-x) latency compute formula
			double lat = 0.0;

			if (ld > capacity)
			{
				continue;
			}
			else if (capacity - ld == 0)
			{
				lat = distance[SRC] + 1 / DIVZERO;
			}
			else
			{
				lat = distance[SRC] + 1 / (capacity - ld);
			}

			if (lat < distance[dst])
			{
				distance[dst] = lat;
				parent[dst] = SRC;
			}
		}
	}

	int find_min()
	{
		double min_distance = INF;
		int node = -1;
		for (set<int>::iterator it = node_set.begin(); it != node_set.end(); it++)
		{
			if (min_distance > distance[*it])
			{
				min_distance = distance[*it];
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
			distance[i] = INF;
			parent[i] = INF;
			node_set.insert(i);
		}

		distance[REQ->src] = 0;
		parent[REQ->src] = -1;
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
	}

	void update_sp(int SRC) // used for compute route for request which does not belong to current application
	{
		for (int i = 0; i < graph->adjL[SRC].size(); i++)
		{
			int dst = graph->adjL[SRC][i]->dst;
			double ld = graph->load[SRC][dst];
			double capacity = graph->adjL[SRC][i]->cap;

			double remain_bw = 0.0;
			if (capacity - ld == 0) remain_bw = DIVZERO; // link is almost full loaded
			else remain_bw = capacity - ld;
			
			double combine_load = ((double)alpha / (double)RMFACTOR) * adj_load[SRC][dst] + ((double)(RMFACTOR - alpha) / (double)RMFACTOR) * ld;
			// used for reinforcement mechanism

			double effect = 0.0;
			effect = combine_load / remain_bw;
	
			if (ld > capacity) continue;
			if (distance[SRC] + effect < distance[dst])
			{
				distance[dst] = distance[SRC] + effect;
				parent[dst] = SRC;
				latency[dst] = latency[SRC] + 1 / remain_bw;
			}
			else if (distance[SRC] + effect == distance[dst])
			{
				if (latency[dst] > latency[SRC] + 1 / remain_bw)
				{
					parent[dst] = SRC;
					latency[dst] = latency[SRC] + 1 / remain_bw;
				}
			}
		}
	}

	int find_min_sp() // used for compute route for request which does not belong to current application
	{
		double min_distance = INF;
		double min_latency = INF;
		int node = -1;
		for (set<int>::iterator it = node_set.begin(); it != node_set.end(); it++)
		{
			if (min_distance > distance[*it])
			{
				min_distance = distance[*it];
				min_latency = latency[*it];
				node = *it;
			}
			else if (min_distance == distance[*it])
			{
				if (min_latency >= latency[*it])
				{
					min_latency = latency[*it];
					node = *it;
				}
			}
		}
		return node;
	}

	void dijkstra_sp(Request* REQ) // used for compute route for request which does not belong to current application
	{ 
		bool found = 0;
		node_set.clear();
		for (int i = 0; i < graph->n; i++)
		{
			distance[i] = INF; // the influence of flow request to current flow on links
			parent[i] = INF;
			latency[i] = INF; // latency of flow request
			node_set.insert(i);
		}

		distance[REQ->src] = 0;
		parent[REQ->src] = -1;
		latency[REQ->src] = 0;
		update_sp(REQ->src);
		node_set.erase(REQ->src);
		while (!node_set.empty())
		{
			int next = find_min_sp();
			if (next == REQ->dst)
			{
				found = true; // the route finished
				record_path(REQ);
				return;
			}
			update_sp(next);
			node_set.erase(next);
		}
	}

	void propose(Request* REQ) // propose a route
	{
		// if the flow request belongs to this application, choose the best path for this flow
		if (REQ->type == id) dijkstra(REQ);
		// else, choose the path which has smallest effect on current flow
		else dijkstra_sp(REQ);
	}

	void evaluate(Request* REQ, vector<Voter*>& VOTERS)
	{
		// if the flow request belongs to this application, just calculate the latency of the path
		if (REQ->type == id)
		{
			for (int i = 0; i < VOTERS.size(); i++)
			{
				double delay = 0.0;
				for (int j = 0; j < VOTERS[i]->path_record.size() - 1; j++)
				{
					int tail = VOTERS[i]->path_record[j];
					int head = VOTERS[i]->path_record[j + 1];
					// delay = flow / (capacity-load), the smaller value is better
					double remain_bw = 0.0;
					if (graph->adjM[tail][head]->cap - graph->load[tail][head] <= 1.0) remain_bw = 1.0;
					else remain_bw = graph->adjM[tail][head]->cap - graph->load[tail][head];
					delay += 1.0 / remain_bw;
				}
				evaluate_list[i] = delay;
			}
		}

		// else, calculate the effect to current flow
		else
		{
			for (int i = 0; i < VOTERS.size(); i++)
			{
				double effect = DIVZERO;
				for (int j = 0; j < VOTERS[i]->path_record.size() - 1; j++)
				{
					int tail = VOTERS[i]->path_record[j];
					int head = VOTERS[i]->path_record[j + 1];
					//effect = mine load / (capacity-load), the smaller value is better
					double remain_bw = 0.0;
					if (graph->adjM[tail][head]->cap - graph->load[tail][head] <= 1.0) remain_bw = 1.0;
					else remain_bw = graph->adjM[tail][head]->cap - graph->load[tail][head];
					effect += adj_load[tail][head] * (1.0 / remain_bw);
				}
				evaluate_list[i] = effect;
			}
		}
	}

	~Qos() { ; }
};

#endif
