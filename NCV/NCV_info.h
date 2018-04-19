#pragma once

#include "NCV_basic.h"

class VirtualMachine
{
public:
	int id;
	char type; // represent the function type of the virtual machine served, which is in lowercase
	double rcs; // represent the computing capability of a virtual machine, of which 100 is the default value
	double rcs_free;
	int service_count;

	VirtualMachine(int ID, char TYPE, double RCS)
	{
		id = ID;
		type = TYPE;
		rcs = RCS;
		rcs_free = rcs;
		service_count = 0;
	}
};

class Host
{
public:
	int id;
	int vol_free;
	double vm_rcs;
	vector<VirtualMachine*> macs; // store the VMs running on the host

	Host(int ID, int VOLUME, double RCS)
	{
		id = ID;
		vol_free = VOLUME;
		vm_rcs = RCS;
	}

	~Host()
	{
		int tmp_size = macs.size();
		for (int i = 0; i < tmp_size; i++)
		{
			if (macs[i] != NULL)
			{
				delete macs[i];
				macs[i] = NULL;
			}
		}
	}

	bool vm_addable()
	{
		if (vol_free < 1) return false;
		else return true;
	}

	VirtualMachine* vm_add(char FUNCTION, double RATE)
	{
		if (vol_free < 1)
		{
			cout << "NCV ERROR Host.vm_add(): the host is not sufficient to start a new virtual machine" << endl;
			return NULL;
		}
		VirtualMachine* new_mac = new VirtualMachine(macs.size(), FUNCTION, vm_rcs);
		return new_mac;
	}

	int vm_count()
	{
		return macs.size();
	}

	bool vm_reusable(char FUNCTION, double RATE)
	{
		int macs_cnt = macs.size();
		for (int i = 0; i < macs_cnt; i++)
		{
			if ((macs[i]->type == FUNCTION) && (macs[i]->rcs_free > RATE)) return true;
		}
		return false;
	}

	VirtualMachine* vm_reuse(char FUNCTION, double RATE)
	{
		VirtualMachine* vm_ans = NULL;
		int macs_cnt = macs.size();
		double reuse_level = 0.0; // represent the used computing resource, find the VM which has the largest amount of used resource
		for (int i = 0; i < macs_cnt; i++)
		{
			if (macs[i]->type == FUNCTION && (macs[i]->rcs_free > RATE))
			{
				if (reuse_level < (macs[i]->rcs - macs[i]->rcs_free + RATE))
				{
					reuse_level = macs[i]->rcs - macs[i]->rcs_free + RATE;
					vm_ans = macs[i];
				}
			}
		}
		cout << "NCV VirtualMachine.vm_reuse(): reusable VM acquired" << "\n";
		return vm_ans;
	}

	void vm_deploy(char FUNCTION, double RATE, VirtualMachine* VM)
	{
		if (VM->type != FUNCTION)
		{
			cout << "Host ERROR: (vm_deploy( , , ))the VM can not serve this function." << endl;
			return;
		}
		if (VM->service_count < 1)
		{
			vol_free -= 1;
			macs.push_back(VM);
		}
		VM->rcs_free -= RATE;
		VM->service_count += 1;
	}
};

class Node
{
public:
	int id;
	char type; // 's' means the node is a switch, and 'h' means the node is a host
	int vol;
	Host* host;

	Node(int ID, int VOLUME)
	{
		id = ID;
		vol = VOLUME;
		host = NULL;
		if (vol == 0)
		{
			type = 's'; // switch
		}
		else
		{
			type = 'h'; // host
		}
	}

	Node(int ID, int VOLUME, double RCS)
	{
		id = ID;
		vol = VOLUME;
		host = NULL;
		if (vol == 0)
		{
			type = 's'; // switch
		}
		else
		{
			type = 'h'; // host
			host = new Host(id, vol, RCS);
		}
	}
};

class Edge
{
public:
	Node* src = NULL;
	Node* dst = NULL;
	double cap;

	Edge(Node* SRC, Node* DST, double BW)
	{
		src = SRC;
		dst = DST;
		cap = BW;
	}
};

class Topo
{
public:
	int node_num;
	int edge_num; // doubled, because of undirected graph

	int host_num;
	int switch_num;
	double base_bandwidth; // minimal bandwidth unit of an Edge in the Topo
	int volume; // Host's volume of running VMs
	double host_vm_rcs; // VM's computing resource
	
	double overall_bandwidth; // record the sum of bandwidth in a network

	vector<Edge*> edge_list;
	vector<vector<Edge*> > adj_list;
	vector<vector<Edge*> > adj_matrix;
	vector<vector<double> > load_matrix;
	vector<Node*> host_list;
	unordered_map<int, Node*> node_map;

	Topo() { ; }

	virtual ~Topo() { ; }
};

class SimpleFatTree : public Topo
{
private:
	int level;
	
	void core_constructor(int LEVEL, int VOLUME, double BANDWIDTH, double RCS)
	{
		level = LEVEL;
		volume = VOLUME;
		base_bandwidth = BANDWIDTH;
		host_vm_rcs = RCS;
		switch_num = pow(2, level) - 1;
		host_num = pow(2, level);
		node_num = host_num + switch_num;
		routine_constructor();
		edge_num = edge_list.size();
		overall_bandwidth = 0.0;
	}

	void routine_constructor()
	{
		adj_list.resize(node_num, vector<Edge*>(0));
		adj_matrix.resize(node_num, vector<Edge*>(node_num, NULL));
		load_matrix.resize(node_num, vector<double>(node_num, 0.0));
		queue<Node*> node_queue;
		for (int i = 0; i < level; i++)
		{
			int left_index = pow(2, i) - 1;
			int right_index = pow(2, i) - 1 + pow(2, i);
			for (int j = left_index; j < right_index; j++)
			{
				Node* root = NULL;
				Node* left = NULL;
				Node* right = NULL;
				double bw_tmp = 0.0;
				if (node_queue.empty())
				{
					root = new Node(j, 0);
				}
				else
				{
					root = node_queue.front();
					node_queue.pop();
				}
				node_map[j] = root;
				if (i == level - 1)
				{
					left = new Node(j * 2 + 1, volume, host_vm_rcs); // to start a new host
					right = new Node(j * 2 + 2, volume, host_vm_rcs);
					host_list.push_back(left);
					host_list.push_back(right);
					bw_tmp = base_bandwidth;
				}
				else
				{
					left = new Node(j * 2 + 1, 0); // to start a new switch
					right = new Node(j * 2 + 2, 0);
					bw_tmp = base_bandwidth * pow(2, level - 2 - i);
					node_queue.push(left);
					node_queue.push(right);
				}
				Edge* edge_left = new Edge(root, left, bw_tmp);
				Edge* edge_right = new Edge(root, right, bw_tmp);
				Edge* edge_rleft = new Edge(left, root, bw_tmp);
				Edge* edge_rright = new Edge(right, root, bw_tmp);
				edge_list.push_back(edge_left);
				edge_list.push_back(edge_right);
				edge_list.push_back(edge_rleft);
				edge_list.push_back(edge_rright);
				node_map[left->id] = left;
				node_map[right->id] = right;
				adj_list[root->id].push_back(edge_left);
				adj_list[root->id].push_back(edge_right);
				adj_list[left->id].push_back(edge_rleft);
				adj_list[right->id].push_back(edge_rright);
				adj_matrix[root->id][left->id] = edge_left;
				adj_matrix[root->id][right->id] = edge_right;
				adj_matrix[left->id][root->id] = edge_rleft;
				adj_matrix[right->id][root->id] = edge_rright;
				overall_bandwidth += bw_tmp * 4;
			}
		}
	}

public:
	SimpleFatTree(int LEVEL)
	{
		core_constructor(LEVEL, 10, 100.0, 100.0);
	}

	SimpleFatTree(int LEVEL, int VOLUME)
	{
		core_constructor(LEVEL, VOLUME, 100.0, 100.0);
	}

	SimpleFatTree(int LEVEL, int VOLUME, double BANDWIDTH)
	{
		core_constructor(LEVEL, VOLUME, BANDWIDTH, 100.0);
	}

	SimpleFatTree(int LEVEL, int VOLUME, double BANDWIDTH, double RCS)
	{
		core_constructor(LEVEL, VOLUME, BANDWIDTH, RCS);
	}

	~SimpleFatTree() { ; }
};

class Traffic
{
private:
	void chain_generate(int TYPES, int LENGTH)
	{
		set<char> function_set;
		while (function_set.size() < LENGTH)
		{
			function_set.insert('a' + (rand() % TYPES));
		}
		for (auto f : function_set)
		{
			function_chain.push_back(f);
		}
	}

public:
	int id;
	double rate; // represent the bandwidth concumption of the traffic request

	int src;
	int dst;
	vector<char> function_chain; // represented in lowercase character

	Traffic(int ID, int SRC, int DST, double RATE, int MAX_TYPES, int CHAIN_LENGTH)
	{
		// the number of service types is less than 26, which service type will be presented by lowercase letter
		id = ID;
		src = SRC; // here src and dst presented by integer rather not Node class
		dst = DST;
		rate = RATE;
		chain_generate(MAX_TYPES, CHAIN_LENGTH);
	}
};

class Scheme
{
public:
	vector<pair<Node*, VirtualMachine*> > service_chain;
	vector<Node*> host_chain;
	double bw = 0.0;

	Scheme() { ; }

	Scheme(double RATE)
	{
		bw = RATE;
	}

	void service_add(Node* HOST, VirtualMachine* VM)
	{
		pair<Node*, VirtualMachine*> tmp_pair(HOST, VM);
		service_chain.push_back(tmp_pair);
		if (host_chain.empty() || host_chain.back() != HOST)
		{
			host_chain.push_back(HOST);
		}
	}
};

// quick note: mapping from traffic request to service chain scheme will be stored in a 'unordered_map'
// quick note: vm resource consumption of a service chain will be represented by link bandwidth consumption for the ease of computing
