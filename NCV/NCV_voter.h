#pragma once

#include "NCV_operate.h"

/*
* GSP: greedy on shortest path
*/
class GSP : public Entity 
{
public:
	GSP(Topo* GRAPH, GraphAlgorithm* GA)
	{
		id = 1;
		graph = GRAPH;
		prop = NULL;
		graph_algorithm = GA;
		//graph_algorithm->allpair_shortest_path();
		type = 's';
	}

	Scheme* proposal_generate(Traffic* REQ)
	{
		cout << "NCV GSP.proposal_generate(): starts to generate a proposal" << "\n";
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
		if (graph == NULL) return prop;
		prop = new Scheme(REQ->rate);
		set_constraint();
		node_sp = graph_algorithm->shortest_path_acquire(REQ->src, REQ->dst, REQ->rate); // to get shortest path of the request
		host_sp = graph_algorithm->host_path_acquire(node_sp); // get the set of the hosts which are along the shortest path
		cout << "NCV GSP.proposal_generate(): shortest path acquired" << "\n";

		dp_svp(REQ);
		cout << "NCV GSP.proposal_generate(): reuse factor maximized" << "\n";

		complete_chain(REQ); // here is to complete service chain with services not on reuseable VM
		cout << "NCV GSP.proposal_generate(): a service chain completed" << "\n";

		prop_generate(REQ);
		cout << "NCV GSP.proposal_generate(): a scheme completed" << "\n";

		if (prop != NULL && !link_capable(REQ, prop))
		{
			delete prop;
			prop = NULL;
		}
		cout << "NCV GSP.proposal_generate(): link sufficiency checked" << "\n";
		return prop;
	}

	double evaluate(Traffic* REQ, Scheme* SCHEME)
	{
		if (SCHEME == NULL) return 0.0;
		int hop_cnt = 0;
		int host_size = SCHEME->host_chain.size();
		cout << "NCV GSP.evaluate(): starts evaluating" << "\n";
		hop_cnt += graph_algorithm->distances[REQ->src][SCHEME->host_chain[0]->id];
		for (int i = 0; i < host_size - 1; i++)
		{
			hop_cnt += graph_algorithm->distances[SCHEME->host_chain[i]->id][SCHEME->host_chain[i + 1]->id];
		}
		hop_cnt += graph_algorithm->distances[SCHEME->host_chain[host_size - 1]->id][REQ->dst];
		cout << "NCV GSP.evaluate(): evaluation finished" << "\n";
		if (hop_cnt < 1) return 10.0;
		else return (10.0 / (double)hop_cnt);
	}

	~GSP()
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
	}

private:
	vector<int> host_free;
	vector<Node*> host_sp; // shortest path
	vector<Node*> node_sp; // shortest path
	vector<Node*> node_chain; // store hosts running vm with specific service

	void set_constraint()
	{
		host_free.clear();
		int host_size = graph->host_num;
		host_free.resize(host_size);
		for (int i = 0; i < host_size; i++)
		{
			host_free[i] = graph->host_list[i]->host->vol_free;
		}
	}

	void prop_generate(Traffic* REQ)
	{
		int functions_size = node_chain.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (prop != NULL)
				{
					delete prop;
					prop = NULL;
					cout << "NCV ERROR GSP.prop_generate(): there is no sufficient host" << endl;
					return;
				}
			}
			else
			{
				VirtualMachine* vm_tmp = node_chain[i]->host->vm_reuse(REQ->function_chain[i], REQ->rate);
				if (vm_tmp == NULL)
				{
					vm_tmp = node_chain[i]->host->vm_add(REQ->function_chain[i], REQ->rate);
				}
				if (vm_tmp == NULL)
				{
					if (prop != NULL)
					{
						delete prop;
						prop = NULL;
						cout << "NCV ERROR GSP.prop_generate(): there is no sufficient VM" << endl;
						return;
					}
				}
				prop->service_add(node_chain[i], vm_tmp);
			}
		}
	}

	void complete_chain(Traffic* REQ)
	{
		int functions_size = REQ->function_chain.size();
		int path_size = node_sp.size();
		int hosts_size = host_sp.size();
		for (auto n : node_chain)
		{
			if (n != NULL) cout << "Host ID: " << n->id << " ";
		}
		cout << "\n";
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (i == 0)
				{
					if (hosts_size == 0) // find the nearest host to the src-node which can reuse or then add the vm
					{
						node_chain[i] = near_reuse_or_add(node_sp[0], REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "NCV ERROR GSP.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
					else
					{
						Node* host_tmp = NULL;
						for (int j = 1; j < functions_size; j++)
						{
							if (node_chain[j] != NULL)
							{
								host_tmp = node_chain[j];
								break;
							}
						}
						if (host_tmp == NULL) host_tmp = host_sp[0];
						node_chain[i] = near_reuse_or_add(host_tmp, REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "NCV ERROR GSP.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
				}
				else
				{
					if (node_chain[i - 1] == NULL) return;
					node_chain[i] = near_reuse_or_add(node_chain[i - 1], REQ->function_chain[i], REQ->rate);
					if (node_chain[i] == NULL)
					{
						cout << "NCV ERROR GSP.complete_chain(): there is no sufficietn node" << "\n";
						return;
					}
				}
			}
			else
			{
				if (!node_chain[i]->host->vm_reusable(REQ->function_chain[i], REQ->rate))
				{
					cout << "NCV ERROR GSP.complete_chain(): dp_SVP has some problems" << "\n";
				}
			}
		}
	}

	void dp_svp(Traffic* REQ) // here is so-called SVP, actually is dp
	{
		int hosts_size = host_sp.size();
		int functions_size = REQ->function_chain.size();
		node_chain.clear();
		node_chain.resize(functions_size, NULL);
		vector<vector<int> > dp_reuse_factor(hosts_size, vector<int>(functions_size + 1, 0));
		vector<vector<vector<int> > > host_reuse_factor(hosts_size,
			vector<vector<int> >(functions_size, vector<int>(functions_size, 0)));
		// the second index means the start index of the function chain
		vector<vector<bool> > service_reusable(hosts_size, vector<bool>(functions_size, false));
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = 0; j < functions_size; j++)
			{
				service_reusable[i][j] = host_sp[i]->host->vm_reusable(REQ->function_chain[j], REQ->rate);
			}
		}
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = functions_size - 1; j >= 0; j--)
			{
				int tmp_cnt = 0;
				for (int k = j; k >= 0; k--)
				{
					if (service_reusable[i][k])
					{
						tmp_cnt += 1;
					}
					host_reuse_factor[i][k][j] = tmp_cnt;
				}
			}
		}
		for (int i = 0; i < functions_size; i++)
		{
			if (service_reusable[0][i])
			{
				if (i == 0) dp_reuse_factor[0][i + 1] = 1;
				else dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i] += 1;
				node_chain[i] = host_sp[0];
			}
			else
			{
				if (i != 0) dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i];
			}
		}
		for (int i = 1; i < hosts_size; i++)
		{
			dp_reuse_factor[i][0] = host_reuse_factor[i - 1][0][functions_size - 1];
			for (int j = 1; j < functions_size; j++)
			{
				for (int k = 1; k <= j; k++)
				{
					if (dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j] > dp_reuse_factor[i][j])
					{
						dp_reuse_factor[i][j] = dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j];
						for (int p = k; p <= j; p++)
						{
							if (service_reusable[i][p])
							{
								node_chain[p] = host_sp[i];
							}
						}
					}
				}
			}
		}
		// SVP(dp) ends here
	}

	Node* near_reuse_or_add(Node* NODE, char FUNCTION, double RATE)
	{
		Node* node_ans = NULL;
		int dist = INT_MAX / 3;
		int hosts_size = graph->host_num;
		int idx = -1;
		for (int i = 0; i < hosts_size; i++)
		{
			if (graph_algorithm->distances[NODE->id][graph->host_list[i]->id] < dist)
			{
				if (graph->host_list[i]->host->vm_reusable(FUNCTION, RATE))
				{
					node_ans = graph->host_list[i];
					dist = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
					idx = -1;
				}
				else
				{
					if (host_free[i] > 0 && graph->host_list[i]->host->vm_addable())
					{
						node_ans = graph->host_list[i];
						dist = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
						idx = i;
					}
				}
			}
		}
		if (idx != -1) host_free[idx] -= 1;
		return node_ans;
	}
};

/*
* CSP: constrained greedy on shortest path
* host can only start limited numbers of new VMs
*/
class CSP : public Entity
{
public:
	CSP(Topo* GRAPH, int LIMIT, GraphAlgorithm* GA)
	{
		id = 1;
		graph = GRAPH;
		prop = NULL;
		graph_algorithm = GA;
		//graph_algorithm->allpair_shortest_path(); // to get all-pair shortest paths
		limit = LIMIT;
		type = 's';
	}

	Scheme* proposal_generate(Traffic* REQ)
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
		if (graph == NULL) return prop;
		prop = new Scheme(REQ->rate);
		set_constraint();
		node_sp = graph_algorithm->shortest_path_acquire(REQ->src, REQ->dst, REQ->rate); // to get shortest path of the request
		host_sp = graph_algorithm->host_path_acquire(node_sp); // get the set of the hosts which is on the path above

		dp_svp(REQ);

		complete_chain(REQ); // here is to complete service chain with services not on reuseable vm

		prop_generate(REQ);

		if (prop != NULL && !link_capable(REQ, prop))
		{
			delete prop;
			prop = NULL;
		}
		return prop;
	}

	double evaluate(Traffic* REQ, Scheme* SCHEME)
	{
		if (SCHEME == NULL) return 0.0;
		double hop_cnt = 0.0;
		int host_size = SCHEME->host_chain.size();
		// firstly count the hops of the path
		hop_cnt += graph_algorithm->distances[REQ->src][SCHEME->host_chain[0]->id];
		for (int i = 0; i < host_size - 1; i++)
		{
			hop_cnt += graph_algorithm->distances[SCHEME->host_chain[i]->id][SCHEME->host_chain[i + 1]->id];
			limit_map[SCHEME->host_chain[i]] = limit;
		}
		hop_cnt += graph_algorithm->distances[SCHEME->host_chain[host_size - 1]->id][REQ->dst];
		limit_map[SCHEME->host_chain[host_size - 1]] = limit;
		int function_size = SCHEME->service_chain.size();
		for (int i = 0; i < function_size; i++)
		{
			limit_map[SCHEME->service_chain[i].first] -= 1;
		}
		// secondly count the number of new VM of this scheme
		for (int i = 0; i < host_size; i++)
		{
			if (limit_map[SCHEME->host_chain[i]] < 0)
			{
				hop_cnt += 2 * (0 - limit_map[SCHEME->host_chain[i]]);
			}
		}
		if (hop_cnt < 1) return 10.0;
		else return (10.0 / hop_cnt);
	}

	~CSP()
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
	}

private:
	int limit;
	unordered_map<Node*, int> limit_map;
	vector<int> host_free;
	vector<Node*> host_sp; // shortest path
	vector<Node*> node_sp; // shortest path
	vector<Node*> node_chain; // store hosts running vm with specific service

	void set_constraint()
	{
		host_free.clear();
		int host_size = graph->host_num;
		host_free.resize(host_size);
		for (int i = 0; i < host_size; i++)
		{
			limit_map[graph->host_list[i]] = limit;
			host_free[i] = graph->host_list[i]->host->vol_free;
		}
	}

	void prop_generate(Traffic* REQ)
	{
		int functions_size = node_chain.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (prop != NULL)
				{
					delete prop;
					prop = NULL;
					cout << "NCV ERROR CSP.prop_generate(): there is no sufficient proposal" << "\n";
					return;
				}
			}
			else
			{
				VirtualMachine* vm_tmp = node_chain[i]->host->vm_reuse(REQ->function_chain[i], REQ->rate);
				if (vm_tmp == NULL)
				{
					vm_tmp = node_chain[i]->host->vm_add(REQ->function_chain[i], REQ->rate);
				}
				if (vm_tmp == NULL)
				{
					if (prop != NULL)
					{
						delete prop;
						prop = NULL;
						cout << "NCV ERROR CSP.prop_generate(): there is no sufficient proposal" << "\n";
						return;
					}
				}
				prop->service_add(node_chain[i], vm_tmp);
			}
		}
	}

	void complete_chain(Traffic* REQ)
	{
		int functions_size = REQ->function_chain.size();
		int path_size = node_sp.size();
		int hosts_size = host_sp.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (i == 0)
				{
					if (hosts_size == 0) // find the nearest host to the src-node which can reuse or then add the vm
					{
						node_chain[i] = near_reuse_or_add(node_sp[0], REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "ERROR CSP:(complete_chain()) there is no sufficietn node" << "\n";
							return;
						}
					}
					else {
						Node* host_tmp = NULL;
						for (int j = 1; j < functions_size; j++)
						{
							if (node_chain[j] != NULL)
							{
								host_tmp = node_chain[j];
								break;
							}
						}
						if (host_tmp == NULL) host_tmp = host_sp[0];
						node_chain[i] = near_reuse_or_add(host_tmp, REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "NCV ERROR CSP.complete_chain(): there is no sufficient node" << "\n";
							return;
						}
					}
				}
				else
				{
					if (node_chain[i - 1]) return;
					node_chain[i] = near_reuse_or_add(node_chain[i - 1], REQ->function_chain[i], REQ->rate);
					if (node_chain[i] == NULL)
					{
						cout << "NCV ERROR GSP.complete_chain(): there is no sufficietn node" << "\n";
						return;
					}
				}
			}
		}
	}

	void dp_svp(Traffic* REQ)
	{
		int hosts_size = host_sp.size();
		int functions_size = REQ->function_chain.size();
		node_chain.clear();
		node_chain.resize(functions_size, NULL);
		vector<vector<int> > dp_reuse_factor(hosts_size, vector<int>(functions_size + 1, 0));
		vector<vector<vector<int> > > host_reuse_factor(hosts_size,
			vector<vector<int> >(functions_size, vector<int>(functions_size, 0)));
		// the second index means the start index of the function chain
		vector<vector<bool> > service_reusable(hosts_size, vector<bool>(functions_size, false));
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = 0; j < functions_size; j++)
			{
				service_reusable[i][j] = host_sp[i]->host->vm_reusable(REQ->function_chain[j], REQ->rate);
			}
		}
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = functions_size - 1; j >= 0; j--)
			{
				int tmp_cnt = 0;
				for (int k = j; k >= 0; k--)
				{
					if (service_reusable[i][k])
					{
						tmp_cnt += 1;
					}
					host_reuse_factor[i][k][j] = tmp_cnt;
				}
			}
		}
		for (int i = 0; i < functions_size; i++)
		{
			if (service_reusable[0][i])
			{
				if (i == 0) dp_reuse_factor[0][i + 1] = 1;
				else dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i] += 1;
				node_chain[i] = host_sp[0];
			}
			else
			{
				if (i != 0) dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i];
			}
		}
		for (int i = 1; i < hosts_size; i++)
		{
			dp_reuse_factor[i][0] = host_reuse_factor[i - 1][0][functions_size - 1];
			for (int j = 1; j < functions_size; j++)
			{
				for (int k = 1; k <= j; k++)
				{
					if (dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j] > dp_reuse_factor[i][j]) {
						dp_reuse_factor[i][j] = dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j];
						for (int p = k; p <= j; p++)
						{
							if (service_reusable[i][p])
							{
								node_chain[p] = host_sp[i];
							}
						}
					}
				}
			}
		}
		// SVP(dp) ends here
	}

	Node* near_reuse_or_add(Node* NODE, char FUNCTION, double RATE)
	{
		Node* node_ans = NULL;
		int dist_tmp = INT_MAX;
		int hosts_size = graph->host_num;
		int idx = -1;
		for (int i = 0; i < hosts_size; i++)
		{
			if (graph_algorithm->distances[NODE->id][graph->host_list[i]->id] < dist_tmp)
			{
				if (graph->host_list[i]->host->vm_reusable(FUNCTION, RATE))
				{
					node_ans = graph->host_list[i];
					dist_tmp = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
					idx = -1;
				}
				else
				{
					if (graph->host_list[i]->host->vm_addable() && limit_map[graph->host_list[i]] > 0
						&& host_free[i] > 0)
					{
						node_ans = graph->host_list[i];
						dist_tmp = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
						idx = i;
					}
				}
			}
		}
		if (idx != -1)
		{
			limit_map[graph->host_list[idx]] -= 1;
			host_free[idx] -= 1;
		}
		return node_ans;
	}
};

/*
* host load balance
* start new VM for the consideration of load balance of host
*/
class HLB : public Entity
{
public:
	HLB(Topo* GRAPH, GraphAlgorithm* GA)
	{
		id = 1;
		graph = GRAPH;
		prop = NULL;
		graph_algorithm = GA;
		//graph_algorithm->allpair_shortest_path();
		type = 's';
	}

	Scheme* proposal_generate(Traffic* REQ)
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
		if (graph == NULL) return prop;
		prop = new Scheme(REQ->rate);
		set_constraint();
		node_sp = graph_algorithm->shortest_path_acquire(REQ->src, REQ->dst, REQ->rate); // to get shortest path of the request
		host_sp = graph_algorithm->host_path_acquire(node_sp); // get the set of the hosts which is on the path above

		dp_svp(REQ);

		complete_chain(REQ); // here is to complete service chain with services not on reuseable vm

		prop_generate(REQ);

		if (prop != NULL && !link_capable(REQ, prop))
		{
			delete prop;
			prop = NULL;
		}
		return prop;
	}

	double evaluate(Traffic* REQ, Scheme* SCHEME)
	{
		if (SCHEME == NULL) return 0.0;
		double hop_cnt = 0.0;
		int host_size = SCHEME->host_chain.size();
		unordered_map<Node*, int> new_vm_map;
		// firstly count the hops of the path
		hop_cnt += graph_algorithm->distances[REQ->src][SCHEME->host_chain[0]->id];
		for (int i = 0; i < host_size - 1; i++)
		{
			hop_cnt += graph_algorithm->distances[SCHEME->host_chain[i]->id][SCHEME->host_chain[i + 1]->id];
			new_vm_map[SCHEME->host_chain[i]] = 0;
		}
		hop_cnt += graph_algorithm->distances[SCHEME->host_chain[host_size - 1]->id][REQ->dst];
		new_vm_map[SCHEME->host_chain[host_size - 1]] = 0;
		int function_size = SCHEME->service_chain.size();
		for (int i = 0; i < function_size; i++)
		{
			if (SCHEME->service_chain[i].second->service_count < 1)
			{
				new_vm_map[SCHEME->service_chain[i].first] += 1;
			}
		}
		// secondly count the minimal free vol of hosts
		int free_vol_min = INT_MAX; // should be as large as possible
		for (int i = 0; i < host_size; i++)
		{
			if (SCHEME->host_chain[i]->host->vol_free - new_vm_map[SCHEME->host_chain[i]] < free_vol_min)
			{
				free_vol_min = SCHEME->host_chain[i]->host->vol_free - new_vm_map[SCHEME->host_chain[i]];
				// warning: the number of new VMs in a host may exceed vol_free value
			}
		}
		if (free_vol_min < 0.0) return 0.0;
		else if (hop_cnt < 1) return 10.0 * (double)free_vol_min;
		else return (10.0 * (double)free_vol_min) / hop_cnt;
	}

	~HLB() {
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
	}

private:
	vector<int> host_free;
	vector<Node*> host_sp; // shortest path
	vector<Node*> node_sp; // shortest path
	vector<Node*> node_chain; // store hosts running vm with specific service

	void set_constraint()
	{
		host_free.clear();
		int host_size = graph->host_num;
		host_free.resize(host_size);
		for (int i = 0; i < host_size; i++)
		{
			host_free[i] = graph->host_list[i]->host->vol_free;
		}
	}

	void prop_generate(Traffic* REQ)
	{
		int functions_size = node_chain.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (prop != NULL)
				{
					delete prop;
					prop = NULL;
					cout << "NCV ERROR HLB.prop_generate(): there is no sufficient proposal" << "\n";
					return;
				}
			}
			else {
				VirtualMachine* vm_tmp = node_chain[i]->host->vm_reuse(REQ->function_chain[i], REQ->rate);
				if (vm_tmp == NULL)
				{
					vm_tmp = node_chain[i]->host->vm_add(REQ->function_chain[i], REQ->rate);
				}
				if (vm_tmp == NULL)
				{
					if (prop != NULL)
					{
						delete prop;
						prop = NULL;
						cout << "NCV ERROR HLB.prop_generate(): there is no sufficient proposal" << "\n";
						return;
					}
				}
				prop->service_add(node_chain[i], vm_tmp);
			}
		}
	}

	void complete_chain(Traffic* REQ)
	{
		int functions_size = REQ->function_chain.size();
		int path_size = node_sp.size();
		int hosts_size = host_sp.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (node_chain[i] == NULL)
			{
				if (i == 0)
				{
					if (hosts_size == 0) // find the nearest host to the src-node which can reuse or then add the vm
					{
						node_chain[i] = near_reuse_or_add(node_sp[0], REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "NCV ERROR HLB.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
					else
					{
						Node* host_tmp = NULL;
						for (int j = 1; j < functions_size; j++)
						{
							if (node_chain[j] != NULL)
							{
								host_tmp = node_chain[j];
								break;
							}
						}
						if (host_tmp == NULL) host_tmp = host_sp[0];
						node_chain[i] = near_reuse_or_add(host_tmp, REQ->function_chain[i], REQ->rate);
						if (node_chain[i] == NULL)
						{
							cout << "NCV ERROR HLB.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
				}
				else
				{
					if (node_chain[i - 1] == NULL) return;
					node_chain[i] = near_reuse_or_add(node_chain[i - 1], REQ->function_chain[i], REQ->rate);
					if (node_chain[i] == NULL)
					{
						cout << "NCV ERROR HLB.complete_chain(): there is no sufficietn node" << "\n";
						return;
					}
				}
			}
		}
	}

	void dp_svp(Traffic* REQ)
	{
		int hosts_size = host_sp.size();
		int functions_size = REQ->function_chain.size();
		node_chain.clear();
		node_chain.resize(functions_size, NULL);
		vector<vector<int> > dp_reuse_factor(hosts_size, vector<int>(functions_size + 1, 0));
		vector<vector<vector<int> > > host_reuse_factor(hosts_size,
			vector<vector<int> >(functions_size, vector<int>(functions_size, 0)));
		//the second index means the start index of the function chain.
		vector<vector<bool> > service_reusable(hosts_size, vector<bool>(functions_size, false));
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = 0; j < functions_size; j++)
			{
				service_reusable[i][j] = host_sp[i]->host->vm_reusable(REQ->function_chain[j], REQ->rate);
			}
		}
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = functions_size - 1; j >= 0; j--)
			{
				int tmp_cnt = 0;
				for (int k = j; k >= 0; k--)
				{
					if (service_reusable[i][k])
					{
						tmp_cnt += 1;
					}
					host_reuse_factor[i][k][j] = tmp_cnt;
				}
			}
		}
		for (int i = 0; i < functions_size; i++)
		{
			if (service_reusable[0][i])
			{
				if (i == 0) dp_reuse_factor[0][i + 1] = 1;
				else dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i] += 1;
				node_chain[i] = host_sp[0];
			}
			else
			{
				if (i != 0) dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i];
			}
		}
		for (int i = 1; i < hosts_size; i++)
		{
			dp_reuse_factor[i][0] = host_reuse_factor[i - 1][0][functions_size - 1];
			for (int j = 1; j < functions_size; j++)
			{
				for (int k = 1; k <= j; k++)
				{
					if (dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j] > dp_reuse_factor[i][j])
					{
						dp_reuse_factor[i][j] = dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j];
						for (int p = k; p <= j; p++)
						{
							if (service_reusable[i][p])
							{
								node_chain[p] = host_sp[i];
							}
						}
					}
				}
			}
		}
		// SVP(dp) ends here
	}

	Node* near_reuse_or_add(Node* NODE, char FUNCTION, double RATE)
	{
		Node* node_ans = NULL;
		int dist_tmp = INT_MAX;
		int hosts_size = graph->host_num;
		int idx = -1;
		for (int i = 0; i < hosts_size; i++)
		{
			if (graph_algorithm->distances[NODE->id][graph->host_list[i]->id] < dist_tmp && graph->host_list[i]->host->vm_reusable(FUNCTION, RATE))
			{
				node_ans = graph->host_list[i];
				dist_tmp = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
				idx = -1;
			}
		}
		if (node_ans == NULL)
		{
			int free_vols = 0;
			for (int i = 0; i < hosts_size; i++)
			{
				if (host_free[i] > free_vols)
				{
					node_ans = graph->host_list[i];
					free_vols = host_free[i];
					idx = i;
				}
			}
		}
		if (idx != -1) host_free[idx] -= 1;
		return node_ans;
	}
};

/*
* GVR: greedy on VM reuse factor
*/
class GVR : public Entity
{
public:
	GVR(Topo* GRAPH, GraphAlgorithm* GA)
	{
		id = 4;
		graph = GRAPH;
		prop = NULL;
		graph_algorithm = GA;
		type = 'l';
	}

	Scheme* proposal_generate(Traffic* REQ)
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
		if (graph == NULL) return prop;
		host_free.clear();
		host_free.resize(graph->host_num);
		for (int i = 0; i < graph->host_num; i++)
		{
			host_free[i] = graph->host_list[i]->host->vol_free;
		}
		prop = new Scheme(REQ->rate);
		Node* cur_host = NULL;
		VirtualMachine* cur_vm = NULL;
		char cur_service;
		int functions_size = REQ->function_chain.size();
		for (int i = 0; i < functions_size; i++)
		{
			cur_service = REQ->function_chain[i];
			if (cur_host == NULL)
			{
				cur_host = host_reuse(cur_service, REQ->rate);
				if (cur_host == NULL)
				{
					cur_host = host_new_vm();
					if (cur_host == NULL)
					{
						cout << "NCV ERROR GVR.proposal_generate(): cannot find a complete service chain" << "\n";
						delete prop;
						prop = NULL;
						return prop;
					}
					else
					{
						cur_vm = cur_host->host->vm_add(cur_service, REQ->rate);
					}
				}
				else
				{
					cur_vm = cur_host->host->vm_reuse(cur_service, REQ->rate);
				}
			}
			else
			{
				if (!cur_host->host->vm_reusable(cur_service, REQ->rate))
				{
					Node* tmp_host = host_reuse(cur_service, REQ->rate);
					if (tmp_host == NULL) // can not find another host which has a reusable vm
					{
						if (!cur_host->host->vm_addable()) // cur_host can not open a new vm
						{
							tmp_host = host_new_vm(); // find another host which can open a new vm
							if (tmp_host == NULL)
							{
								cout << "NCV ERROR GVR.proposal_generate(): cannot find a complete service chain" << "\n";
								delete prop;
								prop = NULL;
								return prop;
							}
							else // host which can open a new vm found
							{
								cur_host = tmp_host;
								cur_vm = cur_host->host->vm_add(cur_service, REQ->rate);
							}
						}
						else // cur_host can open a new vm
						{
							cur_vm = cur_host->host->vm_add(cur_service, REQ->rate);
						}
					}
					else // another host which has a reusable vm found
					{
						cur_host = tmp_host;
						cur_vm = tmp_host->host->vm_reuse(cur_service, REQ->rate);
					}
				}
				else // cur_host has a reusable vm
				{
					cur_vm = cur_host->host->vm_reuse(cur_service, REQ->rate);
				}
			}
			prop->service_add(cur_host, cur_vm);
		}
		if (prop != NULL && !link_capable(REQ, prop))
		{
			delete prop;
			prop = NULL;
		}
		return prop;
	}

	double evaluate(Traffic* REQ, Scheme* SCHEME)
	{
		if (SCHEME == NULL) return 0.0;
		int reuse_cnt = 0;
		int function_size = SCHEME->service_chain.size();
		for (int i = 0; i < function_size; i++)
		{
			if (SCHEME->service_chain[i].second->service_count > 0)
			{
				reuse_cnt += 1;
			}
		}
		return (double)reuse_cnt;
	}

	~GVR()
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
	}

private:
	vector<int> host_free;

	Node* host_reuse(char SERVICE, double BW) // when to find a host to reuse a service, find the VM with least computing resource deployed
	{
		Node* tmp_host = NULL;
		double max_cap = 0.0;
		for (int i = 0; i < graph->host_num; i++)
		{
			Host* flag = graph->host_list[i]->host;
			if (flag->vm_reusable(SERVICE, BW))
			{
				if (tmp_host == NULL)
				{
					tmp_host = graph->host_list[i];
					max_cap = (flag->vm_reuse(SERVICE, BW))->rcs_free;
					cout << "NCV GVR:.host_reuse(): a reusable VM" << "\n";
				}
				else
				{
					if (flag->vm_reuse(SERVICE, BW)->rcs_free > max_cap)
					{
						max_cap = (flag->vm_reuse(SERVICE, BW))->rcs_free;
						tmp_host = graph->host_list[i];
					}
				}
			}
		}
		return tmp_host;
	}

	Node* host_new_vm() // when to start a new VM, find the host with maximum number of free volumes
	{
		Node* tmp_host = NULL;
		int max_vol = 0;
		int index = -1;
		for (int i = 0; i < graph->host_num; i++)
		{
			if (host_free[i] > max_vol)
			{
				tmp_host = graph->host_list[i];
				max_vol = host_free[i];
				index = i;
			}
		}
		if (tmp_host != NULL) host_free[index] -= 1;
		return tmp_host;
	}
};

/*
* JVP: joint VNF placement and path selection
*/
class JVP : public Entity
{
public:
	JVP(Topo* GRAPH, GraphAlgorithm* GA)
	{
		id = 6;
		graph = GRAPH;
		prop = NULL;
		graph_algorithm = GA;
		type = 'j';
	}

	Scheme* proposal_generate(Traffic* REQ)
	{
		if (prop != NULL)
		{
			delete prop;
			prop = NULL;
		}
		if (graph == NULL) return prop;
		prop = new Scheme(REQ->rate);
		set_constraint();
		node_sp = graph_algorithm->shortest_path_acquire(REQ->src, REQ->dst, REQ->rate); // to get distances information first
		host_sp = graph_algorithm->host_path_acquire(node_sp);

		reuse_factor_acquire(REQ); // to get x*(reuse_factor) and l(x*)(path_length)
		cout << "NCV JVP.proposal_generate(): x* and l(x*) acquired" << "\n";

		path_extend(REQ); // complete the path-extension
		cout << "NCV JVP.proposal_generate(): path extension done" << "\n";

		chain_complete(REQ);
		cout << "NCV JVP.proposal_generate(): service chain completed" << "\n";

		prop_generate(REQ);
		cout << "NCV JVP.proposal_generate()) a scheme completed" << "\n";

		if (prop != NULL && !link_capable(REQ, prop))
		{
			delete prop;
			prop = NULL;
		}
		cout << "NCV JVP.proposal_generate()) link sufficiency checked" << "\n";
		return prop;
	}

	double evaluate(Traffic* REQ, Scheme* SCHEME)
	{
		return 100.00;
	}

	~JVP()
	{
		if (prop != NULL) delete prop;
		prop = NULL;
	}

private:
	/*
	* MP problem to acquire x* and l(x*).
	*/

	int reuse_factor;
	int path_length;
	int reusable_nf;
	vector<char> reusable_chain;

	vector<vector<Node*> > u_suffix_j;

	void reuse_factor_acquire(Traffic* REQ)
	{
		reusable_nf = nf_reuse_count(REQ);
		int req_n = 0;
		for (int i = 0; i <= reusable_nf; i++)
		{
			int length = (int)path_length_acquire(REQ, i);
			int n = request_count(REQ, i, length);
			if (n > req_n)
			{
				reuse_factor = i;
				path_length = length;
			}
		}
		path_stretch(REQ);
	}

	double path_length_acquire(Traffic* REQ, int RF)
	{
		double path_length_ans = 0.0;
		if (RF == 0)
		{
			double l_suffix_tri = l_suffix_tri_calculate(REQ);
			int chain_length = REQ->function_chain.size();
			int n_suffix_e_src_dst = 0;
			Node* flag = graph->node_map[REQ->dst];
			while (flag->id != REQ->src)
			{
				if (flag->type == 'h') n_suffix_e_src_dst += flag->host->vol_free;
				flag = graph_algorithm->parents[REQ->src][flag->id];
			}
			if (flag->type == 'h') n_suffix_e_src_dst += flag->host->vol_free;
			path_length_ans = (double)graph_algorithm->distances[REQ->src][REQ->dst] + l_suffix_tri * max(0.0, (double)(chain_length - n_suffix_e_src_dst));
		}
		else
		{
			double l_suffix_r = l_suffix_r_calculate();
			double l_suffix_tri = l_suffix_tri_calculate(REQ);
			double n_suffix_e = n_suffix_e_calculate(REQ);
			int chain_length = REQ->function_chain.size();
			path_length_ans = (double)(RF + 1) * l_suffix_r + l_suffix_tri * max(0.0, (double)(chain_length - RF - (RF + 1) * n_suffix_e));
		}
		return path_length_ans;
	}

	int nf_reuse_count(Traffic* REQ)
	{
		int ans = 0;
		int chain_length = REQ->function_chain.size();
		reusable_chain.clear();
		u_suffix_j.clear();
		for (int i = 0; i < chain_length; i++)
		{
			for (int j = 0; j < graph->host_num; j++)
			{
				if (graph->host_list[j]->host->vm_reusable(REQ->function_chain[i], REQ->rate))
				{
					ans += 1;
					reusable_chain.push_back(REQ->function_chain[i]);
					break;
				}
			}
		}
		u_suffix_j.resize(ans + 2, vector<Node*>(0));
		u_suffix_j[0].push_back(graph->node_map[REQ->src]);
		for (int i = 0; i < ans; i++)
		{
			for (int j = 0; j < graph->host_num; j++)
			{
				if (graph->host_list[j]->host->vm_reusable(reusable_chain[i], REQ->rate))
				{
					u_suffix_j[i + 1].push_back(graph->host_list[j]);
				}
			}
		}
		u_suffix_j[ans + 1].push_back(graph->node_map[REQ->dst]);
		return ans;
	}

	int request_count(Traffic* REQ, int RF, int PL)
	{
		int ans = 0;
		int c_suffix_em = 0; // remained empty vm
		double c_suffix_l = graph->overall_bandwidth; //remained bandwidth resource 
		for (int i = 0; i < graph->host_num; i++)
		{
			c_suffix_em += graph->host_list[i]->host->vol_free;
		}
		if (c_suffix_em <= 0 || c_suffix_l <= 0) return 0;
		ans = min((c_suffix_em / ((int)REQ->function_chain.size() - RF)), (int)(c_suffix_l / (REQ->rate * PL)));
		if (ans < 0) return 0;
		return ans;
	}

	double l_suffix_r_calculate()
	{
		int count = u_suffix_j.size() - 1;
		double ans = 0.0;
		for (int i = 0; i < count; i++)
		{
			int count1 = u_suffix_j[i].size();
			int count2 = u_suffix_j[i + 1].size();
			double avrg = 0.0;
			for (int j = 0; j < count1; j++)
			{
				for (int k = 0; k < count2; k++)
				{
					avrg += graph_algorithm->distances[u_suffix_j[i][j]->id][u_suffix_j[i + 1][k]->id];
				}
			}
			ans += avrg / (double)(count1 * count2);
		}
		ans = ans / (double)(reusable_nf + 1);
		return ans;
	}

	double l_suffix_tri_calculate(Traffic* REQ)
	{
		double ans = 0.0;
		int chain_length = REQ->function_chain.size();
		vector<vector<Node*> > u(chain_length + 2, vector<Node*>(0));
		u[0].push_back(graph->node_map[REQ->src]);
		for (int i = 0; i < chain_length; i++)
		{
			for (int j = 0; j < graph->host_num; j++)
			{
				if (graph->host_list[j]->host->vol_free > 0 || graph->host_list[j]->host->vm_reusable(REQ->function_chain[i], REQ->rate))
				{
					u[i + 1].push_back(graph->host_list[j]);
				}
			}
		}
		u[chain_length + 1].push_back(graph->node_map[REQ->dst]);
		for (int i = 0; i <= chain_length; i++)
		{
			int count = 0;
			double avrg = 0.0;
			int count1 = u[i].size();
			int count2 = u[i + 1].size();
			for (int j = 0; j < count1; j++)
			{
				for (int k = 0; k < count2; k++)
				{
					if (i == 0 && u[i + 1][k]->host->vol_free < 1) continue;
					else if (i == chain_length && u[i][j]->host->vol_free < 1) continue;
					else if (u[i][j]->host->vol_free < 1 && u[i + 1][k]->host->vol_free < 1) continue;
					else
					{
						count += 1;
						avrg += graph_algorithm->distances[u[i][j]->id][u[i + 1][k]->id];
					}
				}
				ans += avrg / (double)count;
			}
		}
		ans = ans / (double)(chain_length + 1);
		return ans;
	}

	double n_suffix_e_calculate(Traffic* REQ)
	{
		int count = u_suffix_j.size() - 1;
		double ans = 0.0;
		for (int i = 0; i < count; i++)
		{
			int count1 = u_suffix_j[i].size();
			int count2 = u_suffix_j[i + 1].size();
			double avrg = 0.0;
			for (int j = 0; j < count1; j++)
			{
				for (int k = 0; k < count2; k++)
				{
					Node* flag = u_suffix_j[i + 1][k];
					while (flag->id != REQ->src)
					{
						if (flag->type == 'h') avrg += (double)flag->host->vol_free;
						flag = graph_algorithm->parents[REQ->src][flag->id];
					}
					if (flag->type == 'h') avrg += (double)flag->host->vol_free;
				}
			}
			ans += avrg / (double)(count1 * count2);
		}
		ans = ans / (double)(reusable_nf + 1);
		return ans;
	}

	void path_stretch(Traffic* REQ)
	{
		double alpha = JVP_STRETCH_ALPHA;
		double demands = (double)JVP_STRETCH_N;
		double future_consumption = ((1 - alpha) / alpha) * REQ->rate * (double)path_length * demands;
		double ratio = graph->overall_bandwidth / future_consumption;
		double SF = max(ratio, 1.0);
		path_length = (int)((double)path_length * SF);
	}

	/*
	* path-extension problem to complete the service chain
	*/

	vector<int> host_free;
	vector<Node*> host_sp;
	vector<Node*> node_sp;
	vector<Node*> vm_chain; // store hosts running vm with specific service
	Node* host_v;

	void path_extend(Traffic* REQ)
	{
		vm_chain.resize(REQ->function_chain.size(), NULL);
		host_v = graph->node_map[REQ->src];
		int fore_factor = 0;
		int cur_factor = 0;
		int cur_length = node_sp.size();
		cur_factor = svp_solve(REQ, graph->node_map[REQ->src]);
		while (cur_factor > fore_factor && cur_length <= path_length)
		{
			fore_factor = cur_factor;
			int flag_factor = 0;
			int flag_length = 0;
			for (int i = 0; i < graph->host_num; i++)
			{
				if (graph->host_list[i] == host_v) continue;
				else
				{
					vm_chain.resize(REQ->function_chain.size(), NULL);
					int svp_ans = svp_solve(REQ, graph->host_list[i]);
					if (flag_factor < svp_ans)
					{
						flag_factor = svp_ans;
						flag_length = node_sp.size();
						host_v = graph->host_list[i];
					}
				}
			}
			cur_factor = flag_factor;
			cur_length = flag_length;
		}
	}

	int svp_solve(Traffic* REQ, Node* V)
	{
		int rf_count = 0;
		if (V->id != REQ->src || host_v != V)
		{
			vector<Node*> v_to_V = graph_algorithm->shortest_path_acquire(host_v->id, V->id, REQ->rate);
			v_to_V.pop_back();
			vector<Node*> V_to_t = graph_algorithm->shortest_path_acquire(V->id, REQ->dst, REQ->rate);
			while (node_sp.back() != host_v)
			{
				node_sp.pop_back();
			}
			node_sp.pop_back();
			node_sp.insert(node_sp.end(), v_to_V.begin(), v_to_V.end());
			node_sp.insert(node_sp.end(), V_to_t.begin(), V_to_t.end());
			host_sp = graph_algorithm->host_path_acquire(node_sp);
		}

		int hosts_size = host_sp.size();
		int functions_size = REQ->function_chain.size();
		vector<vector<int> > dp_reuse_factor(hosts_size, vector<int>(functions_size + 1, 0));
		vector<vector<vector<int> > > host_reuse_factor(hosts_size,
			vector<vector<int> >(functions_size, vector<int>(functions_size, 0)));
		// the second index means the start index of the function chain
		vector<vector<bool> > service_reusable(hosts_size, vector<bool>(functions_size, false));
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = 0; j < functions_size; j++)
			{
				service_reusable[i][j] = host_sp[i]->host->vm_reusable(REQ->function_chain[j], REQ->rate);
			}
		}
		for (int i = 0; i < hosts_size; i++)
		{
			for (int j = functions_size - 1; j >= 0; j--)
			{
				int tmp_cnt = 0;
				for (int k = j; k >= 0; k--)
				{
					if (service_reusable[i][k])
					{
						tmp_cnt += 1;
					}
					host_reuse_factor[i][k][j] = tmp_cnt;
				}
			}
		}
		for (int i = 0; i < functions_size; i++)
		{
			if (service_reusable[0][i])
			{
				if (i == 0) dp_reuse_factor[0][i + 1] = 1;
				else dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i] += 1;
				vm_chain[i] = host_sp[0];
			}
			else
			{
				if (i != 0) dp_reuse_factor[0][i + 1] = dp_reuse_factor[0][i];
			}
		}
		for (int i = 1; i < hosts_size; i++)
		{
			dp_reuse_factor[i][0] = host_reuse_factor[i - 1][0][functions_size - 1];
			for (int j = 1; j < functions_size; j++)
			{
				for (int k = 1; k <= j; k++)
				{
					if (dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j] > dp_reuse_factor[i][j])
					{
						dp_reuse_factor[i][j] = dp_reuse_factor[i - 1][k] + host_reuse_factor[i][k][j];
						for (int p = k; p <= j; p++)
						{
							if (service_reusable[i][p])
							{
								vm_chain[p] = host_sp[i];
							}
						}
					}
				}
			}
		}
		// SVP(dp) ends here

		for (int i = 0; i < functions_size; i++)
		{
			if (vm_chain[i] != NULL) rf_count += 1;
		}
		return rf_count;
	}

	void chain_complete(Traffic* REQ)
	{
		int functions_size = REQ->function_chain.size();
		int path_size = node_sp.size();
		int hosts_size = host_sp.size();
		for (auto n : vm_chain)
		{
			if (n != NULL) cout << n->id << " ";
		}
		cout << "\n";
		for (int i = 0; i < functions_size; i++)
		{
			if (vm_chain[i] == NULL)
			{
				if (i == 0)
				{
					if (hosts_size == 0) // find the nearest host to the src - node which can reuse or then add the vm
					{
						vm_chain[i] = near_reuse_or_add(node_sp[0], REQ->function_chain[i], REQ->rate);
						if (vm_chain[i] == NULL)
						{
							cout << "NCV ERROR JVP.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
					else
					{
						Node* host_tmp = NULL;
						for (int j = 1; j < functions_size; j++)
						{
							if (vm_chain[j] != NULL)
							{
								host_tmp = vm_chain[j];
								break;
							}
						}
						if (host_tmp == NULL) host_tmp = host_sp[0];
						vm_chain[i] = near_reuse_or_add(host_tmp, REQ->function_chain[i], REQ->rate);
						if (vm_chain[i] == NULL)
						{
							cout << "NCV ERROR JVP.complete_chain(): there is no sufficietn node" << "\n";
							return;
						}
					}
				}
				else
				{
					if (vm_chain[i - 1] == NULL) return;
					vm_chain[i] = near_reuse_or_add(vm_chain[i - 1], REQ->function_chain[i], REQ->rate);
					if (vm_chain[i] == NULL)
					{
						cout << "NCV ERROR JVP.complete_chain(): there is no sufficietn node" << "\n";
						return;
					}
				}
			}
			else
			{
				if (!vm_chain[i]->host->vm_reusable(REQ->function_chain[i], REQ->rate))
				{
					cout << "NCV ERROR JVP.complete_chain(): dp_SVP has some problems" << "\n";
				}
			}
		}
	}

	void set_constraint()
	{
		host_free.clear();
		int host_size = graph->host_num;
		host_free.resize(host_size);
		for (int i = 0; i < host_size; i++)
		{
			host_free[i] = graph->host_list[i]->host->vol_free;
		}
	}

	Node* near_reuse_or_add(Node* NODE, char FUNCTION, double RATE)
	{
		Node* node_ans = NULL;
		int dist = INT_MAX / 3;
		int hosts_size = graph->host_num;
		int idx = -1;
		for (int i = 0; i < hosts_size; i++)
		{
			if (graph_algorithm->distances[NODE->id][graph->host_list[i]->id] < dist)
			{
				if (graph->host_list[i]->host->vm_reusable(FUNCTION, RATE))
				{
					node_ans = graph->host_list[i];
					dist = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
					idx = -1;
				}
				else
				{
					if (host_free[i] > 0 && graph->host_list[i]->host->vm_addable())
					{
						node_ans = graph->host_list[i];
						dist = graph_algorithm->distances[NODE->id][graph->host_list[i]->id];
						idx = i;
					}
				}
			}
		}
		if (idx != -1) host_free[idx] -= 1;
		return node_ans;
	}

	void prop_generate(Traffic* REQ)
	{
		int functions_size = vm_chain.size();
		for (int i = 0; i < functions_size; i++)
		{
			if (vm_chain[i] == NULL)
			{
				if (prop != NULL)
				{
					delete prop;
					prop = NULL;
					cout << "NCV ERROR JVP.prop_generate(): there is no sufficient host" << "\n";
					return;
				}
			}
			else
			{
				VirtualMachine* vm_tmp = vm_chain[i]->host->vm_reuse(REQ->function_chain[i], REQ->rate);
				if (vm_tmp == NULL)
				{
					vm_tmp = vm_chain[i]->host->vm_add(REQ->function_chain[i], REQ->rate);
				}
				if (vm_tmp == NULL)
				{
					if (prop != NULL)
					{
						delete prop;
						prop = NULL;
						cout << "NCV ERROR JVP.prop_generate(): there is no sufficient VM" << "\n";
						return;
					}
				}
				prop->service_add(vm_chain[i], vm_tmp);
			}
		}
	}
};
