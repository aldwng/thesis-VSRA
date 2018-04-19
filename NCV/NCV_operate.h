#pragma once

#include "NCV_basic.h"
#include "NCV_info.h"

static string log_file = "logs.txt";

class Counter
{
protected:
	int num_voter;
	int num_candidate;
	vector<vector<double> > table;
	string choice = "range"; // default choice is range voting
	int range = 99; // default range of range voting is 99.
	int winner = -1;

	void method_range()
	{
		vector<double> max_ballot(num_voter, 0.0);
		for (int i = 0; i < num_voter; i++)
		{
			for (int j = 0; j < num_candidate; j++)
			{
				//cout << table[i][j] << " ";
				max_ballot[i] = max(max_ballot[j], table[i][j]);
			}
			//cout << "\n";
		}
		vector<int> sum_score(num_candidate, 0);
		for (int i = 0; i < num_voter; i++)
		{
			if (max_ballot[i] < 0.001) max_ballot[i] = 1.0;
			for (int j = 0; j < num_candidate; j++)
			{
				int tmp_ballot = (int)(range * (table[i][j] / max_ballot[i]));
				sum_score[j] += tmp_ballot;
			}
		}
		int tmp_max = -1;
		for (int i = 0; i < num_candidate; i++)
		{
			if (sum_score[i] > tmp_max)
			{
				tmp_max = sum_score[i];
				winner = i;
			}
		}
		cout << "\n";
	}

	void count_votes()
	{
		if (choice == "range") method_range();
		else return;
	}

public:
	Counter(vector<vector<double> > &TABLE)
	{
		num_voter = TABLE.size();
		if (num_voter)
		{
			num_candidate = TABLE[0].size();
			table.resize(num_voter, vector<double>(num_candidate, 0.0));
		}
		for (int i = 0; i < num_voter; i++)
		{
			for (int j = 0; j < num_candidate; j++)
			{
				table[i][j] = TABLE[i][j];
			}
		}
	}

	
	void set_choice(string CHOICE)
	{
		choice = CHOICE;
	} 

	void set_range(int RANGE)
	{
		range = RANGE;
	}

	int winner_out()
	{
		count_votes();
		return winner;
	}
};

class GraphAlgorithm
{
public:
	GraphAlgorithm(Topo* GRAPH)
	{
		graph = GRAPH;
		apsp_acquired = false;
	}

	vector<vector<Node*> > parents; // store the path
									// i->k->j: parents[i][j] = parents[k][j] = k, parents[i][k] = k
	vector<vector<int> > distances; // store the distances between two nodes
	vector<Node*> path;
	vector<Node*> host_path;

	vector<Node*>& shortest_path_acquire(int SRC, int DST, double BW)
	{
		if (!apsp_acquired) // shortest hop, graph could only be calculated once
		{
			allpair_shortest_path_acquire();
			apsp_acquired = true;
		}
		path.clear();
		// here to record the shortest path from floyd-warshall information
		int tmp_tail = DST;
		while (parents[SRC][tmp_tail] != graph->node_map[SRC])
		{
			path.push_back(graph->node_map[tmp_tail]);
			tmp_tail = parents[SRC][tmp_tail]->id;
		}
		path.push_back(graph->node_map[tmp_tail]);
		path.push_back(graph->node_map[SRC]);
		// here to reverse the vector path
		int left = 0;
		int right = path.size() - 1;
		while (left < right)
		{
			Node* swap = path[left];
			path[left] = path[right];
			path[right] = swap;
			left++;
			right--;
		}
		cout << "NCV GraphAlgorithm.shortest_path_acquire(): shortest path acquired" << "\n";
		return path;
	}

	vector<Node*>& host_path_acquire(vector<Node*> &PATH)
	{
		// first to acquire the shortest path, then host path can be acquired
		host_path.clear();
		int tmp_size = PATH.size();
		for (int i = 0; i < tmp_size; i++)
		{
			if (PATH[i]->type == 'h')
			{
				host_path.push_back(PATH[i]);
			}
		}
		return host_path;
	}

private:
	Topo* graph;
	bool apsp_acquired; // all-pair shortest paths

	void allpair_shortest_path_acquire()
	{
		distances.resize(graph->node_num, vector<int>(graph->node_num, (INT_MAX / 3)));
		parents.resize(graph->node_num, vector<Node*>(graph->node_num, NULL));
		for (int i = 0; i < graph->node_num; i++)
		{
			distances[i][i] = 0;
			parents[i][i] = graph->node_map[i];
		}
		for (int i = 0; i < graph->edge_num; i++)
		{
			distances[graph->edge_list[i]->src->id][graph->edge_list[i]->dst->id] = 1;
			parents[graph->edge_list[i]->src->id][graph->edge_list[i]->dst->id] = graph->edge_list[i]->src;
		}
		// here is Floyd-Warshall algorithm
		for (int k = 0; k < graph->node_num; k++)
		{
			for (int i = 0; i < graph->node_num; i++)
			{
				for (int j = 0; j < graph->node_num; j++)
				{
					if (distances[i][j] > distances[i][k] + distances[k][j])
					{
						distances[i][j] = distances[i][k] + distances[k][j];
						parents[i][j] = parents[k][j];
					}
				}
			}
		}
		// DP(Floyd-Warshall) ends here
	}
};

class Entity
{
public:
	int id;
	Topo* graph;
	Scheme* prop;
	GraphAlgorithm* graph_algorithm;
	char type;

	virtual Scheme* proposal_generate(Traffic* PARAM_TRAFFIC_REQ) = 0;

	virtual double evaluate(Traffic* PARAM_REQ, Scheme* PARAM_SCHEME) = 0;

	virtual ~Entity() {}

	bool link_capable(Traffic* REQ, Scheme* SCHEME)
	{
		cout << "NCV Entity.link_capable(): checking link capability starts" << "\n";
		int hosts_size = SCHEME->host_chain.size();
		if (hosts_size == 0) return true;
		int hop_cnt = 0;
		int base_dist = graph_algorithm->distances[REQ->src][REQ->dst];
		for (int i = 0; i <= hosts_size; i++)
		{
			Node* src_tmp = NULL;
			Node* flag = NULL;
			if (i == 0)
			{
				if (SCHEME->host_chain[i]->id != REQ->src)
				{
					src_tmp = graph->node_map[REQ->src];
					flag = SCHEME->host_chain[i];
				}
				cout << "NCV Entity.link_capable(): path from the src to the first host" << "\n";
			}
			else if (i == hosts_size)
			{
				if (SCHEME->host_chain[i - 1]->id != REQ->dst)
				{
					src_tmp = SCHEME->host_chain[i - 1];
					flag = graph->node_map[REQ->dst];
				}
				cout << "NCV Entity.link_capable(): path from the last host to the dst" << "\n";
			}
			else
			{
				src_tmp = SCHEME->host_chain[i - 1];
				flag = SCHEME->host_chain[i];
				cout << "NCV Entity.link_capable(): path between two other hosts in the chain" << "\n";
			}
			while (flag != NULL && src_tmp != NULL && flag->id != src_tmp->id)
			{
				Node* tmp = graph_algorithm->parents[src_tmp->id][flag->id];
				hop_cnt += 1;
				if (graph->load_matrix[tmp->id][flag->id] + REQ->rate > graph->adj_matrix[tmp->id][flag->id]->cap)
				{
					cout << "NCV ERROR Entity.link_capable(): link is not sufficient" << "\n";
					return false;
				}
				else
				{
					flag = tmp;
				}
			}
			cout << "NCV Entity.link_capable(): path between two hosts checke" << "\n";
		}
		if (type == 'l')
		{
			if (hop_cnt > 5 * base_dist) return false;
		}
		if (type == 'u')
		{
			if (hop_cnt > 6 * base_dist) return false;
		}
		return true;
	}
};
